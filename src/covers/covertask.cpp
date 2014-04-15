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
#include "covertask.h"

#include "info_system.h"

#include "core/database/database.h"
#include "core/mediaitem/mediaitem.h"
#include "utilities.h"
#include "debug.h"

#include <QSqlQuery>
#include <QVariant>

#include <QFile>
#include <QImage>


/*
********************************************************************************
*                                                                              *
*    Class CoverTask                                                           *
*                                                                              *
********************************************************************************
*/

CoverTask::CoverTask(QObject *parent) : QObject(parent)
{
    setObjectName("covertask");

    //! init
    m_isFullDbSearch      = true;
    m_exit                = false;
    m_isRunning           = false;
    
    m_timeout.setInterval( 10000 );
    m_timeout.setSingleShot( false );
    connect( &m_timeout, SIGNAL( timeout() ), SLOT( slot_request_timeout() ) );    
}

CoverTask::~CoverTask()
{
}


//! -- CoverTask::setRequest -------------------------------------------------
void CoverTask::setRequest(const QString& artist, const QString& album)
{
    m_requests.clear();
    
    INFO::InfoRequestData request;

    request.requestId = INFO::infosystemRequestId();
    request.type = INFO::InfoAlbumCoverArt;
  
    INFO::InfoStringHash hash;
    hash["artist"] = artist;
    hash["album"]  = album;
    hash["covername"]  = MEDIA::coverName(artist, album);

    request.data = QVariant::fromValue< INFO::InfoStringHash >( hash );
    
    const QString path = UTIL::CONFIGDIR + "/albums/" + hash["covername"];

    if(!QFile::exists(path))
      m_requests.insert(request.requestId, request);

    m_isFullDbSearch = false;
}


//! -- CoverTask::run --------------------------------------------------------
void CoverTask::start()
{
    Debug::debug() << Q_FUNC_INFO;
    if(m_isFullDbSearch)
      slot_check_database();
    /*else
      -> take request as parameter  */

    m_isRunning = true;
    m_max = m_requests.size();

    slot_process_cover_search();

    //! Start thread event loop --> makes signals and slots work
    QEventLoop eventLoop;
    eventLoop.exec();
    
    m_isFullDbSearch      = true;
    m_requests.clear();
}

//! -- slot_finish_cover_search ------------------------------------------------
void CoverTask::slot_finish_cover_search()
{
    Debug::debug() << Q_FUNC_INFO;
    m_timeout.stop();
    m_isRunning       = false;
    m_isFullDbSearch  = true;
    emit finished();
}


void CoverTask::slot_request_timeout()
{
    if(!m_requests.isEmpty())
      m_requests.take(m_requests.keys().first());
    
    slot_process_cover_search();
}


//! -- slot_process_cover_search -----------------------------------------------
void CoverTask::slot_process_cover_search()
{
    Debug::debug() << Q_FUNC_INFO << " request size:" << m_requests.size();

    if(!m_requests.isEmpty() && !m_exit) 
    {
      emit progress ( ((m_max - m_requests.values().size())*100)/m_max );
      
      INFO::InfoRequestData request = m_requests.value(m_requests.keys().first());
    
      connect( InfoSystem::instance(),
             SIGNAL( info( INFO::InfoRequestData, QVariant ) ),
             SLOT( slot_system_info( INFO::InfoRequestData, QVariant ) ),  Qt::UniqueConnection );

      InfoSystem::instance()->getInfo( request );
      
      /* timer is needed as we dont have info signal from InfoSystem each time */
      m_timeout.start();
    }
    else 
    {
       m_timeout.stop();
       slot_finish_cover_search();
    }
}

void CoverTask::slot_system_info( INFO::InfoRequestData request, QVariant output)
{
    m_timeout.stop();
    Debug::debug() << Q_FUNC_INFO << "get info for request.id " << request.requestId;
 
    if(!m_requests.contains(request.requestId)) {
      slot_process_cover_search();
      return;
    }

    m_requests.take(request.requestId);

    if(output.isNull()) {
      slot_process_cover_search();
      return;
    }
    
    /* get request info */
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();
   
    /* filepath for cover file */
    const QString filePath = UTIL::CONFIGDIR + "/albums/" + hash["covername"];

    QFile file(filePath);
    if(file.exists())
    {
      Debug::debug() << "       [CoverTask] file exists, start next cover search";
    }
    else if (!file.open(QIODevice::WriteOnly))
    {
      Debug::debug() << "       [CoverTask] error writing album image";
    }
    else
    {
      const QByteArray bytes = output.toByteArray();

      QImage image = QImage::fromData(bytes);
      image = image.scaled(QSize(110, 110), Qt::KeepAspectRatio, Qt::SmoothTransformation);
      image.save(filePath, "png", -1);
    }
    
    slot_process_cover_search();
}


//! -- slot_check_database -----------------------------------------------------
void CoverTask::slot_check_database()
{
    Debug::debug() << Q_FUNC_INFO;

    Database db;
    if (!db.connect())  return;

    m_requests.clear();
    
    //! albums database loop
    QSqlQuery query("SELECT name,cover,artist_name FROM view_albums",*db.sqlDb());
    
    while (query.next())
    {
        
        const QString path = UTIL::CONFIGDIR + "/albums/" + query.value(1).toString();

        if(!QFile::exists(path)) 
        {
          INFO::InfoRequestData request;
          request.requestId = INFO::infosystemRequestId();
          request.type = INFO::InfoAlbumCoverArt;
  
          INFO::InfoStringHash hash;
          hash["artist"] = query.value(2).toString();
          hash["album"]  = query.value(0).toString();
          hash["covername"]  = query.value(1).toString();

          request.data = QVariant::fromValue< INFO::InfoStringHash >( hash );

          m_requests.insert(request.requestId, request);
        }
    } // fin while album query
}


