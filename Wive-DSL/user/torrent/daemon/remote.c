/*
 * This file Copyright (C) 2008-2010 Mnemosyne LLC
 *
 * This file is licensed by the GPL version 2.  Works owned by the
 * Transmission project are granted a special exemption to clause 2(b)
 * so that the bulk of its code can remain under the MIT license.
 * This exemption does not extend to derived works not owned by
 * the Transmission project.
 *
 * $Id: remote.c 9890 2010-01-05 23:47:50Z charles $
 */

#include <ctype.h> /* isspace */
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strcmp */

#ifdef WIN32
 #include <direct.h> /* getcwd */
#else
 #include <unistd.h> /* getcwd */
#endif

#include <event.h>

#define CURL_DISABLE_TYPECHECK /* otherwise -Wunreachable-code goes insane */
#include <curl/curl.h>

#include <libtransmission/transmission.h>
#include <libtransmission/bencode.h>
#include <libtransmission/rpcimpl.h>
#include <libtransmission/json.h>
#include <libtransmission/tr-getopt.h>
#include <libtransmission/utils.h>
#include <libtransmission/version.h>

#define MY_NAME "transmission-remote"
#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT atoi(TR_DEFAULT_RPC_PORT_STR)

enum { TAG_SESSION, TAG_STATS, TAG_LIST, TAG_DETAILS, TAG_FILES, TAG_PEERS };

static const char*
getUsage( void )
{
    return
        "Transmission " LONG_VERSION_STRING
        "  http://www.transmissionbt.com/\n"
        "A fast and easy BitTorrent client\n"
        "\n"
        "Usage: " MY_NAME
        " [host] [options]\n"
        "       "
        MY_NAME " [port] [options]\n"
                "       "
        MY_NAME " [host:port] [options]\n"
                "\n"
                "See the man page for detailed explanations and many examples.";
}

static tr_option opts[] =
{
    { 'a', "add",                   "Add torrent files by filename or URL", "a",  0, NULL },
    { 970, "alt-speed",             "Use the alternate Limits", "as",  0, NULL },
    { 971, "no-alt-speed",          "Don't use the alternate Limits", "AS",  0, NULL },
    { 972, "alt-speed-downlimit",   "max alternate download speed (in KB/s)", "asd",  1, "<speed>" },
    { 973, "alt-speed-uplimit",     "max alternate upload speed (in KB/s)", "asu",  1, "<speed>" },
    { 974, "alt-speed-scheduler",   "Use the scheduled on/off times", "asc",  0, NULL },
    { 975, "no-alt-speed-scheduler","Don't use the scheduled on/off times", "ASC",  0, NULL },
    { 976, "alt-speed-time-begin",  "Time to start using the alt speed limits (in hhmm)", NULL,  1, "<time>" },
    { 977, "alt-speed-time-end",    "Time to stop using the alt speed limits (in hhmm)", NULL,  1, "<time>" },
    { 978, "alt-speed-days",        "Numbers for any/all days of the week - eg. \"1-7\"", NULL,  1, "<days>" },
    { 'c', "incomplete-dir",        "Where to store new torrents until they're complete", "c", 1, "<dir>" },
    { 'C', "no-incomplete-dir",     "Don't store incomplete torrents in a different location", "C", 0, NULL },
    { 'b', "debug",                 "Print debugging information", "b",  0, NULL },
    { 'd', "downlimit",             "Set the maximum global download speed in KB/s", "d",  1, "<speed>" },
    { 'D', "no-downlimit",          "Don't limit the global download speed", "D",  0, NULL },
    { 910, "encryption-required",   "Encrypt all peer connections", "er", 0, NULL },
    { 911, "encryption-preferred",  "Prefer encrypted peer connections", "ep", 0, NULL },
    { 912, "encryption-tolerated",  "Prefer unencrypted peer connections", "et", 0, NULL },
    { 'f', "files",                 "List the current torrent(s)' files", "f",  0, NULL },
    { 'g', "get",                   "Mark files for download", "g",  1, "<files>" },
    { 'G', "no-get",                "Mark files for not downloading", "G",  1, "<files>" },
    { 'i', "info",                  "Show the current torrent(s)' details", "i",  0, NULL },
    { 920, "session-info",          "Show the session's details", "si", 0, NULL },
    { 921, "session-stats",         "Show the session's statistics", "st", 0, NULL },
    { 'l', "list",                  "List all torrents", "l",  0, NULL },
    { 960, "move",                  "Move current torrent's data to a new folder", NULL, 1, "<path>" },
    { 961, "find",                  "Tell Transmission where to find a torrent's data", NULL, 1, "<path>" },
    { 'm', "portmap",               "Enable portmapping via NAT-PMP or UPnP", "m",  0, NULL },
    { 'M', "no-portmap",            "Disable portmapping", "M",  0, NULL },
    { 'n', "auth",                  "Set authentication info", "n",  1, "<username:password>" },
    { 'N', "netrc",                 "Set authentication info from a .netrc file", "N",  1, "<filename>" },
    { 'o', "dht",                   "Enable distributed hash tables (DHT)", "o", 0, NULL },
    { 'O', "no-dht",                "Disable distributed hash tables (DHT)", "O", 0, NULL },
    { 'p', "port",                  "Port for incoming peers (Default: " TR_DEFAULT_PEER_PORT_STR ")", "p", 1, "<port>" },
    { 'P', "random-port",           "Random port for incomping peers", "P", 0, NULL },
    { 900, "priority-high",         "Set the files' priorities as high", "ph", 1, "<files>" },
    { 901, "priority-normal",       "Set the files' priorities as normal", "pn", 1, "<files>" },
    { 902, "priority-low",          "Set the files' priorities as low", "pl", 1, "<files>" },
    { 'r', "remove",                "Remove the current torrent(s)", "r",  0, NULL },
    { 930, "peers",                 "Set the current torrent(s)' maximum number of peers each", "pr", 1, "<max>" },
    { 931, "global-peers",          "Set the global maximum number of peers", "gpr", 1, "<max>" },
    { 'R', "remove-and-delete",     "Remove the current torrent(s) and delete local data", NULL, 0, NULL },
    { 950, "seedratio",             "Let the current torrent(s) seed until a specific ratio", "sr", 1, "ratio" },
    { 951, "seedratio-default",     "Let the current torrent(s) use the global seedratio settings", "srd", 0, NULL },
    { 952, "no-seedratio",          "Let the current torrent(s) seed regardless of ratio", "SR", 0, NULL },
    { 953, "global-seedratio",      "All torrents, unless overridden by a per-torrent setting, should seed until a specific ratio", "gsr", 1, "ratio" },
    { 954, "no-global-seedratio",   "All torrents, unless overridden by a per-torrent setting, should seed regardless of ratio", "GSR", 0, NULL },
    { 's', "start",                 "Start the current torrent(s)", "s",  0, NULL },
    { 'S', "stop",                  "Stop the current torrent(s)", "S",  0, NULL },
    { 't', "torrent",               "Set the current torrent(s)", "t",  1, "<torrent>" },
    { 980, "torrent-downlimit",     "Set the maximum download speed for the current torrent(s) in KB/s", "td",  1, "<speed>" },
    { 981, "no-torrent-downlimit",  "Don't limit the download speed for the current torrent(s)", "TD",  0, NULL },
    { 982, "torrent-uplimit",       "Set the maximum upload speed for the current torrent(s) in KB/s", "tu",  1, "<speed>" },
    { 983, "no-torrent-uplimit",    "Don't limit the upload speed for the current torrent(s)", "TU",  0, NULL },
    { 984, "honor-session",         "Make the current torrent(s) honor the session limits", "hl",  0, NULL },
    { 985, "no-honor-session",      "Make the current torrent(s) not honor the session limits", "HL",  0, NULL },
    { 'u', "uplimit",               "Set the maximum global upload speed in KB/s", "u",  1, "<speed>" },
    { 'U', "no-uplimit",            "Don't limit the global upload speed", "U",  0, NULL },
    { 'v', "verify",                "Verify the current torrent(s)", "v",  0, NULL },
    { 'V', "version",               "Show version number and exit", "V", 0, NULL },
    { 'w', "download-dir",          "Set the default download folder", "w",  1, "<path>" },
    { 'x', "pex",                   "Enable peer exchange (PEX)", "x",  0, NULL },
    { 'X', "no-pex",                "Disable peer exchange (PEX)", "X",  0, NULL },
    { 940, "peer-info",             "List the current torrent(s)' peers", "pi",  0, NULL },
    {   0, NULL,                    NULL, NULL, 0, NULL }
};

