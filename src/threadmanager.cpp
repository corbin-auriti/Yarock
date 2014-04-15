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

#include "threadmanager.h"
#include "models/local/local_track_populator.h"
#include "models/local/local_playlist_populator.h"
#include "core/database/databasebuilder.h"
#include "covers/covertask.h"

#include "widgets/statuswidget.h"

#include "debug.h"

ThreadManager* ThreadManager::INSTANCE = 0;
/*
********************************************************************************
*                                                                              *
*    Class ThreadManager                                                       *
*                                                                              *
********************************************************************************
*/
ThreadManager::ThreadManager()
{
    INSTANCE = this;

    // QThread
    m_databaseBuilder         = new DataBaseBuilder();
    m_localTrackPopulator     = new LocalTrackPopulator();
    m_localPlaylistPopulator  = new LocalPlaylistPopulator();
    m_coverTask               = 0;

    // connection
    QObject::connect(m_databaseBuilder,SIGNAL(buildingFinished()),this,SLOT(dbBuildFinish()));
    QObject::connect(m_databaseBuilder,SIGNAL(buildingProgress(int)),this,SLOT(dbBuildProgressChanged(int)));

    QObject::connect(m_localTrackPopulator,SIGNAL(populatingFinished()),this,SLOT(slot_on_localtrackmodel_populated()));
    QObject::connect(m_localTrackPopulator,SIGNAL(populatingProgress(int)),this,SLOT(slot_on_localtrackmodel_populating_changed(int)));

    QObject::connect(m_localPlaylistPopulator,SIGNAL(populatingFinished()),this,SLOT(slot_on_localplaylistmodel_populated()));
    QObject::connect(m_localPlaylistPopulator,SIGNAL(populatingProgress(int)),this,SLOT(slot_on_localplaylistmodel_populating_changed(int)));
}


ThreadManager::~ThreadManager()
{
    Debug::debug() << "ThreadManager -> wait to finish ...";
    delete m_databaseBuilder;
    delete m_localTrackPopulator;
    delete m_localPlaylistPopulator;
}


/*******************************************************************************
    ThreadManager::stopThread
*******************************************************************************/
void ThreadManager::stopThread()
{
    Debug::debug() << "ThreadManager -> stop running thread";
    if(m_databaseBuilder->isRunning())
      cancelThread(DB_THREAD);

    if(m_localTrackPopulator->isRunning())
      cancelThread(POPULATOR_C_THREAD);

    if(m_localPlaylistPopulator->isRunning())
      cancelThread(POPULATOR_P_THREAD);

    if(m_coverTask != 0)
      if(m_coverTask->isRunning())
        cancelThread(COVER_SEARCH_THREAD);
}


/*******************************************************************************
    Database Scanner Thread
*******************************************************************************/
void ThreadManager::databaseBuild(QStringList listDir)
{
    if(m_databaseBuilder->isRunning())
      cancelThread(DB_THREAD);

    Debug::debug() << " ThreadManager start a database builder thread";
    m_databaseBuilder->rebuildFolder(listDir);
    m_databaseBuilder->start();

    uint i = StatusWidget::instance()->startProgressMessage(tr("Updating music database") + " (0%)");
    messageIds.insert("DbUpdate", i);
    emit dbBuildStart();    
}

void ThreadManager::dbBuildProgressChanged(int progress)
{
    QString message = QString(tr("Updating music database") + " (%1%)").arg(QString::number(progress));
    StatusWidget::instance()->updateProgressMessage( messageIds.value("DbUpdate"), message );
}

void ThreadManager::dbBuildFinish()
{
    if (messageIds.contains("DbUpdate"))
      StatusWidget::instance()->stopProgressMessage( messageIds.take("DbUpdate") );

    emit dbBuildFinished();
    
    /* after db is built => repopulate local track model */
    this->populateLocalTrackModel();
}

bool ThreadManager::isDbRunning()
{
    return m_databaseBuilder->isRunning();
}


/*******************************************************************************
    Local track Model Population thread
*******************************************************************************/
void ThreadManager::populateLocalTrackModel()
{
   Debug::debug() << "ThreadManager -> start a new collection populator";
   if(m_localTrackPopulator->isRunning()) {
     cancelThread(POPULATOR_C_THREAD);
   }

   m_localTrackPopulator->start();

   uint i = StatusWidget::instance()->startProgressMessage(tr("Loading music collection"));
   messageIds.insert("LoadMusic", i);
}

void ThreadManager::slot_on_localtrackmodel_populating_changed(int progress)
{
    QString message = QString(tr("Loading Music Collection") + " (%1%)").arg(QString::number(progress));
    StatusWidget::instance()->updateProgressMessage( messageIds.value("LoadMusic"), message );
}

void ThreadManager::slot_on_localtrackmodel_populated()
{
    if (messageIds.contains("LoadMusic"))
      StatusWidget::instance()->stopProgressMessage( messageIds.take("LoadMusic") );

    emit modelPopulationFinished(MODEL_COLLECTION);

    // for each collection update do LocalPlaylistModel update
    this->populateLocalPlaylistModel();
}

