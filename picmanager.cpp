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

#include "picmanager.h"

#include "global.h"
#include "osrelated.h"
#include "toolkit.h"

#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QFile>


PicManager::PicManager(QWidget *parent)
    : ImageViewer(parent), curCache(new ImageCache()),
      listMode(FileNameListMode), currentIndex(-1), fsWatcher(this)
{
//    state = NoFileNoPicture;
    connect(&fsWatcher, SIGNAL(directoryChanged(QString)),
            SLOT(directoryChanged()));
    connect(&fsWatcher, SIGNAL(fileChanged(QString)),
            SLOT(fileChanged(QString)));
}

PicManager::~PicManager()
{
    SafeDelete(curCache);
}

void PicManager::updateFileNameList(const QString &curfile)
{
    QFileInfo fileInfo(curfile);
    curDir = fileInfo.absolutePath();
    list = QDir(curDir, SUPPORT_FORMAT, QDir_SORT_FLAG, QDir::Files)
            .entryList();
    currentIndex = list.indexOf(fileInfo.fileName());
}

void PicManager::updateFullPathList(const QString &file)
{
    int index = list.indexOf(file);
    //! 如果文件名已经过滤并且简化，则可以不用验证index != -1 ??
    if(!QFile::exists(file) && index != -1)
        list.removeAt(index);
    currentIndex = list.indexOf(file);
}

void PicManager::updateFileIndex(int oldIndex)
{
    if(currentIndex != -1) //! verify if currentIndex is valid.
        return;

    if(list.isEmpty()){
        noFileToShow();
        return;
    }

    if(oldIndex >= list.size())
        currentIndex = list.size() - 1;
    else if(oldIndex != -1)
        currentIndex = oldIndex;
    else
        currentIndex = 0;

    readFile(list.at(currentIndex));
}

void PicManager::directoryChanged()
{
    int oldIndex = currentIndex;
    updateFileNameList(curPath);
    updateFileIndex(oldIndex);
}

void PicManager::fileChanged(const QString &filePath)
{
    int oldIndex = currentIndex;
    updateFullPathList(filePath);
    updateFileIndex(oldIndex);
}

void PicManager::readFile(const QString &file)
{
    QString Seperator(curDir.endsWith('/') ? "" : "/");
    bool isAbsolute = file.contains('/') || file.contains('\\'); ///
    QString path(isAbsolute ? file : curDir + Seperator + file);

    QFileInfo fileInfo(path);

    //! must test if hasPicture() !
    if(/*hasPicture() && */ curPath == fileInfo.absoluteFilePath() )//! if the image needs refresh?
        return;

    curPath = fileInfo.absoluteFilePath();
    curName = fileInfo.fileName();

    SafeDelete(curCache);

    curCache = ImageCache::getCache(curPath);

    if(curCache->movie){
        if(curCache->movie->state() == QMovie::NotRunning)
            curCache->movie->start();
        connect(curCache->movie, SIGNAL(updated(QRect)), SLOT(updateGifImage()));
    }

    QString msg = curCache->image.isNull() ? tr("Cannot load picture:\n'%1'.")
                                      .arg(curPath) : QString::null;
    loadImage(curCache->image, msg);
//     state = image.isNull() ? FileNoPicture : FilePicture;
    emit fileNameChange(curName);
}

void PicManager::noFileToShow()
{
    curCache->image = QImage();
    curPath = curName = QString::null;
//    state = NoFileNoPicture;
    loadImage(curCache->image);
    emit fileNameChange("");    //
}

void PicManager::openFile(const QString &file)
{
//    if(!QFileInfo(file).isFile()) return;

    QStringList oldWatchList(fsWatcher.files() + fsWatcher.directories());
    if(!oldWatchList.empty())
        fsWatcher.removePaths(oldWatchList);

    listMode = FileNameListMode;
    updateFileNameList(file);
    // make sure if file is no a picture, this will show error message.
    readFile(file); //readFile(list.at(currentIndex));

    fsWatcher.addPath(curDir);
    if(!QFileInfo(curDir).isRoot())// watch the parent dir, will get notice when rename current dir.
        fsWatcher.addPath(QFileInfo(curDir).absolutePath());
}