static void
showUsage( void )
{
    tr_getopt_usage( MY_NAME, getUsage( ), opts );
}

static int
numarg( const char * arg )
{
    char *     end = NULL;
    const long num = strtol( arg, &end, 10 );

    if( *end )
    {
        fprintf( stderr, "Not a number: \"%s\"\n", arg );
        showUsage( );
        exit( EXIT_FAILURE );
    }
    return num;
}

static char * reqs[256]; /* arbitrary max */
static int    reqCount = 0;
static tr_bool debug = 0;
static char * auth = NULL;
static char * netrc = NULL;
static char * sessionId = NULL;

static char*
tr_getcwd( void )
{
    char buf[2048];
    *buf = '\0';
#ifdef WIN32
    _getcwd( buf, sizeof( buf ) );
#else
    getcwd( buf, sizeof( buf ) );
#endif
    return tr_strdup( buf );
}

static char*
absolutify( const char * path )
{
    char * buf;

    if( *path == '/' )
        buf = tr_strdup( path );
    else {
        char * cwd = tr_getcwd( );
        buf = tr_buildPath( cwd, path, NULL );
        tr_free( cwd );
    }

    return buf;
}

static char*
getEncodedMetainfo( const char * filename )
{
    size_t    len = 0;
    char *    b64 = NULL;
    uint8_t * buf = tr_loadFile( filename, &len );

    if( buf )
    {
        b64 = tr_base64_encode( buf, len, NULL );
        tr_free( buf );
    }
    return b64;
}

static void
addIdArg( tr_benc *    args,
          const char * id )
{
    if( !*id )
    {
        fprintf(
            stderr,
            "No torrent specified!  Please use the -t option first.\n" );
        id = "-1"; /* no torrent will have this ID, so should be a no-op */
    }
    if( strcmp( id, "all" ) )
    {
        const char * pch;
        tr_bool isList = strchr(id,',') || strchr(id,'-');
        tr_bool isNum = TRUE;
        for( pch=id; isNum && *pch; ++pch )
            if( !isdigit( *pch ) )
                isNum = FALSE;
        if( isNum || isList )
            tr_rpc_parse_list_str( tr_bencDictAdd( args, "ids" ), id, strlen( id ) );
        else
            tr_bencDictAddStr( args, "ids", id ); /* it's a torrent sha hash */
    }
}

static void
addTime( tr_benc * args, const char * key, const char * arg )
{
    int time;
    tr_bool success = FALSE;

    if( arg && ( strlen( arg ) == 4 ) )
    {
        const char hh[3] = { arg[0], arg[1], '\0' };
        const char mm[3] = { arg[2], arg[3], '\0' };
        const int hour = atoi( hh );
        const int min = atoi( mm );

        if( 0<=hour && hour<24 && 0<=min && min<60 )
        {
            time = min + ( hour * 60 );
            success = TRUE;
        }
    }

    if( success )
        tr_bencDictAddInt( args, key, time );
    else
        fprintf( stderr, "Please specify the time of day in 'hhmm' format.\n" );
}

static void
addDays( tr_benc * args, const char * key, const char * arg )
{
    int days = 0;

    if( arg )
    {
        int i;
        int valueCount;
        int * values = tr_parseNumberRange( arg, -1, &valueCount );
        for( i=0; i<valueCount; ++i )
        {
            if ( values[i] < 0 || values[i] > 7 ) continue;
            if ( values[i] == 7 ) values[i] = 0;

            days |= 1 << values[i];
        }
        tr_free( values );
    }

    if ( days )
        tr_bencDictAddInt( args, key, days );
    else
        fprintf( stderr, "Please specify the days of the week in '1-3,4,7' format.\n" );
}

static void
addFiles( tr_benc *    args,
          const char * key,
          const char * arg )
{
    tr_benc * files = tr_bencDictAddList( args, key, 100 );

    if( !*arg )
    {
        fprintf( stderr, "No files specified!\n" );
        arg = "-1"; /* no file will have this index, so should be a no-op */
    }
    if( strcmp( arg, "all" ) )
    {
        int i;
        int valueCount;
        int * values = tr_parseNumberRange( arg, -1, &valueCount );
        for( i=0; i<valueCount; ++i )
            tr_bencListAddInt( files, values[i] );
        tr_free( values );
    }
}

#define TR_N_ELEMENTS( ary ) ( sizeof( ary ) / sizeof( *ary ) )

static const char * files_keys[] = {
    "files",
    "name",
    "priorities",
    "wanted"
};

static const char * details_keys[] = {
    "activityDate",
    "addedDate",
    "comment",
    "corruptEver",
    "creator",
    "dateCreated",
    "doneDate",
    "downloadDir",
    "downloadedEver",
    "downloadLimit",
    "downloadLimited",
    "error",
    "errorString",
    "eta",
    "hashString",
    "haveUnchecked",
    "haveValid",
    "honorsSessionLimits",
    "id",
    "isPrivate",
    "leftUntilDone",
    "name",
    "peersConnected",
    "peersGettingFromUs",
    "peersSendingToUs",
    "peer-limit",
    "pieceCount",
    "pieceSize",
    "rateDownload",
    "rateUpload",
    "recheckProgress",
    "sizeWhenDone",
    "startDate",
    "status",
    "totalSize",
    "trackerStats",
    "uploadedEver",
    "uploadLimit",
    "uploadLimited",
    "pieces",
    "webseeds",
    "webseedsSendingToUs"
};

static const char * list_keys[] = {
    "error",
    "errorString",
    "eta",
    "id",
    "leftUntilDone",
    "name",
    "peersGettingFromUs",
    "peersSendingToUs",
    "rateDownload",
    "rateUpload",
    "sizeWhenDone",
    "status",
    "uploadRatio"
};

