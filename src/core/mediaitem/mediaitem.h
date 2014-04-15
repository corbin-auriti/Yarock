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

#ifndef _MEDIAITEM_H_
#define _MEDIAITEM_H_

// Qt
#include <QVariant>
#include <QString>
#include <QSize>
#include <QList>
#include <QSharedData>
#include <QStringList>
#include <QMap>
#include <QPixmap>

// local
#include "shareddata.h"


// taglib
#include <taglib/fileref.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>


/*
********************************************************************************
*    Typedef                                                                   *
********************************************************************************
*/
// MediaItem type
typedef enum {TYPE_EMPTY = 0, 
              TYPE_ARTIST, 
              TYPE_ALBUM, 
              TYPE_TRACK, 
              TYPE_TRACK_CUE, 
              TYPE_STREAM, 
              TYPE_PLAYLIST,
              TYPE_LINK } T_TYPE;

// MediaItem Playlist type
typedef enum {T_DATABASE = 0, T_FILE, T_SMART} T_PLAYLIST;

/*
********************************************************************************
*    Class                                                                     *
********************************************************************************
*/
namespace MEDIA
{
    class Media;       // base shared data class
    class Artist;
    class Album;
    class Track;
    class Playlist;
    class Link;
    

    typedef ExplicitlySharedDataPointer<Media>    MediaPtr;
    typedef ExplicitlySharedDataPointer<Artist>   ArtistPtr;
    typedef ExplicitlySharedDataPointer<Album>    AlbumPtr;
    typedef ExplicitlySharedDataPointer<Track>    TrackPtr;
    typedef ExplicitlySharedDataPointer<Playlist> PlaylistPtr;
    typedef ExplicitlySharedDataPointer<Link>     LinkPtr;


class Media : public QSharedData
{
  public:
    Media();
    ~Media();

    T_TYPE type() const {return t_type;}
    void setType(T_TYPE t) {t_type = t;}

    // Simple getters/accessors
    MediaPtr parent() const { return parentItem;}
    void setParent(const MediaPtr p) { parentItem = p;}

    MediaPtr child(int index) const;
    QList<MediaPtr> children() const { return childItems;}

    int childCount() const;
    int childNumber() const;

    // Methods
    bool removeChildren(int idx);
    MediaPtr addChildren(T_TYPE type);
    void insertChildren(MediaPtr child);
    void deleteChildren();

    
  private:
    T_TYPE             t_type;
    QList<MediaPtr>    childItems;
    MediaPtr           parentItem;
};


class Link : public Media
{
  public:
    Link();
    ~Link(){};

    /*------ ATTRIBUTS ------*/
    QString      name;
    QString      categorie;
    QString      url;
    int          state;
};


class Artist : public Media
{
  public:
    Artist();
    ~Artist(){};

    /*------ ATTRIBUTS ------*/
    int          id;
    QString      name;
    QList<AlbumPtr>  album_covers;
    float        rating;
    int          playcount;

    bool         isFavorite;
    bool         isPlaying;
    bool         isUserRating;
};


class Album : public Media
{
  public:
    Album();
    ~Album(){};

    QString yearToString() const;
    QImage image() const;
    bool isMultiset() {return !ids.isEmpty();}

    /*------ ATTRIBUTS ------*/
    int          id;
    QList<int>   ids; // for multiset grouped album
    QString      name;
    int          year;
    QString      coverpath;
    int          playcount;
    float        rating;
    int          disc_number; // or disc_count for multiset

    bool         isFavorite;
    bool         isPlaying;
    bool         isUserRating;
};


class Track : public Media
{
  public:
    Track();
    ~Track() {};

    QString durationToString() const;
    QString yearToString() const;
    static QString path(const QString& filename);
    QString coverName() const;
    QString lastplayed_ago() const;

    /*------ ATTRIBUTS ------*/
    int          id;
    QString      url;
    QString      name;
    QString      title;
    QString      artist;
    QString      album;
    QString      genre;
    QString      categorie;
    QImage       image;

    int          duration;   // (int) durée (second)
    uint         num;        // (uint) Numero
    int          year;
    int          lastPlayed; // contains QDateTime value (see HistoryManager)
    int          playcount;
    qreal        trackGain;
    qreal        trackPeak;
    qreal        albumGain;
    qreal        albumPeak;
    float        rating;

