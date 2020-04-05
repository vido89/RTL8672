/*
 * This file Copyright (C) 2007-2010 Mnemosyne LLC
 *
 * This file is licensed by the GPL version 2.  Works owned by the
 * Transmission project are granted a special exemption to clause 2(b)
 * so that the bulk of its code can remain under the MIT license.
 * This exemption does not extend to derived works not owned by
 * the Transmission project.
 *
 * $Id: inout.c 10023 2010-01-26 21:41:40Z charles $
 */

#ifdef HAVE_LSEEK64
 #define _LARGEFILE64_SOURCE
#endif

#include <assert.h>
#include <errno.h>
#include <stdlib.h> /* realloc */
#include <string.h> /* memcmp */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <openssl/sha.h>

#include "transmission.h"
#include "crypto.h"
#include "fdlimit.h"
#include "inout.h"
#include "platform.h"
#include "stats.h"
#include "torrent.h"
#include "utils.h"

/****
*****  Low-level IO functions
****/

#ifdef WIN32
 #if defined(read)
  #undef read
 #endif
 #define read  _read

 #if defined(write)
  #undef write
 #endif
 #define write _write
#endif

enum { TR_IO_READ, TR_IO_PREFETCH,
       /* Any operations that require write access must follow TR_IO_WRITE. */
       TR_IO_WRITE
};

int64_t
tr_lseek( int fd, int64_t offset, int whence )
{
#if defined( HAVE_LSEEK64 )
    return lseek64( fd, (off64_t)offset, whence );
#elif defined( WIN32 )
    return _lseeki64( fd, offset, whence );
#else
    return lseek( fd, (off_t)offset, whence );
#endif
}

/* returns 0 on success, or an errno on failure */
static int
readOrWriteBytes( tr_session       * session,
                  const tr_torrent * tor,
                  int                ioMode,
                  tr_file_index_t    fileIndex,
                  uint64_t           fileOffset,
                  void *             buf,
                  size_t             buflen )
{
    const tr_info * info = &tor->info;
    const tr_file * file = &info->files[fileIndex];

    int             fd = -1;
    int             err = 0;
    const tr_bool doWrite = ioMode >= TR_IO_WRITE;

    assert( fileIndex < info->fileCount );
    assert( !file->length || ( fileOffset < file->length ) );
    assert( fileOffset + buflen <= file->length );

    if( !file->length )
        return 0;

    fd = tr_fdFileGetCached( session, tr_torrentId( tor ), fileIndex, doWrite );

    if( fd < 0 )
    {
        /* the fd cache doesn't have this file...
         * we'll need to open it and maybe create it */
        char * subpath;
        const char * base;
        tr_bool fileExists;
        tr_preallocation_mode preallocationMode;

        fileExists = tr_torrentFindFile2( tor, fileIndex, &base, &subpath );

        if( !fileExists )
        {
            base = tr_torrentGetCurrentDir( tor );

            if( tr_sessionIsIncompleteFileNamingEnabled( tor->session ) )
                subpath = tr_torrentBuildPartial( tor, fileIndex );
            else
                subpath = tr_strdup( file->name );
        }

        if( ( file->dnd ) || ( ioMode < TR_IO_WRITE ) )
            preallocationMode = TR_PREALLOCATE_NONE;
        else
            preallocationMode = tor->session->preallocationMode;

        if( ( ioMode < TR_IO_WRITE ) && !fileExists ) /* does file exist? */
        {
            err = ENOENT;
        }
        else
        {
            char * filename = tr_buildPath( base, subpath, NULL );

            if( ( fd = tr_fdFileCheckout( session, tor->uniqueId, fileIndex, filename,
                                          doWrite, preallocationMode, file->length ) ) < 0 )
            {
                err = errno;
                tr_torerr( tor, "tr_fdFileCheckout failed for \"%s\": %s", filename, tr_strerror( err ) );
            }

            tr_free( filename );
        }

        if( doWrite && !err )
            tr_statsFileCreated( tor->session );

        tr_free( subpath );
    }

    if( !err )
    {
        if( ioMode == TR_IO_READ ) {
            const int rc = tr_pread( fd, buf, buflen, fileOffset );
            if( rc < 0 ) {
                err = errno;
                tr_torerr( tor, "read failed for \"%s\": %s",
                           file->name, tr_strerror( err ) );
            }
        } else if( ioMode == TR_IO_PREFETCH ) {
            const int rc = tr_prefetch( fd, fileOffset, buflen );
            if( rc < 0 ) {
                tr_tordbg( tor, "prefetch failed for \"%s\": %s",
                           file->name, tr_strerror( err ) );
            }
        } else if( ioMode == TR_IO_WRITE ) {
            const int rc = tr_pwrite( fd, buf, buflen, fileOffset );
            if( rc < 0 ) {
                err = errno;
                tr_torerr( tor, "write failed for \"%s\": %s",
                           file->name, tr_strerror( err ) );
            }
        } else {
            abort();
        }
    }

    return err;
}