static int
readargs( int argc, const char ** argv )
{
    int c;
    tr_bool addingTorrents = FALSE;
    int status = EXIT_SUCCESS;
    char id[4096];
    const char * optarg;

    *id = '\0';

    while( ( c = tr_getopt( getUsage( ), argc, argv, opts, &optarg ) ) )
    {
        int     i, n;
        int     addArg = TRUE;
        tr_benc top, *args, *fields;
        tr_bencInitDict( &top, 3 );
        args = tr_bencDictAddDict( &top, "arguments", 0 );

        switch( c )
        {
            case TR_OPT_UNK:
                if( addingTorrents )
                {
                    char * tmp = getEncodedMetainfo( optarg );
                    if( tmp )
                    {
                        tr_bencDictAddStr( &top, "method", "torrent-add" );
                        tr_bencDictAddStr( args, "metainfo", tmp );
                        tr_free( tmp );
                    }
                    else
                    {
                        tr_bencDictAddStr( &top, "method", "torrent-add" );
                        tr_bencDictAddStr( args, "filename", optarg );
                    }
                }
                else
                {
                    fprintf( stderr, "Unknown option: %s\n", optarg );
                    addArg = FALSE;
                    status |= EXIT_FAILURE;
                }
                break;

            case 'a':
                addingTorrents = TRUE;
                addArg = FALSE;
                break;

            case 'b':
                debug = TRUE;
                addArg = FALSE;
                break;

            case 'c':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddStr( args, TR_PREFS_KEY_INCOMPLETE_DIR, optarg );
                tr_bencDictAddBool( args, TR_PREFS_KEY_INCOMPLETE_DIR_ENABLED, TRUE );
                break;

            case 'C':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_INCOMPLETE_DIR_ENABLED, FALSE );
                break;

            case 'd':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddInt( args, TR_PREFS_KEY_DSPEED, numarg( optarg ) );
                tr_bencDictAddBool( args, TR_PREFS_KEY_DSPEED_ENABLED, TRUE );
                break;

            case 'D':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_DSPEED_ENABLED, FALSE );
                break;

            case 'f':
                tr_bencDictAddStr( &top, "method", "torrent-get" );
                tr_bencDictAddInt( &top, "tag", TAG_FILES );
                addIdArg( args, id );
                n = TR_N_ELEMENTS( files_keys );
                fields = tr_bencDictAddList( args, "fields", n );
                for( i = 0; i < n; ++i )
                    tr_bencListAddStr( fields, files_keys[i] );
                break;

            case 'g':
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                addIdArg( args, id );
                addFiles( args, "files-wanted", optarg );
                break;

            case 'G':
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                addIdArg( args, id );
                addFiles( args, "files-unwanted", optarg );
                break;

            case 'i':
                tr_bencDictAddStr( &top, "method", "torrent-get" );
                tr_bencDictAddInt( &top, "tag", TAG_DETAILS );
                addIdArg( args, id );
                n = TR_N_ELEMENTS( details_keys );
                fields = tr_bencDictAddList( args, "fields", n );
                for( i = 0; i < n; ++i )
                    tr_bencListAddStr( fields, details_keys[i] );
                break;

            case 'l':
                tr_bencDictAddStr( &top, "method", "torrent-get" );
                tr_bencDictAddInt( &top, "tag", TAG_LIST );
                n = TR_N_ELEMENTS( list_keys );
                fields = tr_bencDictAddList( args, "fields", n );
                for( i = 0; i < n; ++i )
                    tr_bencListAddStr( fields, list_keys[i] );
                break;

            case 'm':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_PORT_FORWARDING, TRUE );
                break;

            case 'M':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_PORT_FORWARDING, FALSE );
                break;

            case 'n':
                auth = tr_strdup( optarg );
                addArg = FALSE;
                break;

            case 'N':
                netrc = tr_strdup( optarg );
                addArg = FALSE;
                break;

            case 'p':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddInt( args, TR_PREFS_KEY_PEER_PORT, numarg( optarg ) );
                break;

            case 'P':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_PEER_PORT_RANDOM_ON_START, TRUE);
                break;

            case 'r':
                tr_bencDictAddStr( &top, "method", "torrent-remove" );
                addIdArg( args, id );
                break;

            case 'R':
                tr_bencDictAddStr( &top, "method", "torrent-remove" );
                addIdArg( args, id );
                tr_bencDictAddBool( args, "delete-local-data", TRUE );
                break;

            case 's':
                tr_bencDictAddStr( &top, "method", "torrent-start" );
                addIdArg( args, id );
                break;

            case 'S':
                tr_bencDictAddStr( &top, "method", "torrent-stop" );
                addIdArg( args, id );
                break;

            case 't':
                tr_strlcpy( id, optarg, sizeof( id ) );
                addArg = FALSE;
                break;

            case 'u':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddInt( args, TR_PREFS_KEY_USPEED, numarg( optarg ) );
                tr_bencDictAddBool( args, TR_PREFS_KEY_USPEED_ENABLED, TRUE );
                break;

            case 'U':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_USPEED_ENABLED, FALSE );
                break;

            case 'v':
                tr_bencDictAddStr( &top, "method", "torrent-verify" );
                addIdArg( args, id );
                break;

            case 'V':
                fprintf( stderr, "Transmission %s\n", LONG_VERSION_STRING );
                exit( 0 );
                break;

            case 'w': {
                char * path = absolutify( optarg );
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddStr( args, TR_PREFS_KEY_DOWNLOAD_DIR, path );
                tr_free( path );
                break;
            }

            case 'o':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_DHT_ENABLED, TRUE );
                break;

            case 'O':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_DHT_ENABLED, FALSE );
                break;

            case 'x':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_PEX_ENABLED, TRUE );
                break;

            case 'X':
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_PEX_ENABLED, FALSE );
                break;

            case 900:
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                addIdArg( args, id );
                addFiles( args, "priority-high", optarg );
                break;

            case 901:
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                addIdArg( args, id );
                addFiles( args, "priority-normal", optarg );
                break;

            case 902:
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                addIdArg( args, id );
                addFiles( args, "priority-low", optarg );
                break;

            case 910:
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddStr( args, TR_PREFS_KEY_ENCRYPTION, "required" );
                break;

            case 911:
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddStr( args, TR_PREFS_KEY_ENCRYPTION, "preferred" );
                break;

            case 912:
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddStr( args, TR_PREFS_KEY_ENCRYPTION, "tolerated" );
                break;

            case 920:
                tr_bencDictAddStr( &top, "method", "session-get" );
                tr_bencDictAddInt( &top, "tag", TAG_SESSION );
                break;

            case 921:
                tr_bencDictAddStr( &top, "method", "session-stats" );
                tr_bencDictAddInt( &top, "tag", TAG_STATS );
                break;

            case 930:
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                addIdArg( args, id );
                tr_bencDictAddInt( args, "peer-limit", atoi(optarg) );
                break;

            case 931:
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddInt( args, TR_PREFS_KEY_PEER_LIMIT_GLOBAL, atoi(optarg) );
                break;

            case 940:
                tr_bencDictAddStr( &top, "method", "torrent-get" );
                tr_bencDictAddInt( &top, "tag", TAG_PEERS );
                addIdArg( args, id );
                fields = tr_bencDictAddList( args, "fields", 1 );
                tr_bencListAddStr( fields, "peers" );
                break;

            case 950:
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                tr_bencDictAddReal( args, "seedRatioLimit", atof(optarg) );
                tr_bencDictAddInt( args, "seedRatioMode", TR_RATIOLIMIT_SINGLE );
                addIdArg( args, id );
                break;

            case 951:
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                tr_bencDictAddInt( args, "seedRatioMode", TR_RATIOLIMIT_GLOBAL );
                addIdArg( args, id );
                break;

            case 952:
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                tr_bencDictAddInt( args, "seedRatioMode", TR_RATIOLIMIT_UNLIMITED );
                addIdArg( args, id );
                break;

            case 953:
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddReal( args, "seedRatioLimit", atof(optarg) );
                tr_bencDictAddBool( args, "seedRatioLimited", TRUE );
                break;

            case 954:
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, "seedRatioLimited", FALSE );
                break;

            case 960:
                tr_bencDictAddStr( &top, "method", "torrent-set-location" );
                tr_bencDictAddStr( args, "location", optarg );
                tr_bencDictAddBool( args, "move", TRUE );
                addIdArg( args, id );
                break;

            case 961:
                tr_bencDictAddStr( &top, "method", "torrent-set-location" );
                tr_bencDictAddStr( args, "location", optarg );
                tr_bencDictAddBool( args, "move", FALSE );
                addIdArg( args, id );
                break;

            case 970:
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_ALT_SPEED_ENABLED, TRUE );
                break;

            case 971:
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_ALT_SPEED_ENABLED, FALSE );
                break;

            case 972:
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddInt( args, TR_PREFS_KEY_ALT_SPEED_DOWN, numarg( optarg ) );
                break;

            case 973:
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddInt( args, TR_PREFS_KEY_ALT_SPEED_UP, numarg( optarg ) );
                break;

            case 974:
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_ALT_SPEED_TIME_ENABLED, TRUE );
                break;

            case 975:
                tr_bencDictAddStr( &top, "method", "session-set" );
                tr_bencDictAddBool( args, TR_PREFS_KEY_ALT_SPEED_TIME_ENABLED, FALSE );
                break;

            case 976:
                tr_bencDictAddStr( &top, "method", "session-set" );
                addTime( args, TR_PREFS_KEY_ALT_SPEED_TIME_BEGIN, optarg);
                break;

            case 977:
                tr_bencDictAddStr( &top, "method", "session-set" );
                addTime( args, TR_PREFS_KEY_ALT_SPEED_TIME_END, optarg);
                break;

            case 978:
                tr_bencDictAddStr( &top, "method", "session-set" );
                addDays( args, TR_PREFS_KEY_ALT_SPEED_TIME_DAY, optarg );
                break;

            case 980:
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                addIdArg( args, id );
                tr_bencDictAddInt( args, "downloadLimit", numarg( optarg ) );
                tr_bencDictAddBool( args, "downloadLimited", TRUE );
                break;

            case 981:
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                addIdArg( args, id );
                tr_bencDictAddBool( args, "downloadLimited", FALSE );
                break;

            case 982:
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                addIdArg( args, id );
                tr_bencDictAddInt( args, "uploadLimit", numarg( optarg ) );
                tr_bencDictAddBool( args, "uploadLimited", TRUE );
                break;

            case 983:
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                addIdArg( args, id );
                tr_bencDictAddBool( args, "uploadLimited", FALSE );
                break;

            case 984:
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                addIdArg( args, id );
                tr_bencDictAddBool( args, "honorsSessionLimits", TRUE );
                break;

            case 985:
                tr_bencDictAddStr( &top, "method", "torrent-set" );
                addIdArg( args, id );
                tr_bencDictAddBool( args, "honorsSessionLimits", FALSE );
                break;

            case TR_OPT_ERR:
                fprintf( stderr, "invalid option\n" );
                showUsage( );
                status |= EXIT_FAILURE;
                break;

            default:
                fprintf( stderr, "got opt [%d]\n", (int)c );
                showUsage( );
                break;
        }

        if( addArg )
        {
            reqs[reqCount++] = tr_bencToStr( &top, TR_FMT_JSON_LEAN, NULL );
        }

        tr_bencFree( &top );
    }

    return status;
}

