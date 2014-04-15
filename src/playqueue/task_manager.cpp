/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2014 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
*                                                                                       *
*  This program is free software; you can redistribute it and/or modify it under        *
*  the terms of the GNU General Public License as published by the Free Software        *
*  Foundation; either version 2 of the License, or (at your option) any later           *
*  version.                                                                             *
*                                                                                       *
*  This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
*  PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
*                                                                                       *
*  You should have received a copy of the GNU General Public License along with         *
*  this program.  If not, see <http://www.gnu.org/licenses/>.                           *
*****************************************************************************************/

#include "task_manager.h"

#include "playqueue_model.h"
#include "playlistpopulator.h"
#include "playlistwriter.h"
#include "playlistdbwriter.h"

//#include "widgets/statuswidget.h" TODO

#include "debug.h"

/*
********************************************************************************
*                                                                              *
*    Class TaskManager                                                         *
*                                                                              *
********************************************************************************
*/
TaskManager::TaskManager(PlayqueueModel* model) : QObject()
{
    m_model       = model;
    
    m_threadPool  = new QThreadPool(this);

    // Qrunnable
    m_populator   = new PlaylistPopulator();
    m_populator->setModel(m_model);
    m_populator->setAutoDelete(false);

    m_writer      = new PlaylistWriter();
    m_writer->setModel(m_model);
    m_writer->setAutoDelete(false);

    m_db_writer   = new PlaylistDbWriter();
    m_db_writer->setModel(m_model);
    m_db_writer->setAutoDelete(false);

    // connection
    QObject::connect(m_populator,SIGNAL(playlistPopulated()),this,SIGNAL(playlistPopulated()));
    QObject::connect(m_populator,SIGNAL(async_load(QString,int)),this,SLOT(slot_load_async(QString,int)));

    QObject::connect(m_writer,SIGNAL(playlistSaved()),this,SIGNAL(playlistSaved()));
    QObject::connect(m_db_writer,SIGNAL(playlistSaved()),this,SIGNAL(playlistSaved()));
}

TaskManager::~TaskManager()
{
    Debug::debug() << "TaskManager -> wait to finish ...";
    m_threadPool->waitForDone();
    delete m_populator;
    delete m_writer;
    delete m_db_writer;
    delete m_threadPool;
}


/*******************************************************************************
  slot_load_async
*******************************************************************************/
void TaskManager::slot_load_async(QString url,int row)
{
    AsynchronousLoadTask* loader = new AsynchronousLoadTask(url, row);
    loader->setModel(m_model);
    loader->start_asynchronous_download();
}


/*******************************************************************************
 PlayQueue Population
*******************************************************************************/
void TaskManager::playlistAddFiles(const QStringList &files)
{
    if(m_populator->isRunning()) return;
    m_populator->addFiles(files);
    m_threadPool->start(m_populator, 1);
}

void TaskManager::playlistAddFile(const QString &file)
{
    if(m_populator->isRunning()) return;
    m_populator->addFile(file);
    m_threadPool->start (m_populator, 1);
}

void TaskManager::playlistAddUrls(QList<QUrl> listUrl, int playlist_row)
{
    if(m_populator->isRunning()) return;
    m_populator->addUrls(listUrl, playlist_row);
    m_threadPool->start (m_populator, 1);
}

void TaskManager::playlistAddMediaItems(QList<MEDIA::TrackPtr> list, int playlist_row)
{
    if(m_populator->isRunning()) return;
    m_populator->addMediaItems(list, playlist_row);
    m_threadPool->start (m_populator, 1);
}


/*******************************************************************************
 Playqueue to playlist Writer
*******************************************************************************/
void TaskManager::playlistSaveToFile(const QString &filename)
{
    if(m_writer->isRunning()) return;
    m_writer->saveToFile(filename);
    m_threadPool->start (m_writer);
}

void TaskManager::playlistSaveToDb(const QString &name, int bd_id)
{
    if(m_db_writer->isRunning()) return;
    m_db_writer->saveToDatabase(name, bd_id);
    m_threadPool->start (m_db_writer);
}