    bool         isFavorite;
    bool         isPlaying;
    bool         isBroken;
    bool         isPlayed;
    bool         isStopAfter;
    int          disc_number;
};

class Playlist : public Media
{
  public:
    Playlist();
    ~Playlist(){};

    QString dateToString() const;

    /*------ ATTRIBUTS ------*/
    int          id;
    T_PLAYLIST   p_type;    // playlist type: 0 = internal, 1 = user
    QString      url;
    QString      name;
    QString      icon;
    QVariant     rules;
    int          date;      // contains QDateTime

    bool         isFavorite;
    bool         isPlaying;
    bool         isBroken;
};

} // end namespace MEDIA


Q_DECLARE_METATYPE( MEDIA::MediaPtr )
Q_DECLARE_METATYPE( MEDIA::LinkPtr )
Q_DECLARE_METATYPE( MEDIA::ArtistPtr )
Q_DECLARE_METATYPE( MEDIA::AlbumPtr )
Q_DECLARE_METATYPE( MEDIA::TrackPtr )
Q_DECLARE_METATYPE( MEDIA::PlaylistPtr )



/*
********************************************************************************
*                                                                              *
*    namespace MEDIA                                                           *
*                                                                              *
********************************************************************************
*/
namespace MEDIA {
  void qResetAll(const QList<MediaPtr> mediaList);

  //! ------ Rating ------------------------------------------------------------
  float rating(const MediaPtr mi);

  //! ------ PLaying/Broken status ---------------------------------------------
  void registerTrackPlaying(MEDIA::TrackPtr tk, bool isPlaying);
  void registerTrackBroken(MEDIA::TrackPtr tk, bool isBroken);
  
  
  //! ------ Media Filter ------------------------------------------------------
  bool isAudioFile(const QString& url);
  bool isPlaylistFile(const QString& url);
  bool isCueFile(const QString& url);
  bool isShoutCastUrl(const QString& url);
  bool isTuneInUrl(const QString& url);

  //! ------ Remote Url management ---------------------------------------------
  bool isLocal(const QString& url);

  //! ------ Read Tag with Taglib methods --------------------------------------
  QImage LoadImageFromFile(const QString& filename, QSize size = QSize(110,110));
  QPixmap LoadCoverFromFile(const QString& filename, QSize size = QSize(110,110));
  QByteArray LoadCoverByteArrayFromFile(const QString& filename);

  // add detection of disk number FIXME (not very clean way)
  TrackPtr FromLocalFile(const QString url, int* p_disc=0);
  TrackPtr FromDataBase(const QString url);
  TrackPtr FromDataBase(int trackId);

  //! ------ cover utilities ---------------------------------------------------
  QString coverName(const QString& artist, const QString& album);

  //! --- sort utilities -------------------------------------------------------
  bool compareTrackItemGenre(const TrackPtr, const TrackPtr);
  bool compareAlbumItemYear(const AlbumPtr, const AlbumPtr);
  bool compareAlbumItemPlaycount(const AlbumPtr, const AlbumPtr);
  bool compareAlbumItemRating(const AlbumPtr, const AlbumPtr);
  bool compareArtistItemPlaycount(const ArtistPtr, const ArtistPtr);
  bool compareArtistItemRating(const ArtistPtr, const ArtistPtr);
  bool compareStreamName(const TrackPtr, const TrackPtr);
  bool compareStreamCategorie(const TrackPtr, const TrackPtr);

  //! --- replay gain utilities ------------------------------------------------
  enum ReplayGainTag
  {
        ReplayGain_Track_Gain,
        ReplayGain_Track_Peak,
        ReplayGain_Album_Gain,
        ReplayGain_Album_Peak
  };
  typedef QMap<ReplayGainTag, qreal> ReplayGainTagMap;
  ReplayGainTagMap readReplayGainTags( TagLib::FileRef fileref );

    //! --- statistic tags utilities -------------------------------------------
  enum StatisticTag
  {
        Rating_Tag,
        Playcount_Tag
  };
  typedef QMap<StatisticTag, int> StatisticTagMap;
  StatisticTagMap readStatisticTags( TagLib::FileRef fileref );

} // end namespace MEDIA

#endif //_MEDIAITEM_H_