/* [host:port] or [host] or [port] */
static void
getHostAndPort( int * argc, char ** argv, char ** host, int * port )
{
    if( *argv[1] != '-' )
    {
        int          i;
        const char * s = argv[1];
        const char * delim = strchr( s, ':' );
        if( delim )   /* user passed in both host and port */
        {
            *host = tr_strndup( s, delim - s );
            *port = atoi( delim + 1 );
        }
        else
        {
            char *    end;
            const int i = strtol( s, &end, 10 );
            if( !*end ) /* user passed in a port */
                *port = i;
            else /* user passed in a host */
                *host = tr_strdup( s );
        }

        *argc -= 1;
        for( i = 1; i < *argc; ++i )
            argv[i] = argv[i + 1];
    }
}

static size_t
writeFunc( void * ptr,
           size_t size,
           size_t nmemb,
           void * buf )
{
    const size_t byteCount = size * nmemb;

    evbuffer_add( buf, ptr, byteCount );
    return byteCount;
}


static void
etaToString( char *  buf, size_t  buflen, int64_t eta )
{
    if( eta < 0 )
        tr_snprintf( buf, buflen, "Unknown" );
    else if( eta < 60 )
        tr_snprintf( buf, buflen, "%" PRId64 "sec", eta );
    else if( eta < ( 60 * 60 ) )
        tr_snprintf( buf, buflen, "%" PRId64 " min", eta / 60 );
    else if( eta < ( 60 * 60 * 24 ) )
        tr_snprintf( buf, buflen, "%" PRId64 " hrs", eta / ( 60 * 60 ) );
    else
        tr_snprintf( buf, buflen, "%" PRId64 " days", eta / ( 60 * 60 * 24 ) );
}

static char*
tr_strltime( char * buf, int seconds, size_t buflen )
{
    int  days, hours, minutes;
    char d[128], h[128], m[128], s[128];

    if( seconds < 0 )
        seconds = 0;

    days = seconds / 86400;
    hours = ( seconds % 86400 ) / 3600;
    minutes = ( seconds % 3600 ) / 60;
    seconds = ( seconds % 3600 ) % 60;

    tr_snprintf( d, sizeof( d ), "%'d day%s", days, days==1?"":"s" );
    tr_snprintf( h, sizeof( h ), "%'d hour%s", hours, hours==1?"":"s" );
    tr_snprintf( m, sizeof( m ), "%'d minute%s", minutes, minutes==1?"":"s" );
    tr_snprintf( s, sizeof( s ), "%'d second%s", seconds, seconds==1?"":"s" );

    if( days )
    {
        if( days >= 4 || !hours )
            tr_strlcpy( buf, d, buflen );
        else
            tr_snprintf( buf, buflen, "%s, %s", d, h );
    }
    else if( hours )
    {
        if( hours >= 4 || !minutes )
            tr_strlcpy( buf, h, buflen );
        else
            tr_snprintf( buf, buflen, "%s, %s", h, m );
    }
    else if( minutes )
    {
        if( minutes >= 4 || !seconds )
            tr_strlcpy( buf, m, buflen );
        else
            tr_snprintf( buf, buflen, "%s, %s", m, s );
    }
    else tr_strlcpy( buf, s, buflen );

    return buf;
}

#define KILOBYTE_FACTOR 1024.0
#define MEGABYTE_FACTOR ( 1024.0 * 1024.0 )
#define GIGABYTE_FACTOR ( 1024.0 * 1024.0 * 1024.0 )

static char*
strlratio2( char * buf, double ratio, size_t buflen )
{
    return tr_strratio( buf, buflen, ratio, "Inf" );
}

static char*
strlratio( char * buf,
           double numerator,
           double denominator,
           size_t buflen )
{
    double ratio;

    if( denominator )
        ratio = numerator / denominator;
    else if( numerator )
        ratio = TR_RATIO_INF;
    else
        ratio = TR_RATIO_NA;

    return strlratio2( buf, ratio, buflen );
}

static char*
strlsize( char *  buf, int64_t size, size_t  buflen )
{
    if( !size )
        tr_strlcpy( buf, "None", buflen );
    else if( size < (int64_t)KILOBYTE_FACTOR )
        tr_snprintf( buf, buflen, "%'" PRId64 " bytes", (int64_t)size );
    else
    {
        double displayed_size;
        if( size < (int64_t)MEGABYTE_FACTOR )
        {
            displayed_size = (double) size / KILOBYTE_FACTOR;
            tr_snprintf( buf, buflen, "%'.1f KB", displayed_size );
        }
        else if( size < (int64_t)GIGABYTE_FACTOR )
        {
            displayed_size = (double) size / MEGABYTE_FACTOR;
            tr_snprintf( buf, buflen, "%'.1f MB", displayed_size );
        }
        else
        {
            displayed_size = (double) size / GIGABYTE_FACTOR;
            tr_snprintf( buf, buflen, "%'.1f GB", displayed_size );
        }
    }
    return buf;
}

static char*
getStatusString( tr_benc * t, char * buf, size_t buflen )
{
    int64_t status;

    if( !tr_bencDictFindInt( t, "status", &status ) )
    {
        *buf = '\0';
    }
    else switch( status )
    {
        case TR_STATUS_STOPPED:
            tr_strlcpy( buf, "Stopped", buflen );
            break;

        case TR_STATUS_CHECK_WAIT:
        case TR_STATUS_CHECK: {
            const char * str = status == TR_STATUS_CHECK_WAIT
                             ? "Will Verify"
                             : "Verifying";
            double percent;
            if( tr_bencDictFindReal( t, "recheckProgress", &percent ) )
                tr_snprintf( buf, buflen, "%s (%.0f%%)", str, floor(percent*100.0) );
            else
                tr_strlcpy( buf, str, buflen );

            break;
        }

        case TR_STATUS_DOWNLOAD:
        case TR_STATUS_SEED: {
            int64_t fromUs = 0;
            int64_t toUs = 0;
            tr_bencDictFindInt( t, "peersGettingFromUs", &fromUs );
            tr_bencDictFindInt( t, "peersSendingToUs", &toUs );
            if( fromUs && toUs )
                tr_strlcpy( buf, "Up & Down", buflen );
            else if( toUs )
                tr_strlcpy( buf, "Downloading", buflen );
            else if( fromUs ) {
                int64_t leftUntilDone = 0;
                tr_bencDictFindInt( t, "leftUntilDone", &leftUntilDone );
                if( leftUntilDone > 0 )
                    tr_strlcpy( buf, "Uploading", buflen );
                else
                    tr_strlcpy( buf, "Seeding", buflen );
            } else {
                tr_strlcpy( buf, "Idle", buflen );
            }
            break;
        }
    }

    return buf;
}

