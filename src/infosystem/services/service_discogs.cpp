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
#include "service_discogs.h"
#include "networkaccess.h"
#include "debug.h"
#include "constants.h"

#include <QtCore>
#include <qjson/parser.h>

/*
API
  http://www.discogs.com/help/api
  http://www.discogs.com/developers/
*/
namespace DISCOGS {
static const QString API_KEY = "91734dd989";
}

/*
********************************************************************************
*                                                                              *
*    Class ServiceDiscogs                                                      *
*                                                                              *
********************************************************************************
*/
ServiceDiscogs::ServiceDiscogs() : InfoService()
{
    Debug::debug() << Q_FUNC_INFO;

    setName("discogs");
    
     m_supportedInfoTypes << INFO::InfoAlbumSongs 
                          /*<< INFO::InfoAlbumCoverArt  */
                          << INFO::InfoArtistReleases;
}


ServiceDiscogs::~ServiceDiscogs() {}


void ServiceDiscogs::getInfo( INFO::InfoRequestData requestData )
{
    emit checkCache( requestData );
}


void ServiceDiscogs::fetchInfo( INFO::InfoRequestData requestData )
{
    //Debug::debug() << Q_FUNC_INFO;

    switch ( requestData.type )
    {
        case INFO::InfoArtistReleases      : fetch_artist_releases( requestData );break;
        case INFO::InfoAlbumSongs          : fetch_album_songs( requestData );break;
        case INFO::InfoAlbumCoverArt       : fetch_album_cover( requestData );break;
        default:
        {
            emit finished( requestData );
            return;
        }
    }  
}

void ServiceDiscogs::fetch_image_uri( INFO::InfoRequestData requestData )
{
    //Debug::debug() << Q_FUNC_INFO;
    
    INFO::InfoStringHash hash = requestData.data.value< INFO::InfoStringHash >();

    if ( !hash.contains( "#uri" ))
    {
        emit info( requestData, QVariant() );
        return;
    }
    
    
    QUrl url = QUrl( hash["#uri"] );
    
    QObject* reply = HTTP()->get( url );
    m_requests[reply] = requestData;
    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_image_received(QByteArray)));
}

void ServiceDiscogs::fetch_artist_releases( INFO::InfoRequestData requestData )
{
    //Debug::debug() << Q_FUNC_INFO;
    
    INFO::InfoStringHash hash = requestData.data.value< INFO::InfoStringHash >();

    if ( !hash.contains( "artist" ))
    {
        emit info( requestData, QVariant() );
        return;
    }
    
    QUrl url("http://api.discogs.com/database/search");
    url.addQueryItem( "api_key", DISCOGS::API_KEY );
    url.addQueryItem( "page", QString::number( 1 ) );
    url.addQueryItem( "q", hash["artist"] );
    url.addQueryItem( "type", "artist" );
    
    //Debug::debug() << "    [ServiceDiscogs] get_artist " << url;
    QObject* reply = HTTP()->get(url);
    m_requests[reply] = requestData;

    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_parse_artist_search_response(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
}


void ServiceDiscogs::slot_parse_artist_search_response(QByteArray bytes)
{
    //Debug::debug() << Q_FUNC_INFO;

    /*-------------------------------------------------*/
    /* Get id from sender reply                        */
    /* ------------------------------------------------*/
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply)) {
      return;
    }
    
    INFO::InfoRequestData request =  m_requests.take(reply);

    QJson::Parser parser;
    bool ok;
    QVariantMap reply_map = parser.parse(bytes, &ok).toMap();
    
    if (!ok || !reply_map.contains("results")) {
      return;
    }    
    
    QVariantList results = reply_map["results"].toList();

    foreach (const QVariant& result, results) {
      //Debug::debug() << Q_FUNC_INFO << result;

      QVariantMap result_map = result.toMap();
      
      if (result_map.contains("id")) {
          QUrl url( QString("http://api.discogs.com/artists/%1/releases").arg(result_map["id"].toString()) );
 
          QObject* reply = HTTP()->get( url );
          m_requests[reply] = request;          
          connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_parse_artist_release_response(QByteArray)));
          connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
          return;
      }
    }
}




