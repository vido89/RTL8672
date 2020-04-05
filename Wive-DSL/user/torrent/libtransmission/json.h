/*
 * This file Copyright (C) 2008-2010 Mnemosyne LLC
 *
 * This file is licensed by the GPL version 2.  Works owned by the
 * Transmission project are granted a special exemption to clause 2(b)
 * so that the bulk of its code can remain under the MIT license.
 * This exemption does not extend to derived works not owned by
 * the Transmission project.
 *
 * $Id: json.h 9868 2010-01-04 21:00:47Z charles $
 */

#ifndef TR_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup tr_benc */
int tr_jsonParse( const char * source, /* such as a filename.  only when logging an error */
                  const void *     vbuf,
                  size_t           len,
                  struct tr_benc * setme_benc,
                  const uint8_t ** setme_end );

#ifdef __cplusplus
}
#endif

#endif