static void
printSession( tr_benc * top )
{
    tr_benc *args;
    if( ( tr_bencDictFindDict( top, "arguments", &args ) ) )
    {
        const char * str;
        int64_t      i;
        tr_bool      boolVal;

        printf( "VERSION\n" );
        if( tr_bencDictFindStr( args,  "version", &str ) )
            printf( "  Daemon version: %s\n", str );
        if( tr_bencDictFindInt( args, "rpc-version", &i ) )
            printf( "  RPC version: %" PRId64 "\n", i );
        if( tr_bencDictFindInt( args, "rpc-version-minimum", &i ) )
            printf( "  RPC minimum version: %" PRId64 "\n", i );
        printf( "\n" );

        printf( "TRANSFER\n" );
        if( tr_bencDictFindStr( args,  TR_PREFS_KEY_DOWNLOAD_DIR, &str ) )
            printf( "  Download directory: %s\n", str );
        if( tr_bencDictFindInt( args, TR_PREFS_KEY_PEER_PORT, &i ) )
            printf( "  Listenport: %" PRId64 "\n", i );
        if( tr_bencDictFindBool( args, TR_PREFS_KEY_PORT_FORWARDING, &boolVal ) )
            printf( "  Portforwarding enabled: %s\n", ( boolVal ? "Yes" : "No" ) );
        if( tr_bencDictFindBool( args, TR_PREFS_KEY_PEX_ENABLED, &boolVal ) )
            printf( "  Peer exchange allowed: %s\n", ( boolVal ? "Yes" : "No" ) );
        if( tr_bencDictFindStr( args,  TR_PREFS_KEY_ENCRYPTION, &str ) )
            printf( "  Encryption: %s\n", str );
        printf( "\n" );

        {
            tr_bool altEnabled, altTimeEnabled, upEnabled, downEnabled;
            int64_t altDown, altUp, altBegin, altEnd, altDay, upLimit, downLimit, peerLimit;

            if( tr_bencDictFindInt ( args, TR_PREFS_KEY_ALT_SPEED_DOWN, &altDown ) &&
                tr_bencDictFindBool( args, TR_PREFS_KEY_ALT_SPEED_ENABLED, &altEnabled ) &&
                tr_bencDictFindInt ( args, TR_PREFS_KEY_ALT_SPEED_TIME_BEGIN, &altBegin ) &&
                tr_bencDictFindBool( args, TR_PREFS_KEY_ALT_SPEED_TIME_ENABLED, &altTimeEnabled ) &&
                tr_bencDictFindInt ( args, TR_PREFS_KEY_ALT_SPEED_TIME_END, &altEnd ) &&
                tr_bencDictFindInt ( args, TR_PREFS_KEY_ALT_SPEED_TIME_DAY, &altDay ) &&
                tr_bencDictFindInt ( args, TR_PREFS_KEY_ALT_SPEED_UP, &altUp ) &&
                tr_bencDictFindInt ( args, TR_PREFS_KEY_PEER_LIMIT_GLOBAL, &peerLimit ) &&
                tr_bencDictFindInt ( args, TR_PREFS_KEY_DSPEED, &downLimit ) &&
                tr_bencDictFindBool( args, TR_PREFS_KEY_DSPEED_ENABLED, &downEnabled ) &&
                tr_bencDictFindInt ( args, TR_PREFS_KEY_USPEED, &upLimit ) &&
                tr_bencDictFindBool( args, TR_PREFS_KEY_USPEED_ENABLED, &upEnabled ) )
            {
                char buf[128];

                printf( "LIMITS\n" );
                printf( "  Peer limit: %" PRId64 "\n", peerLimit );

                if( altEnabled )
                    tr_snprintf( buf, sizeof( buf ), "%"PRId64" KB/s", altUp );
                else if( upEnabled )
                    tr_snprintf( buf, sizeof( buf ), "%"PRId64" KB/s", upLimit );
                else
                    tr_strlcpy( buf, "Unlimited", sizeof( buf ) );
                printf( "  Upload speed limit: %s  (%s limit: %"PRId64" KB/s; %s turtle limit: %"PRId64" KB/s)\n",
                        buf,
                        (upEnabled?"Enabled":"Disabled"), upLimit,
                        (altEnabled?"Enabled":"Disabled"), altUp );

                if( altEnabled )
                    tr_snprintf( buf, sizeof( buf ), "%"PRId64" KB/s", altDown );
                else if( downEnabled )
                    tr_snprintf( buf, sizeof( buf ), "%"PRId64" KB/s", downLimit );
                else
                    tr_strlcpy( buf, "Unlimited", sizeof( buf ) );
                printf( "  Download speed limit: %s  (%s limit: %"PRId64" KB/s; %s turtle limit: %"PRId64" KB/s)\n",
                        buf,
                        (downEnabled?"Enabled":"Disabled"), downLimit,
                        (altEnabled?"Enabled":"Disabled"), altDown );

                if( altTimeEnabled ) {
                    printf( "  Turtle schedule: %02d:%02d - %02d:%02d  ",
                            (int)(altBegin/60), (int)(altBegin%60),
                            (int)(altEnd/60), (int)(altEnd%60) );
                    if( altDay & TR_SCHED_SUN )   printf( "Sun " );
                    if( altDay & TR_SCHED_MON )   printf( "Mon " );
                    if( altDay & TR_SCHED_TUES )  printf( "Tue " );
                    if( altDay & TR_SCHED_WED )   printf( "Wed " );
                    if( altDay & TR_SCHED_THURS ) printf( "Thu " );
                    if( altDay & TR_SCHED_FRI )   printf( "Fri " );
                    if( altDay & TR_SCHED_SAT )   printf( "Sat " );
                    printf( "\n" );
                }
            }
        }
    }
}

static void
printSessionStats( tr_benc * top )
{
    tr_benc *args, *d;
    if( ( tr_bencDictFindDict( top, "arguments", &args ) ) )
    {
        char buf[512];
        int64_t up, down, secs, sessions;

        if( tr_bencDictFindDict( args, "current-stats", &d )
            && tr_bencDictFindInt( d, "uploadedBytes", &up )
            && tr_bencDictFindInt( d, "downloadedBytes", &down )
            && tr_bencDictFindInt( d, "secondsActive", &secs ) )
        {
            printf( "\nCURRENT SESSION\n" );
            printf( "  Uploaded:   %s\n", strlsize( buf, up, sizeof( buf ) ) );
            printf( "  Downloaded: %s\n", strlsize( buf, down, sizeof( buf ) ) );
            printf( "  Ratio:      %s\n", strlratio( buf, up, down, sizeof( buf ) ) );
            printf( "  Duration:   %s\n", tr_strltime( buf, secs, sizeof( buf ) ) );
        }

        if( tr_bencDictFindDict( args, "cumulative-stats", &d )
            && tr_bencDictFindInt( d, "sessionCount", &sessions )
            && tr_bencDictFindInt( d, "uploadedBytes", &up )
            && tr_bencDictFindInt( d, "downloadedBytes", &down )
            && tr_bencDictFindInt( d, "secondsActive", &secs ) )
        {
            printf( "\nTOTAL\n" );
            printf( "  Started %lu times\n", (unsigned long)sessions );
            printf( "  Uploaded:   %s\n", strlsize( buf, up, sizeof( buf ) ) );
            printf( "  Downloaded: %s\n", strlsize( buf, down, sizeof( buf ) ) );
            printf( "  Ratio:      %s\n", strlratio( buf, up, down, sizeof( buf ) ) );
            printf( "  Duration:   %s\n", tr_strltime( buf, secs, sizeof( buf ) ) );
        }
    }
}

