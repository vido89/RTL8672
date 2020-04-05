/*
 * This file Copyright (C) 2009-2010 Mnemosyne LLC
 *
 * This file is licensed by the GPL version 2.  Works owned by the
 * Transmission project are granted a special exemption to clause 2(b)
 * so that the bulk of its code can remain under the MIT license.
 * This exemption does not extend to derived works not owned by
 * the Transmission project.
 *
 * $Id: options.h 9868 2010-01-04 21:00:47Z charles $
 */

#ifndef OPTIONS_DIALOG_H
#define OPTIONS_DIALOG_H

#include <QDialog>
#include <QEvent>
#include <QString>
#include <QDir>
#include <QVector>
#include <QMap>
#include <QPushButton>
#include <QStringList>
#include <QCryptographicHash>
#include <QFile>
#include <QTimer>

#include "file-tree.h"

class FileTreeView;
class Prefs;
class QCheckBox;
class Session;

extern "C" { struct tr_benc; };

class FileAdded: public QObject
{
        Q_OBJECT
        const int64_t myTag;
        QString myDelFile;

    public:
        FileAdded( int tag, const QString file ): myTag(tag), myDelFile(file) { }
        ~FileAdded( ) { }

    public slots:
        void executed( int64_t tag, const QString& result, struct tr_benc * arguments ) {
            Q_UNUSED( arguments );
            if( tag == myTag ) {
                if( result == "success" )
                    QFile( myDelFile ).remove( );
                deleteLater();
            }
        }
};

class Options: public QDialog
{
        Q_OBJECT

    public:
        Options( Session& session, const Prefs& prefs, const QString& filename, QWidget * parent = 0 );
        ~Options( );

    private:
        void reload( );
        void clearInfo( );
        void refreshFileButton( int width=-1 );
        void refreshDestinationButton( int width=-1 );
        void refreshButton( QPushButton *, const QString&, int width=-1 );

    private:
        Session& mySession;
        QString myFile;
        QDir myDestination;
        bool myHaveInfo;
        tr_info myInfo;
        FileTreeView * myTree;
        QCheckBox * myStartCheck;
        QCheckBox * myTrashCheck;
        QPushButton * myFileButton;
        QPushButton * myDestinationButton;
        QPushButton * myVerifyButton;
        QVector<int> myPriorities;
        QVector<bool> myWanted;
        FileList myFiles;

    private slots:
        void onAccepted( );
        void onPriorityChanged( const QSet<int>& fileIndices, int );
        void onWantedChanged( const QSet<int>& fileIndices, bool );
        void onVerify( );
        void onTimeout( );
        void onFilenameClicked( );
        void onDestinationClicked( );
        void onFilesSelected( const QStringList& );
        void onDestinationsSelected( const QStringList& );

    private:
        bool eventFilter( QObject *, QEvent * );

    private:
        QTimer myVerifyTimer;
        char myVerifyBuf[2048*4];
        QFile myVerifyFile;
        uint64_t myVerifyFilePos;
        int myVerifyFileIndex;
        uint32_t myVerifyPieceIndex;
        uint32_t myVerifyPiecePos;
        void clearVerify( );
        QVector<bool> myVerifyFlags;
        QCryptographicHash myVerifyHash;
        typedef QMap<uint32_t,int32_t> mybins_t;
        mybins_t myVerifyBins;

};

#endif
