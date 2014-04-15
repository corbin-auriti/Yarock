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
#include "service_musicbrainz.h"
#include "networkaccess.h"
#include "debug.h"
#include "constants.h"

#include <QtCore>
#include <QDomDocument>

/*
********************************************************************************
*                                                                              *
*    Class ServiceMusicBrainz                                                  *
*                                                                              *
********************************************************************************
*/
ServiceMusicBrainz::ServiceMusicBrainz() : InfoService()
{
    Debug::debug() << Q_FUNC_INFO;

    setName("musicbrainz");
    
    m_supportedInfoTypes << INFO::InfoArtistReleases
                         << INFO::InfoAlbumCoverArt;

}


ServiceMusicBrainz::~ServiceMusicBrainz() {}


void ServiceMusicBrainz::getInfo( INFO::InfoRequestData requestData )
{
    emit checkCache( requestData );
}


void ServiceMusicBrainz::fetchInfo( INFO::InfoRequestData requestData )
{
    //Debug::debug() << Q_FUNC_INFO;

    switch ( requestData.type )
    {
        case INFO::InfoArtistReleases : fetch_artist_releases( requestData );break;
        case INFO::InfoAlbumCoverArt  : fetch_album_cover( requestData );break;
        default:
        {
            emit finished( requestData );
            return;
        }
    }  
}

void ServiceMusicBrainz::fetch_artist_releases( INFO::InfoRequestData requestData )
{
    //Debug::debug() << Q_FUNC_INFO;
    
    INFO::InfoStringHash hash = requestData.data.value< INFO::InfoStringHash >();

    if ( !hash.contains( "artist" ))
    {
        emit info( requestData, QVariant() );
        return;
    }
    
    QString query;
    
    query.append( QString( "artist:\"%1\"" ).arg(hash["artist"]) );
    query.append( " AND (type:album OR type:ep)" );
    query.append( " AND status:official" );
    query.append( " AND NOT secondarytype:live" );
    query.append( " AND NOT secondarytype:compilation" );
    
    //QUrl url("http://musicbrainz.org/ws/2/release-group");
    QUrl url("http://musicbrainz.org/ws/2/release");

    url.addQueryItem( "query", query );
    url.addQueryItem( "limit", "11" );

    QObject* reply = HTTP()->get(url);
    m_requests[reply] = requestData;

    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_parse_artist_release_response(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
}


void ServiceMusicBrainz::slot_parse_artist_release_response(QByteArray bytes)
{
    Debug::debug() << Q_FUNC_INFO;

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
    QDomDocument doc;
    doc.setContent( bytes );
    
    QDomNodeList releaseGroupsNL = doc.elementsByTagName( "release" );
    
    if ( releaseGroupsNL.isEmpty() )
    {
        Debug::warning() << Q_FUNC_INFO << " releaseGroupsNL EMPTY";
      
        emit info( request, QVariant() );
        return;
    }
    

    if( request.type == INFO::InfoArtistReleases )
    {
        QVariantList output_releases;
        QVariantMap output_release;

        QString popularId = releaseGroupsNL.at(0).firstChildElement( "artist-credit" ).firstChildElement( "name-credit" ).firstChildElement( "artist" ).attribute( "id" );
        QStringList albums;
        for ( int i = 0; i < releaseGroupsNL.count(); i++ )
        {
            QString groupTitle = releaseGroupsNL.at(i).firstChildElement("title").text();
            QString a = releaseGroupsNL.at(i).firstChildElement( "artist-credit" ).firstChildElement( "name-credit" ).firstChildElement( "artist" ).firstChildElement( "name" ).text();
            QString id = releaseGroupsNL.at(i).firstChildElement( "artist-credit" ).firstChildElement( "name-credit" ).firstChildElement( "artist" ).attribute( "id" );
            

            if ( !albums.contains( groupTitle ) && id == popularId && a.normalized( QString::NormalizationForm_KC ) == input["artist"].normalized( QString::NormalizationForm_KC ) )
            {
                output_release["artist"]  = input.value("artist");
                output_release["album"]   = groupTitle;
                output_release["mbid"]    = releaseGroupsNL.at(i).toElement().attribute("id");

                albums << groupTitle;
                output_releases << output_release;  
            }
        }
     
        if(!output_releases.isEmpty()) {
          //Debug::debug() << Q_FUNC_INFO << output_releases;
          QVariantMap output;    
          output["releases"] = output_releases;
          emit info(request, QVariant(output) );       
        }
    }
}

void ServiceMusicBrainz::fetch_image_from_mbid( INFO::InfoRequestData requestData )
{
    //Debug::debug() << Q_FUNC_INFO;
    
    INFO::InfoStringHash hash = requestData.data.value< INFO::InfoStringHash >();

    if ( !hash.contains( "#mbid" ))
    {
        emit info( requestData, QVariant() );
        return;
    }
    
    QUrl url = QUrl( QString("http://coverartarchive.org/release/%1/front").arg(hash["#mbid"]) );
    
    QObject* reply = HTTP()->get( url );
    m_requests[reply] = requestData;
    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_image_received(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
}


void ServiceMusicBrainz::fetch_album_cover( INFO::InfoRequestData requestData )
{
    //Debug::debug() << Q_FUNC_INFO;
  
    INFO::InfoStringHash hash = requestData.data.value< INFO::InfoStringHash >();

    if ( hash.contains( "#mbid" ) )
    {
        fetch_image_from_mbid( requestData );
        return;
    }
    else if ( !hash.contains( "artist" ) && !hash.contains( "album" ) )
    {
        emit info( requestData, QVariant() );
        return;
    }

    QString query = QString("release:\"%1\" AND artist:\"%2\"")
      .arg(hash["album"].trimmed().replace('"', "\\\""))
      .arg(hash["artist"].trimmed().replace('"', "\\\""));
      
    QUrl url("http://musicbrainz.org/ws/2/release");
    url.addQueryItem( "query", query );
    url.addQueryItem( "limit", "5" );
    
    QObject* reply = HTTP()->get(url);
    m_requests[reply] = requestData;

    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_parse_album_response(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
}


void ServiceMusicBrainz::slot_parse_album_response(QByteArray bytes)
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
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();
    
    /*-------------------------------------------------*/
    /* Parse response                                  */
    /* ------------------------------------------------*/    
    QDomDocument doc;
    doc.setContent( bytes );
    
    QDomNodeList releaseGroupsNL = doc.elementsByTagName( "release" );
    
    if( request.type == INFO::InfoAlbumCoverArt )
    {
        if(releaseGroupsNL.count() >= 1) {
            hash["#mbid"] = releaseGroupsNL.at(0).toElement().attribute("id");

            fetch_image_from_mbid( INFO::InfoRequestData(INFO::InfoAlbumCoverArt, hash) );
            return;
        }
    }
    
    /* no result */
    emit info( request, QVariant() );
}


void ServiceMusicBrainz::slot_image_received(QByteArray bytes)
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


void ServiceMusicBrainz::slot_request_error()
{
    Debug::debug() << Q_FUNC_INFO;

    /* get sender reply */
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))  {
      return;
    }

    INFO::InfoRequestData request = m_requests.take(reply);

    /* send process finished */
    emit info(request, QVariant());
}

