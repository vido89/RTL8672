/*
 * This file Copyright (C) 2007-2010 Mnemosyne LLC
 *
 * This file is licensed by the GPL version 2.  Works owned by the
 * Transmission project are granted a special exemption to clause 2(b)
 * so that the bulk of its code can remain under the MIT license.
 * This exemption does not extend to derived works not owned by
 * the Transmission project.
 *
 * $Id: tr-icon.h 9868 2010-01-04 21:00:47Z charles $
 */

#ifndef TR_ICON_H
#define TR_ICON_H

#include <gtk/gtk.h>
#include "tr-core.h"

#if GTK_CHECK_VERSION( 2, 10, 0 )
 #define STATUS_ICON_SUPPORTED
#endif

gpointer tr_icon_new( TrCore * core );

void tr_icon_refresh( gpointer );

#endif