/*******************************************************************************
    Playlist Model Population thread
*******************************************************************************/
void ThreadManager::populateLocalPlaylistModel()
{
   Debug::debug() << "ThreadManager -> start a new playlist populator";
   if(m_localPlaylistPopulator->isRunning())
     cancelThread(POPULATOR_P_THREAD);

   m_localPlaylistPopulator->start();

   uint i = StatusWidget::instance()->startProgressMessage(tr("Loading playlist files"));
   messageIds.insert("LoadPlaylist", i);
}

void ThreadManager::slot_on_localplaylistmodel_populating_changed(int progress)
{
    QString message = QString(tr("Loading Playlist File") + " (%1%)").arg(QString::number(progress));
    StatusWidget::instance()->updateProgressMessage( messageIds.value("LoadPlaylist"), message );
}

void ThreadManager::slot_on_localplaylistmodel_populated()
{
    Debug::debug() << "ThreadManager -> slot_on_localplaylistmodel_populated";

    if (messageIds.contains("LoadPlaylist"))
      StatusWidget::instance()->stopProgressMessage( messageIds.take("LoadPlaylist") );

    emit modelPopulationFinished(MODEL_PLAYLIST);
}

/*******************************************************************************
  Cover Search Thread
*******************************************************************************/
void ThreadManager::startCoverSearch()
{
    Debug::debug() << "ThreadManager -> startCoverSearch";
  
    if(!m_coverTask) {
      m_coverTask = new CoverTask();

      connect(m_coverTask,SIGNAL(finished()),this,SLOT(coverSearchFinished()));
      connect(m_coverTask,SIGNAL(progress(int)),this,SLOT(coverSearchProgress(int)));
    }

    if(m_coverTask->isRunning())
      cancelThread(COVER_SEARCH_THREAD);

    uint i = StatusWidget::instance()->startProgressMessage(tr("Fetching album cover"));
    messageIds.insert("CoverUpdate", i);

    m_coverTask->start();
}

void ThreadManager::startCoverSearch(const QString& artist, const QString& album)
{
    Debug::debug() << "ThreadManager -> startCoverSearch";

    if(!m_coverTask) {
      m_coverTask = new CoverTask();

      connect(m_coverTask,SIGNAL(finished()),this,SLOT(coverSearchFinished()));
      connect(m_coverTask,SIGNAL(progress(int)),this,SLOT(coverSearchProgress(int)));
    }

    if(m_coverTask->isRunning())
      cancelThread(COVER_SEARCH_THREAD);

    m_coverTask->setRequest(artist, album);
    m_coverTask->start();
}


void ThreadManager::coverSearchFinished()
{
    Debug::debug() << "ThreadManager -> coverSearchFinished";

    if (messageIds.contains("CoverUpdate"))
      StatusWidget::instance()->stopProgressMessage( messageIds.take("CoverUpdate") );
}

void ThreadManager::coverSearchProgress(int progress)
{
    QString message = QString(tr("Fetching Album Cover") + " (%1%)").arg(QString::number(progress));
    StatusWidget::instance()->updateProgressMessage( messageIds.value("CoverUpdate"), message );
}

/*******************************************************************************
  Cancel Thread
*******************************************************************************/
void ThreadManager::cancelThread(E_THREAD thread)
{
    switch(thread) {
      case DB_THREAD:
         Debug::warning() << "ThreadManager -> cancel database builder thread !!";
        m_databaseBuilder->setExit(true);
        m_databaseBuilder->wait();      // wait end of thread should be fast
        m_databaseBuilder->setExit(false);
        if (messageIds.contains("DbUpdate"))
          StatusWidget::instance()->stopProgressMessage( messageIds.take("DbUpdate") );

       break;

      case POPULATOR_C_THREAD:
         Debug::warning() << "ThreadManager -> cancel collection populator thread !!";
        m_localTrackPopulator->setExit(true);
        m_localTrackPopulator->wait();      // wait end of thread should be fast
        m_localTrackPopulator->setExit(false);
        if (messageIds.contains("LoadMusic"))
          StatusWidget::instance()->stopProgressMessage( messageIds.take("LoadMusic") );

       break;

      case  POPULATOR_P_THREAD:
         Debug::warning() << "ThreadManager -> cancel playlist populator thread !!";
        m_localPlaylistPopulator->setExit(true);
        m_localPlaylistPopulator->wait();
        m_localPlaylistPopulator->setExit(false);

        if (messageIds.contains("LoadPlaylist"))
          StatusWidget::instance()->stopProgressMessage( messageIds.take("LoadPlaylist") );

      break;

      case COVER_SEARCH_THREAD:
         Debug::warning() << "ThreadManager -> cancel cover search thread !!";
        m_coverTask->setExit(true);

        if (messageIds.contains("CoverUpdate"))
          StatusWidget::instance()->stopProgressMessage( messageIds.take("CoverUpdate") );

      break;

      default:break;
    }
}

