/*
 * This file Copyright (C) 2007-2010 Mnemosyne LLC
 *
 * This file is licensed by the GPL version 2.  Works owned by the
 * Transmission project are granted a special exemption to clause 2(b)
 * so that the bulk of its code can remain under the MIT license.
 * This exemption does not extend to derived works not owned by
 * the Transmission project.
 *
 * $Id: details.h 9868 2010-01-04 21:00:47Z charles $
 */

#ifndef GTK_TORRENT_INSPECTOR_H
#define GTK_TORRENT_INSPECTOR_H

#include <gtk/gtk.h>
#include "tr-core.h"
#include "tr-torrent.h"

GtkWidget* torrent_inspector_new( GtkWindow * parent,
                                  TrCore    * core );

void torrent_inspector_set_torrents( GtkWidget * widgets,
                                     GSList    * torrent_ids );

#endif /* TG_PREFS_H */