static int
compareOffsetToFile( const void * a,
                     const void * b )
{
    const uint64_t  offset = *(const uint64_t*)a;
    const tr_file * file = b;

    if( offset < file->offset ) return -1;
    if( offset >= file->offset + file->length ) return 1;
    return 0;
}

void
tr_ioFindFileLocation( const tr_torrent * tor,
                       tr_piece_index_t   pieceIndex,
                       uint32_t           pieceOffset,
                       tr_file_index_t  * fileIndex,
                       uint64_t         * fileOffset )
{
    const uint64_t  offset = tr_pieceOffset( tor, pieceIndex, pieceOffset, 0 );
    const tr_file * file;

    file = bsearch( &offset,
                    tor->info.files, tor->info.fileCount, sizeof( tr_file ),
                    compareOffsetToFile );

    *fileIndex = file - tor->info.files;
    *fileOffset = offset - file->offset;

    assert( *fileIndex < tor->info.fileCount );
    assert( *fileOffset < file->length );
    assert( tor->info.files[*fileIndex].offset + *fileOffset == offset );
}

/* returns 0 on success, or an errno on failure */
static int
readOrWritePiece( tr_torrent       * tor,
                  int                ioMode,
                  tr_piece_index_t   pieceIndex,
                  uint32_t           pieceOffset,
                  uint8_t          * buf,
                  size_t             buflen )
{
    int             err = 0;
    tr_file_index_t fileIndex;
    uint64_t        fileOffset;
    const tr_info * info = &tor->info;

    if( pieceIndex >= tor->info.pieceCount )
        return EINVAL;
    if( pieceOffset + buflen > tr_torPieceCountBytes( tor, pieceIndex ) )
        return EINVAL;

    tr_ioFindFileLocation( tor, pieceIndex, pieceOffset,
                           &fileIndex, &fileOffset );

    while( buflen && !err )
    {
        const tr_file * file = &info->files[fileIndex];
        const uint64_t bytesThisPass = MIN( buflen, file->length - fileOffset );

        err = readOrWriteBytes( tor->session, tor, ioMode, fileIndex, fileOffset, buf, bytesThisPass );
        buf += bytesThisPass;
        buflen -= bytesThisPass;
        ++fileIndex;
        fileOffset = 0;

        if( ( err != 0 ) && (ioMode == TR_IO_WRITE ) )
        {
            char * path = tr_buildPath( tor->downloadDir, file->name, NULL );
            tr_torrentSetLocalError( tor, "%s (%s)", tr_strerror( err ), path );
            tr_free( path );
        }
    }

    return err;
}

int
tr_ioRead( tr_torrent       * tor,
           tr_piece_index_t   pieceIndex,
           uint32_t           begin,
           uint32_t           len,
           uint8_t          * buf )
{
    return readOrWritePiece( tor, TR_IO_READ, pieceIndex, begin, buf, len );
}

int
tr_ioPrefetch( tr_torrent       * tor,
               tr_piece_index_t   pieceIndex,
               uint32_t           begin,
               uint32_t           len)
{
    return readOrWritePiece( tor, TR_IO_PREFETCH, pieceIndex, begin,
                             NULL, len );
}

int
tr_ioWrite( tr_torrent       * tor,
            tr_piece_index_t   pieceIndex,
            uint32_t           begin,
            uint32_t           len,
            const uint8_t    * buf )
{
    return readOrWritePiece( tor, TR_IO_WRITE, pieceIndex, begin,
                             (uint8_t*)buf,
                             len );
}

/****
*****
****/

static tr_bool
recalculateHash( tr_torrent       * tor,
                 tr_piece_index_t   pieceIndex,
                 void             * buffer,
                 size_t             buflen,
                 uint8_t          * setme )
{
    size_t   bytesLeft;
    uint32_t offset = 0;
    tr_bool  success = TRUE;
    uint8_t  stackbuf[MAX_STACK_ARRAY_SIZE];
    SHA_CTX  sha;

    /* fallback buffer */
    if( ( buffer == NULL ) || ( buflen < 1 ) )
    {
        buffer = stackbuf;
        buflen = sizeof( stackbuf );
    }

    assert( tor != NULL );
    assert( pieceIndex < tor->info.pieceCount );
    assert( buffer != NULL );
    assert( buflen > 0 );
    assert( setme != NULL );

    SHA1_Init( &sha );
    bytesLeft = tr_torPieceCountBytes( tor, pieceIndex );

    while( bytesLeft )
    {
        const int len = MIN( bytesLeft, buflen );
        success = !tr_ioRead( tor, pieceIndex, offset, len, buffer );
        if( !success )
            break;
        SHA1_Update( &sha, buffer, len );
        offset += len;
        bytesLeft -= len;
    }

    if( success )
        SHA1_Final( setme, &sha );

    return success;
}

tr_bool
tr_ioTestPiece( tr_torrent        * tor,
                tr_piece_index_t    pieceIndex,
                void              * buffer,
                size_t              buflen )
{
    uint8_t hash[SHA_DIGEST_LENGTH];

    return recalculateHash( tor, pieceIndex, buffer, buflen, hash )
           && !memcmp( hash, tor->info.pieces[pieceIndex].hash, SHA_DIGEST_LENGTH );
}
