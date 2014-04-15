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


#include "local_playlist_model.h"
#include "core/mediaitem/mediaitem.h"
//#include "debug.h"

#include <QRegExp>

LocalPlaylistModel* LocalPlaylistModel::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class LocalPlaylistModel                                                  *
*                                                                              *
********************************************************************************
*/
LocalPlaylistModel::LocalPlaylistModel(QObject *parent) : QObject(parent)
{
    INSTANCE = this;

    m_rootItem          = MEDIA::MediaPtr(new MEDIA::Media());
    m_filter_pattern    = "";
    m_playing_track     = MEDIA::TrackPtr(0);
}

LocalPlaylistModel::~LocalPlaylistModel()
{
    m_rootItem.reset();
}

MEDIA::MediaPtr LocalPlaylistModel::rootItem()
{
  return m_rootItem;
}


void LocalPlaylistModel::clear()
{
    m_rootItem.reset();
    m_rootItem        = MEDIA::MediaPtr(new MEDIA::Media());

    m_playing_track     = MEDIA::TrackPtr(0);

    emit modelCleared();
}

//! ----------------------- Get urls method ------------------------------------
QList<MEDIA::TrackPtr> LocalPlaylistModel::getItemChildrenTracks(const MEDIA::MediaPtr parent)
{
    QList<MEDIA::TrackPtr> result;

    if(!parent) return result;

    MEDIA::MediaPtr item = parent;

    if (item->type() == TYPE_TRACK || item->type() == TYPE_STREAM) {
      if(isTrackFiltered(item))
        result.append( item ); // works also for STREAM
    }
    else {
      //! Recursive !!
      for (int i = 0; i < item->childCount(); i++) {
        result.append( getItemChildrenTracks(item->child(i)) );
      }
    }
    return result;
}


//! ------------------------- filtering method ---------------------------------
bool LocalPlaylistModel::matches(const QString text) const
{
    if(m_filter_pattern.length() < 3)
      return text.startsWith ( m_filter_pattern, Qt::CaseInsensitive );
    else
      return text.contains ( m_filter_pattern, Qt::CaseInsensitive );
}


bool LocalPlaylistModel::isPlaylistFiltered(const MEDIA::PlaylistPtr playlistItem)
{
    if (m_filter_pattern.isEmpty()) return true;
    if (!playlistItem) return false;


    //! check playlist name
    if ( matches(playlistItem->url) || matches(playlistItem->name) )
      return true;

    //! check playlist child (url name)
    for (int i = 0; i < playlistItem->childCount(); i++)
    {
      MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(playlistItem->child(i));

      if(track->type() == TYPE_TRACK && matches(track->title))
        return true;
      else if(track->type() == TYPE_STREAM  && matches(track->name))
        return true;
     }

    return false;
}



bool LocalPlaylistModel::isTrackFiltered(const MEDIA::TrackPtr track)
{
    if (m_filter_pattern.isEmpty()) return true; 
    if (!track) return false;

    //! check track item
    if(track->type() == TYPE_TRACK && matches(track->title))
      return true;
    else if(track->type() == TYPE_STREAM  && matches(track->name))
      return true;

    //! check parent playlist name & url
    MEDIA::PlaylistPtr playlistItem = MEDIA::PlaylistPtr::staticCast(track->parent());
    if ( matches(playlistItem->url) || matches(playlistItem->name) )
      return true;

    return false;
}