static void
printDetails( tr_benc * top )
{
    tr_benc *args, *torrents;

    if( ( tr_bencDictFindDict( top, "arguments", &args ) )
      && ( tr_bencDictFindList( args, "torrents", &torrents ) ) )
    {
        int ti, tCount;
        for( ti = 0, tCount = tr_bencListSize( torrents ); ti < tCount;
             ++ti )
        {
            tr_benc *    t = tr_bencListChild( torrents, ti );
            tr_benc *    l;
            const uint8_t * raw;
            size_t       rawlen;
            const char * str;
            char         buf[512];
            char         buf2[512];
            int64_t      i, j, k;
            tr_bool      boolVal;

            printf( "NAME\n" );
            if( tr_bencDictFindInt( t, "id", &i ) )
                printf( "  Id: %" PRId64 "\n", i );
            if( tr_bencDictFindStr( t, "name", &str ) )
                printf( "  Name: %s\n", str );
            if( tr_bencDictFindStr( t, "hashString", &str ) )
                printf( "  Hash: %s\n", str );
            printf( "\n" );

            printf( "TRANSFER\n" );
            getStatusString( t, buf, sizeof( buf ) );
            printf( "  State: %s\n", buf );

            if( tr_bencDictFindStr( t, "downloadDir", &str ) )
                printf( "  Location: %s\n", str );

            if( tr_bencDictFindInt( t, "sizeWhenDone", &i )
              && tr_bencDictFindInt( t, "leftUntilDone", &j ) )
            {
                strlratio( buf, 100.0 * ( i - j ), i, sizeof( buf ) );
                printf( "  Percent Done: %s%%\n", buf );
            }

            if( tr_bencDictFindInt( t, "eta", &i ) )
                printf( "  ETA: %s\n", tr_strltime( buf, i, sizeof( buf ) ) );
            if( tr_bencDictFindInt( t, "rateDownload", &i ) )
                printf( "  Download Speed: %.1f KB/s\n", i / 1024.0 );
            if( tr_bencDictFindInt( t, "rateUpload", &i ) )
                printf( "  Upload Speed: %.1f KB/s\n", i / 1024.0 );
            if( tr_bencDictFindInt( t, "haveUnchecked", &i )
              && tr_bencDictFindInt( t, "haveValid", &j ) )
            {
                strlsize( buf, i + j, sizeof( buf ) );
                strlsize( buf2, j, sizeof( buf2 ) );
                printf( "  Have: %s (%s verified)\n", buf, buf2 );
            }

            if( tr_bencDictFindInt( t, "sizeWhenDone", &i )
              && tr_bencDictFindInt( t, "totalSize", &j ) )
            {
                strlsize( buf, j, sizeof( buf ) );
                strlsize( buf2, i, sizeof( buf2 ) );
                printf( "  Total size: %s (%s wanted)\n", buf, buf2 );
            }
            if( tr_bencDictFindInt( t, "downloadedEver", &i )
              && tr_bencDictFindInt( t, "uploadedEver", &j ) )
            {
                strlsize( buf, i, sizeof( buf ) );
                printf( "  Downloaded: %s\n", buf );
                strlsize( buf, j, sizeof( buf ) );
                printf( "  Uploaded: %s\n", buf );
                strlratio( buf, j, i, sizeof( buf ) );
                printf( "  Ratio: %s\n", buf );
            }
            if( tr_bencDictFindInt( t, "corruptEver", &i ) )
            {
                strlsize( buf, i, sizeof( buf ) );
                printf( "  Corrupt DL: %s\n", buf );
            }
            if( tr_bencDictFindStr( t, "errorString", &str ) && str && *str &&
                tr_bencDictFindInt( t, "error", &i ) && i )
            {
                switch( i ) {
                    case TR_STAT_TRACKER_WARNING: printf( "  Tracker gave a warning: %s\n", str ); break;
                    case TR_STAT_TRACKER_ERROR:   printf( "  Tracker gave an error: %s\n", str ); break;
                    case TR_STAT_LOCAL_ERROR:     printf( "  Error: %s\n", str ); break;
                    default: break; /* no error */
                }
            }
            if( tr_bencDictFindInt( t, "peersConnected", &i )
              && tr_bencDictFindInt( t, "peersGettingFromUs", &j )
              && tr_bencDictFindInt( t, "peersSendingToUs", &k ) )
            {
                printf(
                    "  Peers: "
                    "connected to %" PRId64 ", "
                                            "uploading to %" PRId64
                    ", "
                    "downloading from %"
                    PRId64 "\n",
                    i, j, k );
            }

            if( tr_bencDictFindList( t, "webseeds", &l )
              && tr_bencDictFindInt( t, "webseedsSendingToUs", &i ) )
            {
                const int64_t n = tr_bencListSize( l );
                if( n > 0 )
                    printf(
                        "  Web Seeds: downloading from %" PRId64 " of %"
                        PRId64
                        " web seeds\n", i, n );
            }
            printf( "\n" );

            printf( "HISTORY\n" );
            if( tr_bencDictFindInt( t, "addedDate", &i ) && i )
            {
                const time_t tt = i;
                printf( "  Date added:      %s", ctime( &tt ) );
            }
            if( tr_bencDictFindInt( t, "doneDate", &i ) && i )
            {
                const time_t tt = i;
                printf( "  Date finished:   %s", ctime( &tt ) );
            }
            if( tr_bencDictFindInt( t, "startDate", &i ) && i )
            {
                const time_t tt = i;
                printf( "  Date started:    %s", ctime( &tt ) );
            }
            if( tr_bencDictFindInt( t, "activityDate", &i ) && i )
            {
                const time_t tt = i;
                printf( "  Latest activity: %s", ctime( &tt ) );
            }
            printf( "\n" );

            printf( "TRACKERS\n" );

            if( tr_bencDictFindList( t, "trackerStats", &l ) )
            {
                tr_benc * t;
                for( i=0; (( t = tr_bencListChild( l, i ))); ++i )
                {
                    int64_t downloadCount;
                    tr_bool hasAnnounced;
                    tr_bool hasScraped;
                    const char * host;
                    tr_bool isBackup;
                    int64_t lastAnnouncePeerCount;
                    const char * lastAnnounceResult;
                    int64_t lastAnnounceStartTime;
                    tr_bool lastAnnounceSucceeded;
                    int64_t lastAnnounceTime;
                    const char * lastScrapeResult;
                    tr_bool lastScrapeSucceeded;
                    int64_t lastScrapeStartTime;
                    int64_t lastScrapeTime;
                    int64_t leecherCount;
                    int64_t nextAnnounceTime;
                    int64_t nextScrapeTime;
                    int64_t seederCount;
                    int64_t tier;
                    int64_t announceState;
                    int64_t scrapeState;

                    if( tr_bencDictFindInt ( t, "downloadCount", &downloadCount ) &&
                        tr_bencDictFindBool( t, "hasAnnounced", &hasAnnounced ) &&
                        tr_bencDictFindBool( t, "hasScraped", &hasScraped ) &&
                        tr_bencDictFindStr ( t, "host", &host ) &&
                        tr_bencDictFindBool( t, "isBackup", &isBackup ) &&
                        tr_bencDictFindInt ( t, "announceState", &announceState ) &&
                        tr_bencDictFindInt ( t, "scrapeState", &scrapeState ) &&
                        tr_bencDictFindInt ( t, "lastAnnouncePeerCount", &lastAnnouncePeerCount ) &&
                        tr_bencDictFindStr ( t, "lastAnnounceResult", &lastAnnounceResult ) &&
                        tr_bencDictFindInt ( t, "lastAnnounceStartTime", &lastAnnounceStartTime ) &&
                        tr_bencDictFindBool( t, "lastAnnounceSucceeded", &lastAnnounceSucceeded ) &&
                        tr_bencDictFindInt ( t, "lastAnnounceTime", &lastAnnounceTime ) &&
                        tr_bencDictFindStr ( t, "lastScrapeResult", &lastScrapeResult ) &&
                        tr_bencDictFindInt ( t, "lastScrapeStartTime", &lastScrapeStartTime ) &&
                        tr_bencDictFindBool( t, "lastScrapeSucceeded", &lastScrapeSucceeded ) &&
                        tr_bencDictFindInt ( t, "lastScrapeTime", &lastScrapeTime ) &&
                        tr_bencDictFindInt ( t, "leecherCount", &leecherCount ) &&
                        tr_bencDictFindInt ( t, "nextAnnounceTime", &nextAnnounceTime ) &&
                        tr_bencDictFindInt ( t, "nextScrapeTime", &nextScrapeTime ) &&
                        tr_bencDictFindInt ( t, "seederCount", &seederCount ) &&
                        tr_bencDictFindInt ( t, "tier", &tier ) )
                    {
                        const time_t now = time( NULL );

                        printf( "\n" );
                        printf( "  Tracker #%d: %s\n", (int)(i+1), host );
                        if( isBackup )
                          printf( "  Backup on tier #%d\n", (int)tier );
                        else
                          printf( "  Active in tier #%d\n", (int)tier );

                        if( !isBackup )
                        {
                            if( hasAnnounced )
                            {
                                tr_strltime( buf, now - lastAnnounceTime, sizeof( buf ) );
                                if( lastAnnounceSucceeded )
                                    printf( "  Got a list of %'d peers %s ago\n",
                                            (int)lastAnnouncePeerCount, buf );
                                else
                                    printf( "  Got an error \"%s\" %s ago\n",
                                            lastAnnounceResult, buf );
                            }

                            switch( announceState )
                            {
                                case TR_TRACKER_INACTIVE:
                                    printf( "  No updates scheduled\n" );
                                    break;
                                case TR_TRACKER_WAITING:
                                    tr_strltime( buf, nextAnnounceTime - now, sizeof( buf ) );
                                    printf( "  Asking for more peers in %s\n", buf );
                                    break;
                                case TR_TRACKER_QUEUED:
                                    printf( "  Queued to ask for more peers\n" );
                                    break;
                                case TR_TRACKER_ACTIVE:
                                    tr_strltime( buf, now - lastAnnounceStartTime, sizeof( buf ) );
                                    printf( "  Asking for more peers now... %s\n", buf );
                                    break;
                            }

                            if( hasScraped )
                            {
                                tr_strltime( buf, now - lastScrapeTime, sizeof( buf ) );
                                if( lastScrapeSucceeded )
                                    printf( "  Tracker had %'d seeders and %'d leechers %s ago\n",
                                            (int)seederCount, (int)leecherCount, buf );
                                else
                                    printf( "  Got a scrape error \"%s\" %s ago\n",
                                            lastScrapeResult, buf );
                            }

                            switch( scrapeState )
                            {
                                case TR_TRACKER_INACTIVE:
                                    break;
                                case TR_TRACKER_WAITING:
                                    tr_strltime( buf, nextScrapeTime - now, sizeof( buf ) );
                                    printf( "  Asking for peer counts in %s\n", buf );
                                    break;
                                case TR_TRACKER_QUEUED:
                                    printf( "  Queued to ask for peer counts\n" );
                                    break;
                                case TR_TRACKER_ACTIVE:
                                    tr_strltime( buf, now - lastScrapeStartTime, sizeof( buf ) );
                                    printf( "  Asking for peer counts now... %s\n", buf );
                                    break;
                            }
                        }
                    }
                }
                printf( "\n" );
            }

            printf( "ORIGINS\n" );
            if( tr_bencDictFindInt( t, "dateCreated", &i ) && i )
            {
                const time_t tt = i;
                printf( "  Date created: %s", ctime( &tt ) );
            }
            if( tr_bencDictFindBool( t, "isPrivate", &boolVal ) )
                printf( "  Public torrent: %s\n", ( boolVal ? "No" : "Yes" ) );
            if( tr_bencDictFindStr( t, "comment", &str ) && str && *str )
                printf( "  Comment: %s\n", str );
            if( tr_bencDictFindStr( t, "creator", &str ) && str && *str )
                printf( "  Creator: %s\n", str );
            if( tr_bencDictFindInt( t, "pieceCount", &i ) )
                printf( "  Piece Count: %" PRId64 "\n", i );
            if( tr_bencDictFindInt( t, "pieceSize", &i ) )
                printf( "  Piece Size: %" PRId64 "\n", i );
            printf( "\n" );

            printf( "LIMITS\n" );
            if( tr_bencDictFindBool( t, "downloadLimited", &boolVal )
                && tr_bencDictFindInt( t, "downloadLimit", &i ) )
            {
                printf( "  Download Limit: " );
                if( boolVal )
                    printf( "%" PRId64 " KB/s\n", i );
                else
                    printf( "Unlimited\n" );
            }
            if( tr_bencDictFindBool( t, "uploadLimited", &boolVal )
                && tr_bencDictFindInt( t, "uploadLimit", &i ) )
            {
                printf( "  Upload Limit: " );
                if( boolVal )
                    printf( "%" PRId64 " KB/s\n", i );
                else
                    printf( "Unlimited\n" );
            }
            if( tr_bencDictFindBool( t, "honorsSessionLimits", &boolVal ) )
                printf( "  Honors Session Limits: %s\n", ( boolVal ? "Yes" : "No" ) );
            if( tr_bencDictFindInt ( t, "peer-limit", &i ) )
                printf( "  Peer limit: %" PRId64 "\n", i );
            printf( "\n" );

            printf( "PIECES\n" );
            if( tr_bencDictFindRaw( t, "pieces", &raw, &rawlen ) && tr_bencDictFindInt( t, "pieceCount", &j ) ) {
                int len;
                char * str = tr_base64_decode( raw, rawlen, &len );
                printf( "  " );
                for( i=k=0; k<len; ++k ) {
                    int e;
                    for( e=0; i<j && e<8; ++e, ++i )
                        printf( "%c", str[k] & (1<<(7-e)) ? '1' : '0' );
                    printf( " " );
                    if( !(i%64) )
                        printf( "\n  " );
                }
                tr_free( str );
            }
            printf( "\n" );
        }
    }
}

