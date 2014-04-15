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

#include "playlistpopulator.h"
#include "playqueue_model.h"

#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/playlist_parser.h"
#include "networkaccess.h"

#include "settings.h"
#include "utilities.h"
#include "debug.h"

//Qt
#include <QDataStream>
#include <QFile>
#include <QByteArray>
#include <QDirIterator>
#include <QMutexLocker>
#include <QFileInfo>


/*
********************************************************************************
*                                                                              *
*    Class PlaylistPopulator                                                   *
*                                                                              *
********************************************************************************
*/
PlaylistPopulator::PlaylistPopulator()
{
    setObjectName("PlaylistPopulator");

    m_files.clear();
    m_tracks.clear();
    m_isRunning      = false;
    m_playlist_row   = -1;
}

/*******************************************************************************
  PlaylistPopulator::run
*******************************************************************************/
void PlaylistPopulator::run()
{
  Debug::debug() << " --- PlaylistPopulator--> Start "  << QTime::currentTime().second() << ":" << QTime::currentTime().msec();
  //QTime startTime = QTime::currentTime();

  m_isRunning = true;
    
    
  while (!m_files.isEmpty() || !m_tracks.isEmpty())
  {
      //Debug::debug() << "PlaylistPopulator process loop";

      /*--------------------------------------------------*/
      /* cas des dossiers                                 */
      /* -------------------------------------------------*/
      if (m_files.size() > 0) {
        if (QFileInfo(m_files.first()).isDir()) {
          const QString dirName = m_files.takeFirst();
          const QStringList dirFilter  = QStringList() << "*.mp3" << "*.ogg" << "*.flac" << "*.wav" << "*.m4a" << "*.aac";
          QDirIterator dirIterator(dirName, dirFilter ,QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

          QStringList readfiles;
          while(dirIterator.hasNext()) {
            readfiles << dirIterator.next();
          }

          readfiles.append(m_files);
          m_files = readfiles;
        }
      }

      /*--------------------------------------------------*/
      /* cas des playlists remote                         */
      /* -------------------------------------------------*/
      if (m_files.size() > 0) {
        if( !MEDIA::isLocal(m_files.first()) &&
            (MEDIA::isPlaylistFile(m_files.first()) ||
             MEDIA::isShoutCastUrl(m_files.first()) ||
             MEDIA::isTuneInUrl(m_files.first())) )
        {
            //! remote playlist --> download, make it local
            //Debug::debug() << "PlaylistPopulator startDownload" << m_files.first();
            emit async_load(m_files.takeFirst(), m_playlist_row);
        }
      }

      /*--------------------------------------------------*/
      /* cas des playlists locale                         */
      /* -------------------------------------------------*/
      if (m_files.size() > 0) {
      //! local playlist
        if( MEDIA::isLocal(m_files.first()) && MEDIA::isPlaylistFile(m_files.first())) {
              //! local playlist
              //Debug::debug() << "#1 PlaylistPopulator PlaylistFromFile local " << m_files.first();
              QList<MEDIA::TrackPtr> list = MEDIA::PlaylistFromFile(m_files.takeFirst());

              foreach (MEDIA::TrackPtr track, list) 
              {
                if(track->type() == TYPE_TRACK) {
                    QString url = track->url;
                    track.reset();
                    track = MEDIA::FromDataBase(url);
                    if(!track)
                      track = MEDIA::FromLocalFile(url);

                    m_model->request_insert_track(track, m_playlist_row);

                    track.reset();
                }
                else {
                    m_model->request_insert_track(track, m_playlist_row);
                    track.reset();
                }
             } // foreach MediaItem
          }
      }

      /*--------------------------------------------------*/
      /* cas des fichiers et urls                         */
      /* -------------------------------------------------*/
      if (m_files.size() > 0) {
          if(  MEDIA::isLocal(m_files.first()) &&
               MEDIA::isAudioFile(m_files.first()) &&
              !MEDIA::isPlaylistFile(m_files.first()))
          {
              QString fileName = QFileInfo(m_files.takeFirst()).canonicalFilePath();

              MEDIA::TrackPtr track = MEDIA::FromDataBase(fileName);
              if(!track)
                track = MEDIA::FromLocalFile(fileName);

              m_model->request_insert_track(track, m_playlist_row);
              track.reset();
          }
          else if(!MEDIA::isLocal(m_files.first())) {
              //! remote file
              Debug::debug() << "remote uRL" << m_files.first();
              QString url = m_files.takeFirst();
              MEDIA::TrackPtr stream = MEDIA::TrackPtr(new MEDIA::Track());
              stream->setType(TYPE_STREAM);
              stream->id          = -1;
              stream->url         = url;
              stream->name        = url;
              stream->title       = QString();
              stream->artist      = QString();
              stream->album       = QString();
              stream->categorie   = QString();
              stream->isFavorite  = false;
              stream->isPlaying   = false;
              stream->isBroken    = false;
              stream->isPlayed    = false;
              stream->isStopAfter = false;

              m_model->request_insert_track(stream, m_playlist_row);

              stream.reset();
          }
          else {
              //!WARNING on doit sortir les éléments non traité (si boucle while)
              Debug::warning() << "PlaylistPopulator --> unsupported media !" << m_files.takeFirst();
          }
      }

      /*--------------------------------------------------*/
      /* cas des mediaitems                               */
      /* -------------------------------------------------*/
      if(m_tracks.size() > 0) {
        //Debug::debug() << "PlaylistPopulator m_tracks.size() :" << m_tracks.size();

        MEDIA::TrackPtr track = m_tracks.takeFirst();
        if( MEDIA::isPlaylistFile(track->url) || 
            MEDIA::isShoutCastUrl(track->url) ||
            MEDIA::isTuneInUrl(track->url)  )
        {
            m_files.append( track->url );
        }
        else 
        {
            m_model->request_insert_track(track, m_playlist_row);
        }
      }

  } //! END !m_files.isEmpty() && !m_tracks.isEmpty()

  if(SETTINGS()->_playqueueDuplicate == false)
    m_model->removeDuplicate();

  m_isRunning = false;
  emit playlistPopulated();

  m_model->signalUpdate();

  //Debug::debug() << " --- PlaylistPopulator--> Start "  << startTime.second() << ":" << startTime.msec();
  Debug::debug() << " --- PlaylistPopulator--> End "  << QTime::currentTime().second() << ":" << QTime::currentTime().msec();
}

/*******************************************************************************
  User methode
*******************************************************************************/
/*
  addFile  : add one file to proceed
  addFiles : add files
  addUrls  : add Urls
*/

void PlaylistPopulator::addFile(const QString &file)
{
    //Debug::debug() << "PlaylistPopulator append file :" << file;
    m_playlist_row = -1;
    m_files.append(file);
}

void PlaylistPopulator::addFiles(const QStringList &files)
{
    //Debug::debug() << "PlaylistPopulator append files :" << files;
    m_playlist_row = -1;
    m_files.append(files);
}

void PlaylistPopulator::addUrls(QList<QUrl> listUrl, int playlist_row)
{
    m_playlist_row = playlist_row;

    foreach (const QUrl &url, listUrl) {
      //Debug::debug() << "PlaylistPopulator append url :" << url.toString();
      if(MEDIA::isLocal(url.toString()))
        m_files.append(url.toLocalFile());
      else
        m_files.append(url.toString());
    }
}

void PlaylistPopulator::addMediaItems(QList<MEDIA::TrackPtr> list, int playlist_row)
{
    //Debug::debug() << "PlaylistPopulator addMediaItems " << list;
    m_playlist_row = playlist_row;
    m_tracks.append(list);
}

/*
********************************************************************************
*                                                                              *
*    Class AsynchronousLoadTask                                                *
*                                                                              *
********************************************************************************
*/
AsynchronousLoadTask::AsynchronousLoadTask(const QString& url, int row)
{
    m_url  = url;
    m_row  = row;
}

void AsynchronousLoadTask::start_asynchronous_download()
{
    Debug::debug() << "AsynchronousLoadTask start_asyncronous_download";

    QObject *reply = HTTP()->get(QUrl(m_url));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_download_error()));
    connect(reply, SIGNAL(data(QByteArray)), SLOT(slot_download_done(QByteArray)));
}