void PicManager::openFiles(const QStringList &fileList)
{
    //! ?? check if is file and if is exist, remove from list if no file.
    if(fileList.empty()) return;
    if(fileList.size() == 1){
        openFile(fileList.first());
        qDebug("list only one file: %s", qPrintable(fileList.first()));
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QStringList oldWatchList(fsWatcher.files() + fsWatcher.directories());
    if(!oldWatchList.empty())
        fsWatcher.removePaths(oldWatchList);

    listMode = FullPathListMode;
    curDir = "";
    list = fileList;
    currentIndex = 0;
    readFile(list.at(currentIndex));

    fsWatcher.addPaths(list);       //放到另一个线程中？？？

    QApplication::restoreOverrideCursor();
}

bool PicManager::prePic()
{
    // maybe current file is not a picture, and current dir has no any picture also, so we need check if(list.size() < 1).
    if(!hasFile() || list.size() < 1) return false;

    if(currentIndex - 1 < 0) //arrive the head of file list or source file is deleted.
        currentIndex = list.size();
    readFile(list.at(--currentIndex));
    return true;
}

bool PicManager::nextPic()
{
    if(!hasFile() || list.size() < 1) return false; //

    if(currentIndex + 1 == list.size()) //arrive the end of the file list
        currentIndex = -1;
    readFile(list.at(++currentIndex));
    return true;
}

void PicManager::switchGifPause()
{
    QMovie * &movie = curCache->movie;
    if(movie)
        switch(movie->state()){
        case QMovie::Running:
            movie->setPaused(true);
            break;
        case QMovie::Paused:
            movie->setPaused(false);
            break;
        case QMovie::NotRunning:
            break;
        }
}

void PicManager::nextGifFrame()
{
    QMovie * &movie = curCache->movie;
    if(movie)
        switch(movie->state()){
        case QMovie::Running:
            movie->setPaused(true);
            break;
        case QMovie::Paused:
            movie->jumpToNextFrame();
            break;
        case QMovie::NotRunning:
            break;
        }
}

void PicManager::setGifPaused(bool paused)
{
    if(curCache->movie)
        curCache->movie->setPaused(paused);
}

void PicManager::updateGifImage()
{
    curCache->image = curCache->movie->currentImage();
    updatePixmap(curCache->image);
}

void PicManager::hideEvent ( QHideEvent * event )
{
    setGifPaused(true);
}

void PicManager::showEvent ( QShowEvent * event )
{
    setGifPaused(false);
}

void PicManager::deleteFile(bool needAsk)
{
    if(!hasFile() || !QFile::exists(curPath)) return;

    if(needAsk){
        int ret = QMessageBox::question(
                    0, tr("Delete File"),
                    tr("Are you sure to delete file '%1'?").arg(curName),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if(ret == QMessageBox::No)
            return;
    }

    if(curCache->movie) SafeDelete(curCache->movie); //! gif image: must free movie before delete file.

    OSRelated::moveFile2Trash(curPath); ///
}

QString PicManager::attribute() const
{
    QString info;
    QFileInfo fileInfo(curPath);

    if(fileInfo.exists()){
        const QString timeFormat(tr("yyyy-MM-dd, hh:mm:ss"));
        qint64 size = fileInfo.size();
        QString sizeStr = ToolKit::fileSize2Str(size);

        info += tr("File Name: %1<br>").arg(curName);
        info += tr("File Size: %1 (%2 Bytes)<br>").arg(sizeStr).arg(size);
        info += tr("Created Time: %1<br>")
                .arg(fileInfo.created().toString(timeFormat));
        info += tr("Modified Time: %1<br>")
                .arg(fileInfo.lastModified().toString(timeFormat));
        info += tr("Last Read: %1")
                .arg(fileInfo.lastRead().toString(timeFormat));
        if(!curCache->format.isEmpty())
            info += tr("<br>Image Format: %1").arg(curCache->format);
    }

    if(!currentImage().isNull()){
        if(!info.isEmpty())
            info += tr("<br>");

        if(currentImage().colorCount() > 0)
            info += tr("Color Count: %1<br>").arg(currentImage().colorCount());
        else if(currentImage().depth() >= 16)
            info += tr("Color Count: True color<br>");
        info += tr("Depth: %1<br>").arg(currentImage().depth());
    //    info += tr("BitPlaneCount: %1<br>").arg(image.bitPlaneCount());//the color counts actual used, <= Depth

        int gcd = ToolKit::gcd(currentImage().width(), currentImage().height());
        QString ratioStr = (gcd == 0) ? "1:1" : QString("%1:%2")
                                        .arg(currentImage().width() / gcd)
                                        .arg(currentImage().height() / gcd);


        info += tr("Size: %1 x %2 (%3)<br>")
                .arg(currentImage().width())
                .arg(currentImage().height())
                .arg(ratioStr);
        if(fileInfo.exists() && curCache->frameCount != 1)
            info += tr("Frame Count: %1<br>").arg(curCache->frameCount);
        info += tr("Current Scale: %1%").arg(currentScale() * 100, 0, 'g', 4);
    }

    return info;
}
