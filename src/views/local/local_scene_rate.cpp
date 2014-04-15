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

#include "local_scene.h"

#include "models/local/local_track_model.h"
#include "core/mediaitem/mediaitem.h"
#include "core/database/database.h"

#include "debug.h"

#include <QSqlQuery>
#include <QtGui>

/*******************************************************************************
    LocalScene::rateTrack
*******************************************************************************/
void LocalScene::rateTrack(QGraphicsItem* gi)
{
    TrackGraphicItem *item = static_cast<TrackGraphicItem*>(gi);

    MEDIA::TrackPtr track = item->media;
    if(track->id != -1) {
      Database db;
      if (!db.connect()) return;

      QSqlQuery q("", *db.sqlDb());
      q.prepare("UPDATE `tracks` SET `rating`=:rat WHERE `id`=:id;");
      q.bindValue(":rat", track->rating );
      q.bindValue(":id", track->id );
      q.exec();

      MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(track->parent());
      if(!album->isUserRating)
         album->rating = m_localTrackModel->getItemAutoRating(album);

      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(album->parent());
      if(!artist->isUserRating)
         artist->rating = m_localTrackModel->getItemAutoRating(artist);
     }
}

/*******************************************************************************
    LocalScene::rateArtist
*******************************************************************************/
void LocalScene::rateArtist(QGraphicsItem* gi)
{
    ArtistGraphicItem *item = static_cast<ArtistGraphicItem*>(gi);

    MEDIA::ArtistPtr artist = item->media;

    if(artist->id != -1) {
      Database db;
      if (!db.connect()) return;

      QSqlQuery q("", *db.sqlDb());
      q.prepare("UPDATE `artists` SET `rating`=:rat WHERE `id`=:id;");
      q.bindValue(":rat", artist->rating );
      q.bindValue(":id", artist->id );
      q.exec();
     }
}

/*******************************************************************************
    LocalScene::rateAlbum
*******************************************************************************/
void LocalScene::rateAlbum(QGraphicsItem* gi)
{
    Debug::debug() << "## LocalScene::rateAlbum";
    AlbumGraphicItem *item = static_cast<AlbumGraphicItem *>(gi);

    MEDIA::AlbumPtr album = item->media;
    if(album->id != -1) 
    {
      QList<int> db_ids;
      if(album->isMultiset())
        db_ids << album->ids;
      else
        db_ids << album->id;
    
      Database db;
      if (!db.connect()) return;

      foreach (const int &id, db_ids) 
      {      
        QSqlQuery q("", *db.sqlDb());
        q.prepare("UPDATE `albums` SET `rating`=:rat WHERE `id`=:id;");
        q.bindValue(":rat", album->rating );
        q.bindValue(":id", id );
        Debug::debug() << "database -> rate album :" << q.exec();

        MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(album->parent());
        if(!artist->isUserRating)
           artist->rating = m_localTrackModel->getItemAutoRating(artist);
      }
   }
}
/*******************************************************************************
    LocalScene::rateSelection
*******************************************************************************/
void LocalScene::rateSelection(QList<QGraphicsItem*> selection)
{
    Debug::debug() << "LocalScene::rateSelection";
    QtConcurrent::run(this, &LocalScene::threadedRateSelection, selection);
}

void LocalScene::threadedRateSelection(QList<QGraphicsItem*> selection)
{
    foreach(QGraphicsItem* gi, selection)
    {
      switch(gi->type()) {
        case GraphicsItem::AlbumType      :
        case GraphicsItem::AlbumGenreType : rateAlbum(gi);break;
        case GraphicsItem::ArtistType     : rateArtist(gi);break;
        case GraphicsItem::TrackType      : rateTrack(gi);break;
        default: return;break;
      }
    } // end foreach
}



