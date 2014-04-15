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


#include "covercache.h"
#include "utilities.h"
#include "debug.h"

#include <QFile>
#include <QPixmap>
#include <QPainter>

static const QString noCoverKey  = QString(":/images/emptycoverblue110x110.png");
static const QString urlCoverKey = QString(":/images/media-url-110x110.png");
/*
********************************************************************************
*                                                                              *
*    Class CoverCache                                                          *
*                                                                              *
********************************************************************************
*/
CoverCache* CoverCache::INSTANCE = 0;

CoverCache* CoverCache::instance()
{
    return INSTANCE;
}

CoverCache::CoverCache()
{
    INSTANCE = this;
}

CoverCache::~CoverCache()
{
}

QPixmap CoverCache::cover( const MEDIA::AlbumPtr album )
{
    QPixmap pixmap;

    QPixmapCache::Key key = m_keys.value( album );

    if( key != QPixmapCache::Key() && QPixmapCache::find( key, &pixmap ) )
      return pixmap;

    /* no pixmap in cache */
    QImage image = album->image();

    if(!image.isNull())
    {
      pixmap = QPixmap::fromImage( image );
      m_keys[album] = QPixmapCache::insert( pixmap );
      return pixmap;
    }
    else
    {
      if( QPixmapCache::find( noCoverKey, &pixmap ) )
        return pixmap;

      pixmap = QPixmap(":/images/emptycoverblue110x110.png");
      QPixmapCache::insert( noCoverKey, pixmap );
      return pixmap;
    }
    return pixmap;
}

QPixmap CoverCache::cover(const MEDIA::TrackPtr track )
{
    QPixmap pixmap = QPixmap();

    if(!track)
      return pixmap;

    if(track->type() != TYPE_TRACK)
    {
        //! case for tunein stream with image dowloaded image
        if(!track->image.isNull()) 
        {
            QPixmapCache::Key key = m_keys.value( track );

            if( key != QPixmapCache::Key() && QPixmapCache::find( key, &pixmap ) )
               return pixmap;
    
            pixmap = get_stream_pixmap(track);
            m_keys[track] = QPixmapCache::insert( pixmap );
            return pixmap;
        }
      
        if( QPixmapCache::find( urlCoverKey, &pixmap ) )
          return pixmap;

        pixmap = QPixmap(":/images/media-url-110x110.png");
        QPixmapCache::insert( urlCoverKey, pixmap );
        return pixmap;
    }
    else if (track->id == -1)
    {
        QPixmap pixmap = MEDIA::LoadCoverFromFile( track->url, QSize(110,110) );
        if(!pixmap.isNull())
          return pixmap;

        if( QPixmapCache::find( noCoverKey, &pixmap ) )
          return pixmap;

        pixmap = QPixmap(":/images/emptycoverblue110x110.png");
        QPixmapCache::insert( noCoverKey, pixmap );
        return pixmap;
    }
    else // track exist in collection
    {
        //! 1: first check cover for parent album
        if(track->parent() &&  track->parent()->type() == TYPE_ALBUM) {
          MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(track->parent());
          if(album)
            return cover(album);
        }


        //! 2: check cover path in /<config fir>/albums/
        QString path = UTIL::CONFIGDIR + "/albums/" + track->coverName();
        if( QFile(path).exists() )
          return QPixmap(path);
        else
          return QPixmap(":/images/emptycoverblue110x110.png");
    }

    return pixmap;
}

void CoverCache::invalidate( const MEDIA::AlbumPtr album )
{
    if( !m_keys.contains( album ) )
        return;

    QPixmapCache::Key key = m_keys.value( album );
    QPixmapCache::remove( key );
}


QPixmap CoverCache::get_stream_pixmap(MEDIA::TrackPtr track)
{
     QPixmap pixTemp(QSize(110,110));
     {
      pixTemp.fill(Qt::transparent);
      QPainter p;
      p.begin(&pixTemp);

      
      p.drawPixmap( (110 - track->image.width())/2,3, QPixmap::fromImage(track->image));
      p.end();
     }
     
     return pixTemp;
}