void AsynchronousLoadTask::slot_download_done(QByteArray bytes)
{
    Debug::debug() << "AsynchronousLoadTask slot_download_done";

    if(bytes.isEmpty())
    {
      Debug::warning() << "AsynchronousLoadTask received empty bytes";
    }
    else
    {
      QList<MEDIA::TrackPtr> list = MEDIA::PlaylistFromBytes(bytes);

      foreach (MEDIA::TrackPtr track, list)
      {
        /*-------------------------------*/
        /* TYPE TRACK                    */
        /* ------------------------------*/
        if(track->type() == TYPE_TRACK) 
        {
            MEDIA::TrackPtr track = MEDIA::FromDataBase(track->url);
            if(!track)
             track = MEDIA::FromLocalFile(track->url);

            m_model->request_insert_track(track, m_row);

            track.reset();
        }
        /*-------------------------------*/
        /* TYPE STREAM                   */
        /* ------------------------------*/
        else 
        {
          //Debug::debug() << "AsynchronousLoadTask Remote track found :" << track->url;
          // WARNING : can be recursive 
          if( MEDIA::isPlaylistFile(track->url) || 
              MEDIA::isShoutCastUrl(track->url) ||
              MEDIA::isTuneInUrl(track->url)  )
           {
             Debug::warning() << "AsynchronousLoadTask remote playlist found :" << track->url;
             AsynchronousLoadTask* loader = new AsynchronousLoadTask(track->url, m_row);
             loader->setModel(m_model);
             loader->start_asynchronous_download();
           }
           else 
           {
             Debug::debug() << "AsynchronousLoadTask track found :" << track->url;
             m_model->request_insert_track(track, m_row);
           }
        }
        track.reset();
      } // foreach track
    }

    delete this;
}

void AsynchronousLoadTask::slot_download_error()
{
    Debug::debug() << "AsynchronousLoadTask slot_download_error";
    MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());
    media->setType(TYPE_STREAM);
    media->id           = -1;
    media->url          = m_url;
    media->name         = m_url;
    media->isFavorite   = false;
    media->isPlaying    = false;
    media->isBroken     = true;
    media->isPlayed     = false;
    media->isStopAfter  = false;

    m_model->request_insert_track(media, m_row);

    delete this;
}

