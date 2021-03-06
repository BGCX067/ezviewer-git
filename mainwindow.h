/****************************************************************************
 * EZ Viewer
 * Copyright (C) 2012 huangezhao. CHINA.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "picmanager.h"

class FloatFrame;
class ContralBar;
class QLabel;
class QMenu;
class QAction;
class QPushButton;
class QFileSystemWatcher;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

    void openFiles(const QStringList &list) { viewer->openFiles(list);}

public slots:
    void openFile();

protected slots:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent * event);
    void dropEvent(QDropEvent * event);
    void keyPressEvent(QKeyEvent *e);
    void resizeEvent ( QResizeEvent * event );

    void moveWindow(const QPoint &change) { if(!isFullScreen()) move(pos() + change); }
    void applyConfig();
    void setMyWindowTitle(const QString &title = QString::null);
    void showContextMenu(const QPoint &pos);

    void changeFullScreen();
    void showAttribute();
    void switchSlideShow();
    void setting();
    void about();

    void nextPic()          { viewer->nextPic(); }
    void prePic()           { viewer->prePic(); }
    void rotateLeft()       { viewer->rotateLeft(); }
    void rotateRight()      { viewer->rotateRight(); }
    void mirrorHorizontal() { viewer->mirrorHorizontal(); }
    void mirrorVertical()   { viewer->mirrorVertical(); }
    void switchGifPause()   { viewer->switchGifPause(); }
    void nextGifFrame()     { viewer->nextGifFrame(); }
    void copyToClipboard()  { viewer->copyToClipboard(); }
    void deleteFileAsk()    { viewer->deleteFileAsk(); }
    void deleteFileNoAsk()  { viewer->deleteFileNoAsk(); }
//    void closeWindow()      { close(); }

private:
    void readSettings();
    void writeSettings();
    void watchConfigFile();

    void initContextMenu();
    void initButtomBar();
    void initSwitchFrame(); // init left and right float frame

    void changeTimerInterval(int sec) { slideTimer->setInterval(sec * 1000); }

private:
    PicManager *viewer;
    bool WasMaximized;
    QTimer *slideTimer;
    int slideInterval;//msec

    QFileSystemWatcher *cfgWatcher;

    FloatFrame *buttomFrame;
    FloatFrame *leftFrame;
    FloatFrame *rightFrame;

    ContralBar *contralBar;
    QPushButton *settingButton;
    QPushButton *openButton;
    QPushButton *preButton;
    QPushButton *playButton;
    QPushButton *nextButton;
    QPushButton *rotateLeftButton;
    QPushButton *rotateRightButton;
    QPushButton *deleteButton;

    QMenu *contextMenu;
    QAction *openAction;
    QAction *slideAction;
    QAction *rotateLeftAction;
    QAction *rotateRightAction;
    QAction *mirrorHAction;
    QAction *mirrorVAction;
    QAction *copyAction;
    QAction *deleteAction;
    QAction *attributeAction;
};


#endif // MAINWINDOW_H