static void
printFileList( tr_benc * top )
{
    tr_benc *args, *torrents;

    if( ( tr_bencDictFindDict( top, "arguments", &args ) )
      && ( tr_bencDictFindList( args, "torrents", &torrents ) ) )
    {
        int i, in;
        for( i = 0, in = tr_bencListSize( torrents ); i < in; ++i )
        {
            tr_benc *    d = tr_bencListChild( torrents, i );
            tr_benc *    files, *priorities, *wanteds;
            const char * name;
            if( tr_bencDictFindStr( d, "name", &name )
              && tr_bencDictFindList( d, "files", &files )
              && tr_bencDictFindList( d, "priorities", &priorities )
              && tr_bencDictFindList( d, "wanted", &wanteds ) )
            {
                int j = 0, jn = tr_bencListSize( files );
                printf( "%s (%d files):\n", name, jn );
                printf( "%3s  %4s %8s %3s %9s  %s\n", "#", "Done",
                        "Priority", "Get", "Size",
                        "Name" );
                for( j = 0, jn = tr_bencListSize( files ); j < jn; ++j )
                {
                    int64_t      have;
                    int64_t      length;
                    int64_t      priority;
                    int64_t      wanted;
                    const char * filename;
                    tr_benc *    file = tr_bencListChild( files, j );
                    if( tr_bencDictFindInt( file, "length", &length )
                      && tr_bencDictFindStr( file, "name", &filename )
                      && tr_bencDictFindInt( file, "bytesCompleted", &have )
                      && tr_bencGetInt( tr_bencListChild( priorities,
                                                          j ), &priority )
                      && tr_bencGetInt( tr_bencListChild( wanteds,
                                                          j ), &wanted ) )
                    {
                        char         sizestr[64];
                        double       percent = (double)have / length;
                        const char * pristr;
                        strlsize( sizestr, length, sizeof( sizestr ) );
                        switch( priority )
                        {
                            case TR_PRI_LOW:
                                pristr = "Low"; break;

                            case TR_PRI_HIGH:
                                pristr = "High"; break;

                            default:
                                pristr = "Normal"; break;
                        }
                        printf( "%3d: %3.0f%% %-8s %-3s %9s  %s\n",
                                j,
                                floor( 100.0 * percent ),
                                pristr,
                                ( wanted ? "Yes" : "No" ),
                                sizestr,
                                filename );
                    }
                }
            }
        }
    }
}

static void
printPeersImpl( tr_benc * peers )
{
    int i, n;
    printf( "%-20s  %-12s  %-5s %-6s  %-6s  %s\n",
            "Address", "Flags", "Done", "Down", "Up", "Client" );
    for( i = 0, n = tr_bencListSize( peers ); i < n; ++i )
    {
        double progress;
        const char * address, * client, * flagstr;
        int64_t rateToClient, rateToPeer;
        tr_benc * d = tr_bencListChild( peers, i );

        if( tr_bencDictFindStr( d, "address", &address )
          && tr_bencDictFindStr( d, "clientName", &client )
          && tr_bencDictFindReal( d, "progress", &progress )
          && tr_bencDictFindStr( d, "flagStr", &flagstr )
          && tr_bencDictFindInt( d, "rateToClient", &rateToClient )
          && tr_bencDictFindInt( d, "rateToPeer", &rateToPeer ) )
        {
            printf( "%-20s  %-12s  %-5.1f %6.1f  %6.1f  %s\n",
                    address, flagstr, (progress*100.0),
                    rateToClient / 1024.0,
                    rateToPeer / 1024.0,
                    client );
        }
    }
}

static void
printPeers( tr_benc * top )
{
    tr_benc *args, *torrents;

    if( tr_bencDictFindDict( top, "arguments", &args )
      && tr_bencDictFindList( args, "torrents", &torrents ) )
    {
        int i, n;
        for( i=0, n=tr_bencListSize( torrents ); i<n; ++i )
        {
            tr_benc * peers;
            tr_benc * torrent = tr_bencListChild( torrents, i );
            if( tr_bencDictFindList( torrent, "peers", &peers ) ) {
                printPeersImpl( peers );
                if( i+1<n )
                    printf( "\n" );
            }
        }
    }
}

