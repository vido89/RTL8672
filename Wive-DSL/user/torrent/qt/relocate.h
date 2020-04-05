/*
 * This file Copyright (C) 2009-2010 Mnemosyne LLC
 *
 * This file is licensed by the GPL version 2.  Works owned by the
 * Transmission project are granted a special exemption to clause 2(b)
 * so that the bulk of its code can remain under the MIT license.
 * This exemption does not extend to derived works not owned by
 * the Transmission project.
 *
 * $Id: relocate.h 9868 2010-01-04 21:00:47Z charles $
 */

#ifndef RELOCATE_DIALOG_H
#define RELOCATE_DIALOG_H

#include <QDialog>
#include <QSet>
#include <QString>

class QPushButton;
class QRadioButton;
class Session;

class RelocateDialog: public QDialog
{
        Q_OBJECT

    private:
        static QString myPath;
        static bool myMoveFlag;

    private:
        Session & mySession;
        QSet<int> myIds;
        QPushButton * myDirButton;
        QRadioButton * myMoveRadio;

    private slots:
        void onFileSelected( const QString& path );
        void onDirButtonClicked( );
        void onSetLocation( );
        void onMoveToggled( bool );

    public:
        RelocateDialog( Session&, const QSet<int>& ids, QWidget * parent = 0 );
        ~RelocateDialog( ) { }
};

#endif