void ServiceDiscogs::slot_parse_artist_release_response(QByteArray bytes)
{
    //Debug::debug() << Q_FUNC_INFO;

    /*-------------------------------------------------*/
    /* Get id from sender reply                        */
    /* ------------------------------------------------*/
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply)) {
      return;
    }
    
    INFO::InfoRequestData request =  m_requests.take(reply);
    INFO::InfoStringHash input = request.data.value< INFO::InfoStringHash >();
    /*-------------------------------------------------*/
    /* Parse response                                  */
    /* ------------------------------------------------*/
    QJson::Parser parser;
    bool ok;
    QVariantMap reply_map = parser.parse(bytes, &ok).toMap();

    if (!reply_map.contains("releases"))
      emit finished(request);
    

    QVariantList output_releases;
    
    foreach (const QVariant& release, reply_map.value("releases").toList())
    {
        QVariantMap release_map = release.toMap();
        QVariantMap output_release;

        if (release_map.contains("type"))
          if (release_map.value("type").toString() != "master")
            continue;

        output_release["artist"]  = input.value("artist"); 
        output_release["album"]   = release_map.value("title").toString();  
        output_release["year"]    = release_map.value("year").toString(); 
        
        /* WARNING remove discogs image uri download as we need authentification */
        /*output_release["uri"]     = release_map.value("thumb").toString();*/
    
        output_releases << output_release;  

        if(output_releases.size() >= 11) break;
    }
    
    QVariantMap output;    
    output["releases"] = output_releases;
    emit info(request, QVariant(output) );    
}


void ServiceDiscogs::fetch_album_songs( INFO::InfoRequestData requestData )
{
Q_UNUSED(requestData)
/* TODO */
}

void ServiceDiscogs::fetch_album_cover( INFO::InfoRequestData requestData )
{
    Debug::debug() << Q_FUNC_INFO;
  
    //Debug::debug() << "       [ServiceDiscogs] fetch_album_cover";
    INFO::InfoStringHash hash = requestData.data.value< INFO::InfoStringHash >();

    if ( hash.contains( "#uri" ) )
    {
        fetch_image_uri( requestData );
        return;
    }
    else if ( !hash.contains( "artist" ) && !hash.contains( "album" ) )
    {
        emit info( requestData, QVariant() );
        return;
    }

    QUrl url("http://api.discogs.com/database/search");
    url.addQueryItem( "api_key", DISCOGS::API_KEY );
    url.addQueryItem( "page", QString::number( 1 ) );
    url.addQueryItem( "artist", hash["artist"] );
    url.addQueryItem( "release_title", hash["album"].toLower() );
    url.addQueryItem( "type", "release" );
 
    
    Debug::debug() << Q_FUNC_INFO << url;

    QObject* reply = HTTP()->get(url);
    m_requests[reply] = requestData;

    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_parse_album_search_response(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
}


void ServiceDiscogs::slot_parse_album_search_response(QByteArray bytes)
{
    Debug::debug() << Q_FUNC_INFO;

    /*-------------------------------------------------*/
    /* Get id from sender reply                        */
    /* ------------------------------------------------*/
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply)) {
      Debug::debug() << Q_FUNC_INFO << " no reply found";
      return;
    }
    
    INFO::InfoRequestData request =  m_requests.take(reply);
    
    
    QJson::Parser parser;
    bool ok;
    QVariantMap reply_map = parser.parse(bytes, &ok).toMap();
    
    if (!ok || !reply_map.contains("results")) {
      return;
    }    
    
    QVariantList results = reply_map["results"].toList();
    //Debug::debug() << Q_FUNC_INFO << " results";

    foreach (const QVariant& result, results) 
    {
      QVariantMap result_map = result.toMap();
      if (result_map.contains("thumb"))
      {
          /*  Hack from Clementine 
              In order to use less round-trips, we cheat here.  Instead of
              following the "resource_url", and then scan all images in the
              resource, we go directly to the largest primary image by
              constructing the primary image's url from the thmub's url */

          QObject* reply = HTTP()->get(QUrl(result_map["thumb"].toString().replace("R-90-", "R-")));
          m_requests[reply] = request;
    
          connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_image_received(QByteArray)));
          connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
          return;
      }
    }
    
    Debug::debug() << Q_FUNC_INFO << " no result found";
    emit info(request, QVariant());
}


void ServiceDiscogs::slot_image_received(QByteArray bytes)
{
    Debug::debug() << Q_FUNC_INFO;
    /*-------------------------------------------------*/
    /* Get id from sender reply                        */
    /* ------------------------------------------------*/
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))   return;
    
    INFO::InfoRequestData request =  m_requests.take(reply);

    emit info(request, QVariant(bytes) );
}


void ServiceDiscogs::slot_request_error()
{
    //Debug::debug() << Q_FUNC_INFO;

    /* get sender reply */
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))  {
      return;
    }

    INFO::InfoRequestData request = m_requests.take(reply);

    /* send process finished */
    emit info(request, QVariant());
}
