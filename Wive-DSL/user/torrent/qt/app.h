/*
 * This file Copyright (C) 2009-2010 Mnemosyne LLC
 *
 * This file is licensed by the GPL version 2.  Works owned by the
 * Transmission project are granted a special exemption to clause 2(b)
 * so that the bulk of its code can remain under the MIT license.
 * This exemption does not extend to derived works not owned by
 * the Transmission project.
 *
 * $Id: app.h 9868 2010-01-04 21:00:47Z charles $
 */

#ifndef QTR_APP_H
#define QTR_APP_H

#include <QApplication>
#include <QTimer>

class Prefs;
class Session;
class TorrentModel;
class TrMainWindow;
class WatchDir;

class MyApp: public QApplication
{
        Q_OBJECT

    public:
        MyApp( int& argc, char ** argv );
        virtual ~MyApp( );

    private:
        Prefs * myPrefs;
        Session * mySession;
        TorrentModel * myModel;
        TrMainWindow * myWindow;
        WatchDir * myWatchDir;
        QTimer myModelTimer;
        QTimer myStatsTimer;
        QTimer mySessionTimer;
        time_t myLastFullUpdateTime;

    private slots:
        void consentGiven( );
        void refreshPref( int key );
        void refreshTorrents( );
        void addTorrent( const QString& );

    private:
        void maybeUpdateBlocklist( );
};

#endif