static void
printTorrentList( tr_benc * top )
{
    tr_benc *args, *list;

    if( ( tr_bencDictFindDict( top, "arguments", &args ) )
      && ( tr_bencDictFindList( args, "torrents", &list ) ) )
    {
        int i, n;
        int64_t total_up = 0, total_down = 0, total_size = 0;
        char haveStr[32];

        printf( "%-4s   %-4s  %9s  %-8s  %6s  %6s  %-5s  %-11s  %s\n",
                "ID", "Done", "Have", "ETA", "Up", "Down", "Ratio", "Status",
                "Name" );

        for( i = 0, n = tr_bencListSize( list ); i < n; ++i )
        {
            int64_t      id, eta, status, up, down;
            int64_t      sizeWhenDone, leftUntilDone;
            double       ratio;
            const char * name;
            tr_benc *   d = tr_bencListChild( list, i );
            if( tr_bencDictFindInt( d, "eta", &eta )
              && tr_bencDictFindInt( d, "id", &id )
              && tr_bencDictFindInt( d, "leftUntilDone", &leftUntilDone )
              && tr_bencDictFindStr( d, "name", &name )
              && tr_bencDictFindInt( d, "rateDownload", &down )
              && tr_bencDictFindInt( d, "rateUpload", &up )
              && tr_bencDictFindInt( d, "sizeWhenDone", &sizeWhenDone )
              && tr_bencDictFindInt( d, "status", &status )
              && tr_bencDictFindReal( d, "uploadRatio", &ratio ) )
            {
                char etaStr[16];
                char statusStr[64];
                char ratioStr[32];
                char doneStr[8];
                int64_t error;
                char errorMark;

                if( sizeWhenDone )
                    tr_snprintf( doneStr, sizeof( doneStr ), "%d%%", (int)( 100.0 * ( sizeWhenDone - leftUntilDone ) / sizeWhenDone ) );
                else
                    tr_strlcpy( doneStr, "n/a", sizeof( doneStr ) );

                strlsize( haveStr, sizeWhenDone - leftUntilDone, sizeof( haveStr ) );

                if( leftUntilDone || eta != -1 )
                    etaToString( etaStr, sizeof( etaStr ), eta );
                else
                    tr_snprintf( etaStr, sizeof( etaStr ), "Done" );
                if( tr_bencDictFindInt( d, "error", &error ) && error )
                    errorMark = '*';
                else
                    errorMark = ' ';
                printf(
                    "%4d%c  %4s  %9s  %-8s  %6.1f  %6.1f  %5s  %-11s  %s\n",
                    (int)id, errorMark,
                    doneStr,
                    haveStr,
                    etaStr,
                    up / 1024.0,
                    down / 1024.0,
                    strlratio2( ratioStr, ratio, sizeof( ratioStr ) ),
                    getStatusString( d, statusStr, sizeof( statusStr ) ),
                    name );

                total_up += up;
                total_down += down;
                total_size += sizeWhenDone - leftUntilDone;
            }
        }

        printf( "Sum:         %9s            %6.1f  %6.1f\n",
                strlsize( haveStr, total_size, sizeof( haveStr ) ),
                total_up / 1024.0,
                total_down / 1024.0 );
    }
}

static int
processResponse( const char * host,
                 int          port,
                 const void * response,
                 size_t       len )
{
    tr_benc top;
    int status = EXIT_SUCCESS;

    if( debug )
        fprintf( stderr, "got response (len %d):\n--------\n%*.*s\n--------\n",
                 (int)len, (int)len, (int)len, (const char*) response );

    if( tr_jsonParse( NULL, response, len, &top, NULL ) )
    {
        tr_nerr( MY_NAME, "Unable to parse response \"%*.*s\"", (int)len,
                 (int)len, (char*)response );
        status |= EXIT_FAILURE;
    }
    else
    {
        int64_t      tag = -1;
        const char * str;
        tr_bencDictFindInt( &top, "tag", &tag );

        switch( tag )
        {
            case TAG_SESSION:
                printSession( &top ); break;

            case TAG_STATS:
                printSessionStats( &top ); break;

            case TAG_FILES:
                printFileList( &top ); break;

            case TAG_DETAILS:
                printDetails( &top ); break;

            case TAG_LIST:
                printTorrentList( &top ); break;

            case TAG_PEERS:
                printPeers( &top ); break;

            default:
                if( !tr_bencDictFindStr( &top, "result", &str ) )
                    status |= EXIT_FAILURE;
                else {
                    printf( "%s:%d responded: \"%s\"\n", host, port, str );
                    if( strcmp( str, "success") )
                        status |= EXIT_FAILURE;
                }
        }

        tr_bencFree( &top );
    }

    return status;
}

/* look for a session id in the header in case the server gives back a 409 */
static size_t
parseResponseHeader( void *ptr, size_t size, size_t nmemb, void * stream UNUSED )
{
    const char * line = ptr;
    const size_t line_len = size * nmemb;
    const char * key = TR_RPC_SESSION_ID_HEADER ": ";
    const size_t key_len = strlen( key );

    if( ( line_len >= key_len ) && !memcmp( line, key, key_len ) )
    {
        const char * begin = line + key_len;
        const char * end = begin;
        while( !isspace( *end ) )
            ++end;
        tr_free( sessionId );
        sessionId = tr_strndup( begin, end-begin );
    }

    return line_len;
}

static CURL*
tr_curl_easy_init( struct evbuffer * writebuf )
{
    CURL * curl = curl_easy_init( );
    curl_easy_setopt( curl, CURLOPT_USERAGENT, MY_NAME "/" LONG_VERSION_STRING );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, writeFunc );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, writebuf );
    curl_easy_setopt( curl, CURLOPT_HEADERFUNCTION, parseResponseHeader );
    curl_easy_setopt( curl, CURLOPT_POST, 1 );
    curl_easy_setopt( curl, CURLOPT_NETRC, CURL_NETRC_OPTIONAL );
    curl_easy_setopt( curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY );
    curl_easy_setopt( curl, CURLOPT_TIMEOUT, 60L );
    curl_easy_setopt( curl, CURLOPT_VERBOSE, debug );
    curl_easy_setopt( curl, CURLOPT_ENCODING, "" ); /* "" tells curl to fill in the blanks with what it was compiled to support */
    if( netrc )
        curl_easy_setopt( curl, CURLOPT_NETRC_FILE, netrc );
    if( auth )
        curl_easy_setopt( curl, CURLOPT_USERPWD, auth );
    if( sessionId ) {
        char * h = tr_strdup_printf( "%s: %s", TR_RPC_SESSION_ID_HEADER, sessionId );
        struct curl_slist * custom_headers = curl_slist_append( NULL, h );
        curl_easy_setopt( curl, CURLOPT_HTTPHEADER, custom_headers );
        /* fixme: leaks */
    }
    return curl;
}


static int
processRequests( const char *  host,
                 int           port,
                 const char ** reqs,
                 int           reqCount )
{
    int i;
    CURL * curl = NULL;
    struct evbuffer * buf = evbuffer_new( );
    char * url = tr_strdup_printf( "http://%s:%d/transmission/rpc", host, port );
    int status = EXIT_SUCCESS;

    for( i=0; i<reqCount; ++i )
    {
        CURLcode res;
        evbuffer_drain( buf, EVBUFFER_LENGTH( buf ) );

        if( curl == NULL )
        {
            curl = tr_curl_easy_init( buf );
            curl_easy_setopt( curl, CURLOPT_URL, url );
        }

        curl_easy_setopt( curl, CURLOPT_POSTFIELDS, reqs[i] );

        if( debug )
            fprintf( stderr, "posting:\n--------\n%s\n--------\n", reqs[i] );
        if( ( res = curl_easy_perform( curl ) ) )
        {
            tr_nerr( MY_NAME, "(%s:%d) %s", host, port, curl_easy_strerror( res ) );
            status |= EXIT_FAILURE;
        }
        else {
            long response;
            curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &response );
            switch( response ) {
                case 200:
                    status |= processResponse( host, port, EVBUFFER_DATA(buf), EVBUFFER_LENGTH(buf) );
                    break;
                case 409:
                    /* session id failed.  our curl header func has already
                     * pulled the new session id from this response's headers,
                     * build a new CURL* and try again */
                    curl_easy_cleanup( curl );
                    curl = NULL;
                    --i;
                    break;
                default:
                    fprintf( stderr, "Unexpected response: %s\n", (char*)EVBUFFER_DATA(buf) );
                    status |= EXIT_FAILURE;
                    break;
            }
        }
    }

    /* cleanup */
    tr_free( url );
    evbuffer_free( buf );
    if( curl != NULL )
        curl_easy_cleanup( curl );
    return status;
}

int
main( int     argc,
      char ** argv )
{
    int    i;
    int    port = DEFAULT_PORT;
    char * host = NULL;
    int    exit_status = EXIT_SUCCESS;

    if( argc < 2 ) {
        showUsage( );
        return EXIT_FAILURE;
    }

    getHostAndPort( &argc, argv, &host, &port );
    if( host == NULL )
        host = tr_strdup( DEFAULT_HOST );

    readargs( argc, (const char**)argv );
    if( reqCount == 0 ) {
        showUsage( );
        return EXIT_FAILURE;
    }

    exit_status = processRequests( host, port, (const char**)reqs, reqCount );

    for( i=0; i<reqCount; ++i )
        tr_free( reqs[i] );

    tr_free( host );
    return exit_status;
}

