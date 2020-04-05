/*
 * This file Copyright (C) 2009-2010 Mnemosyne LLC
 *
 * This file is licensed by the GPL version 2.  Works owned by the
 * Transmission project are granted a special exemption to clause 2(b)
 * so that the bulk of its code can remain under the MIT license.
 * This exemption does not extend to derived works not owned by
 * the Transmission project.
 *
 * $Id: file-list.h 9868 2010-01-04 21:00:47Z charles $
 */

#ifndef GTK_TORRENT_FILE_LIST_H
#define GTK_TORRENT_FILE_LIST_H

#include <gtk/gtk.h>
#include "tr-core.h"

GtkWidget * file_list_new( TrCore *, int torrentId );

void        file_list_clear( GtkWidget * );

void        file_list_set_torrent( GtkWidget *, int torrentId );

#endif
