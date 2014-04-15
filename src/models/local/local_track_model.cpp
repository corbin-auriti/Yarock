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


#include "local_track_model.h"
#include "core/mediaitem/mediaitem.h"
#include "debug.h"

#include <QRegExp>

LocalTrackModel* LocalTrackModel::INSTANCE = 0;


/*
********************************************************************************
*                                                                              *
*    Class LocalTrackModel                                                     *
*                                                                              *
********************************************************************************
*/
LocalTrackModel::LocalTrackModel(QObject *parent) : QObject(parent)
{
    INSTANCE         = this;

    m_rootItem       = MEDIA::MediaPtr(new MEDIA::Media());
    m_playing_track  = MEDIA::TrackPtr(0);
    m_filter_pattern = "";
}

LocalTrackModel::~LocalTrackModel()
{
    m_rootItem.reset();
}


MEDIA::MediaPtr LocalTrackModel::rootItem()
{
    return m_rootItem;
}


void LocalTrackModel::clear()
{
    m_rootItem.reset();
    m_rootItem = MEDIA::MediaPtr(new MEDIA::Media());

    trackItemHash.clear();
    albumItemList.clear();

    m_playing_track  = MEDIA::TrackPtr(0);

    emit modelCleared();
}


bool LocalTrackModel::isEmpty() const
{
    return m_rootItem->childCount() == 0;
}
     
     

//! ----------------------- Get tracks method ----------------------------------
QList<MEDIA::TrackPtr> LocalTrackModel::getItemChildrenTracks(const MEDIA::MediaPtr parent)
{
    //Debug::debug() << "LocalTrackModel::getItemChildrenTracks";

    QList<MEDIA::TrackPtr> result;
    if(!parent)
      return result;

    MEDIA::MediaPtr media = parent;

    if (media->type() == TYPE_TRACK) {
      if(isTrackFiltered(MEDIA::TrackPtr::staticCast(media))) {
        result.append( MEDIA::TrackPtr::staticCast(media) );
      }
    }
    else {
      //! Recursive !!
      for (int i = 0; i < media->childCount(); i++) {
        result.append( getItemChildrenTracks(media->child(i)) );
      }
    }

    return result;
}

QList<MEDIA::TrackPtr> LocalTrackModel::getAlbumChildrenTracksGenre(const MEDIA::AlbumPtr album,const QString& genre)
{
    QList<MEDIA::TrackPtr> result;

    if(!album) return result;
    if(album->type() != TYPE_ALBUM) return result;

    for (int i = 0; i < album->childCount(); i++) {
      MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(album->child(i));
      if(!isTrackFiltered(track)) continue;

      if (!QRegExp(genre, Qt::CaseSensitive).exactMatch(track->genre)) continue;

      result.append( MEDIA::TrackPtr::staticCast(album->child(i)) );
    }
    return result;
}


//! ----------------------- Get rating method ----------------------------------
float LocalTrackModel::getItemAutoRating(const MEDIA::MediaPtr media)
{
    float auto_rating = 0.0;
    int   count = 0;
    if(!media) return auto_rating;

    if (media->type() == TYPE_TRACK)
    {
        //Debug::debug() << " => getItemAutoRating : return track rating = " << media->data.rating;

        return MEDIA::TrackPtr::staticCast(media)->rating;
    }
    else {
      //! Recursive !!
      for (/*count*/; count < media->childCount(); count++)
      {
        auto_rating += getItemAutoRating( media->child(count) );
      }
    }

    float result =  media->childCount()!=0 ? float(auto_rating/(media->childCount())) : 0 ;
    result = double(int(result * 5 * 2 + 0.5)) / (5 * 2);
    //Debug::debug() << " => getItemAutoRating : rating = " << result;
    return result;
}





//! ------------------------- filtering method ---------------------------------
bool LocalTrackModel::matches(const QString text) const
{
    if(m_filter_pattern.length() < 3)
      return text.startsWith ( m_filter_pattern, Qt::CaseInsensitive );
    else
      return text.contains ( m_filter_pattern, Qt::CaseInsensitive );
}



bool LocalTrackModel::isArtistFiltered(const MEDIA::ArtistPtr artistItem)
{
    if (m_filter_pattern.isEmpty()) return true;
    if (!artistItem) return false;

    //! check artist item
    if(matches(artistItem->name))
      return true;
   
    //! check child album
    for (int i = 0; i < artistItem->childCount(); i++)
    {
      MEDIA::AlbumPtr albumItem = MEDIA::AlbumPtr::staticCast( artistItem->child(i) );

      if(matches(albumItem->name))
        return true;

      //! check child track
      for (int j = 0; j < albumItem->childCount(); j++)
      {
        MEDIA::TrackPtr trackItem = MEDIA::TrackPtr::staticCast( albumItem->child(j) );

        if(matches(trackItem->url) || matches(trackItem->genre))
          return true;
       }
    }

    return false;
}

bool LocalTrackModel::isAlbumFiltered(const MEDIA::AlbumPtr albumItem)
{
    if (m_filter_pattern.isEmpty()) return true;
    if (!albumItem) return false;

    //! check album item
    if(matches(albumItem->name))
      return true;

    //! check artist parent
    MEDIA::ArtistPtr artistItem = MEDIA::ArtistPtr::staticCast( albumItem->parent() );
    if(matches(artistItem->name))
      return true;
  
    //! check child track
    for (int i = 0; i < albumItem->childCount(); i++)
    {
      MEDIA::TrackPtr trackItem = MEDIA::TrackPtr::staticCast( albumItem->child(i) );

      if(matches(trackItem->url) || matches(trackItem->genre))
        return true;
    }

    return false;
}


bool LocalTrackModel::isTrackFiltered(const MEDIA::TrackPtr trackItem)
{
    if (m_filter_pattern.isEmpty()) return true;
    if (!trackItem) return false;

    //! check track item
    if (matches(trackItem->url) || matches(trackItem->genre) ||
        matches(trackItem->artist) || matches(trackItem->album))
      return true;

    return false;
}

