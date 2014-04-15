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


// local
#include "mediaitem.h"
#include "core/database/database.h"
#include "utilities.h"
#include "debug.h"

// Qt
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlField>

#include <QVariant>
#include <QFileInfo>
#include <QByteArray>
#include <QBuffer>
#include <QUrl>
#include <QFile>
#include <QImage>
#include <QTime>
#include <QPixmap>
#include <QCryptographicHash>

// taglib
#include <taglib/mpegfile.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/mp4file.h>

#include <taglib/vorbisfile.h>
#include <taglib/oggfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/flacfile.h>

// Taglib added support for FLAC pictures in 1.7.0
#if (TAGLIB_MAJOR_VERSION > 1) || (TAGLIB_MAJOR_VERSION == 1 && TAGLIB_MINOR_VERSION >= 7)
# define TAGLIB_HAS_FLAC_PICTURELIST
#endif


const QStringList mediaFilter    = QStringList() << "mp3" << "ogg" << "flac" << "wav" << "m4a" << "aac";
const QStringList playlistFilter = QStringList() << "m3u" << "m3u8" << "pls" << "xspf";

/*
********************************************************************************
*                                                                              *
*    Class Link                                                                *
*                                                                              *
********************************************************************************
*/
MEDIA::Link::Link() : Media()
{
    this->setType(TYPE_LINK);
};

/*
********************************************************************************
*                                                                              *
*    Class Album                                                               *
*                                                                              *
********************************************************************************
*/
MEDIA::Album::Album() : Media()
{
    this->setType(TYPE_ALBUM);

    id             = -1;
    playcount      =  0;
    rating         = -1.0;

    isFavorite     = false;
    isPlaying      = false;
    isUserRating   = false;
};

QString MEDIA::Album::yearToString() const
{
    if (year == -1)
      return QString::null;

    return QString::number(year);
}


QImage MEDIA::Album::image() const
{
    QString path = QString(UTIL::CONFIGDIR + "/albums/" + coverpath);
    return QImage(path);
}

/*
********************************************************************************
*                                                                              *
*    Class Artist                                                              *
*                                                                              *
********************************************************************************
*/
MEDIA::Artist::Artist() : Media()
{
    this->setType(TYPE_ARTIST);

    id             = -1;
    playcount      = 0;
    rating         = -1.0;

    isFavorite     = false;
    isPlaying      = false;
    isUserRating   = false;
};

/*
********************************************************************************
*                                                                              *
*    Class Track                                                               *
*                                                                              *
********************************************************************************
*/
MEDIA::Track::Track() : Media()
{
    this->setType(TYPE_TRACK);

    id           = -1;
    num          = -1;
    year         = -1;
    rating       = -1.0;
    playcount    =  0;
    lastPlayed   = -1;

    isFavorite   = false;
    isPlaying    = false;
    isBroken     = false;
    isPlayed     = false;
    isStopAfter  = false;
}

QString MEDIA::Track::yearToString() const
{
    if (year == -1)
      return QString::null;

    return QString::number(year);
}

QString MEDIA::Track::durationToString() const
{
    return UTIL::durationToString(this->duration);
}

QString MEDIA::Track::path(const QString& filename)
{
    if(MEDIA::isLocal(filename)) {
      return QFileInfo(filename).canonicalFilePath();
    }
    else
      return QUrl(filename).toString();
}

QString MEDIA::Track::coverName() const
{
    return MEDIA::coverName(artist, album);
}

QString MEDIA::Track::lastplayed_ago() const
{
    const QDateTime now  = QDateTime::currentDateTime();
    const QDateTime then = QDateTime::fromTime_t(this->lastPlayed);

    const int days_ago   = then.date().daysTo(now.date());
    const QString s_then = then.toString(QLocale::system().dateFormat(QLocale::ShortFormat));

    if (days_ago == 0)
      return QObject::tr("Today") + " " + s_then;
    if (days_ago == 1)
      return QObject::tr("Yesterday") + " " + s_then;
    if (days_ago <= 7)
      return QObject::tr("%1 days ago").arg(days_ago) + " " + s_then;

    return s_then;
}


/*
********************************************************************************
*                                                                              *
*    Class Playlist                                                            *
*                                                                              *
********************************************************************************
*/
MEDIA::Playlist::Playlist() : Media()
{
    this->setType(TYPE_PLAYLIST);

    id           = -1;
    date         = -1;

    isPlaying    = false;
    isBroken     = false;
}


QString MEDIA::Playlist::dateToString() const
{
  if (date == -1)
    return QString::null;

  QDateTime datetime = QDateTime::fromTime_t(date);

  return datetime.toString ( "dd.MM.yyyy" );
}
/*
********************************************************************************
*                                                                              *
*    Class MEDIA::Media                                                        *
*                                                                              *
********************************************************************************
*/
MEDIA::Media::Media() : QSharedData()
{
    setType(TYPE_EMPTY);
}

MEDIA::Media::~Media()
{
    MEDIA::qResetAll( childItems );
    childItems.clear();
}

MEDIA::MediaPtr MEDIA::Media::child(int index) const
{
    if (index < 0 || index >= childItems.count())
        return MEDIA::MediaPtr(0);

    return childItems.value(index);
}

int MEDIA::Media::childCount() const
{
    return childItems.count();
}

int MEDIA::Media::childNumber() const
{
    return 0;
}

MEDIA::MediaPtr MEDIA::Media::addChildren(T_TYPE type)
{
    MEDIA::MediaPtr childItem = MEDIA::MediaPtr(0);
    switch(type) {
      case TYPE_TRACK   : childItem = MEDIA::TrackPtr( new MEDIA::Track() );break;
      case TYPE_ARTIST  : childItem = MEDIA::ArtistPtr( new MEDIA::Artist() );break;
      case TYPE_ALBUM   : childItem = MEDIA::AlbumPtr( new MEDIA::Album() );break;
      case TYPE_PLAYLIST: childItem = MEDIA::PlaylistPtr( new MEDIA::Playlist() );break;
      case TYPE_LINK    : childItem = MEDIA::LinkPtr( new MEDIA::Link() );break;
      default:  return childItem;
    }

    childItems.append(childItem);
    return childItem;
}

void MEDIA::Media::insertChildren(MEDIA::MediaPtr child)
{
    childItems.append(child);
}

bool MEDIA::Media::removeChildren(int idx)
{
    if (idx < 0 || idx >= childItems.size())
        return false;

    MEDIA::MediaPtr child = childItems.takeAt(idx);
    child.reset();
    delete child.data();
    
    return true;
}

void MEDIA::Media::deleteChildren()
{
    foreach(MEDIA::MediaPtr child, childItems) {
      child.reset();
      delete child.data();
    }
    childItems.clear();
}



/*
********************************************************************************
*                                                                              *
*    namespace MEDIA                                                           *
*                                                                              *
********************************************************************************
*/
void MEDIA::qResetAll(const QList<MEDIA::MediaPtr> mediaList)
{
    foreach(MEDIA::MediaPtr p, mediaList)
      p.reset();
}


float MEDIA::rating(const MediaPtr mi)
{
    if(mi->type() == TYPE_TRACK) {
      return MEDIA::TrackPtr::staticCast( mi )->rating;
    }
    else if(mi->type() == TYPE_ALBUM) {
      return MEDIA::AlbumPtr::staticCast( mi )->rating;
    }
    else if(mi->type() == TYPE_ARTIST) {
      return MEDIA::ArtistPtr::staticCast( mi )->rating;
    }
    else
      return 0.0;

}

/*******************************************************************************
    MEDIA::FromLocalFile
    -> with file path
*******************************************************************************/
MEDIA::TrackPtr MEDIA::FromLocalFile(const QString url, int* p_disc)
{
    //Debug::debug() << " Media Item from local file : " << url;

    //! URL check
    if(url.isEmpty()) return MEDIA::MediaPtr(0);

    MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());
    media->id         = -1;
    media->url        = QFileInfo(url).absoluteFilePath().toUtf8();
    media->name       = QFileInfo(url).baseName();

//     //! encodedName shall be valid as long as fileName exists
//     QByteArray fileName = QFile::encodeName( url );
//     const char * encodedName = fileName.constData();
    #ifdef COMPLEX_TAGLIB_FILENAME
        const wchar_t *encodedName = reinterpret_cast< const wchar_t * >( QFileInfo(url).canonicalFilePath().utf16() );
    #else
        QByteArray fileName = QFile::encodeName( QFileInfo(url).canonicalFilePath() );
        const char *encodedName = fileName.constData();
    #endif
    
    if (!encodedName) {
      Debug::error() << "media item -> encoded path error :" << url;
      media->isBroken   = true;
      return media;

    }

    //! TagLib reference
    TagLib::FileRef fileref = TagLib::FileRef( encodedName, true);
    if (fileref.isNull()) {
      Debug::warning() << "media item -> taglib access failed :" << url;
      media->isBroken   = true;
      return media;
    }

    //! Tag reading
    TagLib::Tag *tag = 0;
    tag = fileref.tag();
    if ( tag ) {
        media->title      = TStringToQString(tag->title()).trimmed();
        media->artist     = TStringToQString(tag->artist()).trimmed();
        media->album      = TStringToQString(tag->album()).trimmed();
        media->genre      = TStringToQString(tag->genre()).trimmed();
        media->num        = tag->track();
        media->year       = tag->year();
    }


    //! Lenght reading
    media->duration   = 0;
    TagLib::AudioProperties *audioProperties = fileref.audioProperties();
    if (audioProperties)
        media->duration   = audioProperties->length(); // Returns the length of the file in seconds

    //! specific tags reading
    QString s_disc;

    if ( TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File *>( fileref.file() ) )
    {
      if ( file->ID3v2Tag() ) {
          if ( !file->ID3v2Tag()->frameListMap()["TPE2"].isEmpty() )
          {
            const QString album_artist = TStringToQString( file->ID3v2Tag()->frameListMap()["TPE2"].front()->toString() ).trimmed();
            if(!album_artist.isEmpty())
              media->artist = album_artist;
          }

          if ( !file->ID3v2Tag()->frameListMap()["TPOS"].isEmpty() )
            s_disc = TStringToQString( file->ID3v2Tag()->frameListMap()["TPOS"].front()->toString() ).trimmed();
      }
    }
    else if (TagLib::Ogg::Vorbis::File* file = dynamic_cast<TagLib::Ogg::Vorbis::File*>( fileref.file() ))
    {
      if (file->tag()) {
        const TagLib::Ogg::FieldListMap& map = file->tag()->fieldListMap();
        if (!map["ALBUMARTIST"].isEmpty()) {
          media->artist = TStringToQString( map["ALBUMARTIST"].front() ).trimmed();
        }
        else if (!map["ALBUM ARTIST"].isEmpty()) {
          media->artist = TStringToQString( map["ALBUM ARTIST"].front() ).trimmed();
        }

        if (!map["DISCNUMBER"].isEmpty() )
          s_disc = TStringToQString( map["DISCNUMBER"].front() ).trimmed();
      }
    }
    else if (TagLib::FLAC::File* file = dynamic_cast<TagLib::FLAC::File*>( fileref.file() ))
    {
      if ( file->xiphComment() ) {
        const TagLib::Ogg::FieldListMap& map = file->xiphComment()->fieldListMap();
        if (!map["ALBUMARTIST"].isEmpty()) {
          media->artist = TStringToQString( map["ALBUMARTIST"].front() ).trimmed();
        }
        else if (!map["ALBUM ARTIST"].isEmpty()) {
          media->artist = TStringToQString( map["ALBUM ARTIST"].front() ).trimmed();
        }

        if (!map["DISCNUMBER"].isEmpty() )
          s_disc = TStringToQString( map["DISCNUMBER"].front() ).trimmed();
      }
    }

    //!DEFAULT values if empty tag
    if (media->artist.isEmpty())
      media->artist = QObject::tr("unknown artist");

    if (media->album.isEmpty())
      media->album = QObject::tr("unknown album");

    if (media->genre.isEmpty())
      media->genre = QObject::tr("unknown genre");

    if (media->title.isEmpty())
      media->title = media->name;

    //! disk number
    if(p_disc != 0) {
      if ( !s_disc.isEmpty() ) 
      {
        int i = s_disc.indexOf('/');
        if ( i != -1 ) {
          *p_disc      = s_disc.left( i ).toInt();
           if( s_disc.right( i ).toInt() == 1)
            *p_disc = 0;
        }
        else
          *p_disc = s_disc.toInt();
      
        //Debug::debug() << " Media Item from local file disc number: " << *p_disc;
      }
      else
        *p_disc = 0;
    }

    //! replaygain metadata
    MEDIA::ReplayGainTagMap map = MEDIA::readReplayGainTags( fileref );
    if ( map.contains( MEDIA::ReplayGain_Track_Gain ) )
      media->trackGain = map[MEDIA::ReplayGain_Track_Gain];

    if ( map.contains( MEDIA::ReplayGain_Track_Peak ) )
      media->trackPeak = map[MEDIA::ReplayGain_Track_Peak];

    if ( map.contains( MEDIA::ReplayGain_Album_Gain ) )
      media->albumGain = map[MEDIA::ReplayGain_Album_Gain];
    else
      media->albumGain = media->trackGain;

    if ( map.contains( MEDIA::ReplayGain_Album_Peak ) )
      media->albumPeak = map[MEDIA::ReplayGain_Album_Peak];
    else
      media->albumPeak = media->trackPeak;

    //! statistics metadata (playcount + rating)
    MEDIA::StatisticTagMap stat_map = MEDIA::readStatisticTags( fileref );
    if ( stat_map.contains( MEDIA::Rating_Tag ) )
      media->rating    = stat_map[MEDIA::Rating_Tag];
    else
      media->rating    = 0.0;

    if ( stat_map.contains( MEDIA::Playcount_Tag ) )
      media->playcount = stat_map[MEDIA::Playcount_Tag];
    else
      media->playcount = 0;

    //! default value
    media->isPlaying    =  false;
    media->isBroken     =  false;
    media->lastPlayed   =  -1;
    media->isPlayed     =  false;
    media->isStopAfter  =  false;

    return media;
}


/*******************************************************************************
    MEDIA::FromDataBase
    -> with track url
*******************************************************************************/
MEDIA::TrackPtr MEDIA::FromDataBase(const QString url)
{
  //Debug::debug() << " Media Item from db : " << url;
  //! search track info into database
  Database db;

  //! Try database connection
  if (db.connect()) {
    QSqlField data("col",QVariant::String);
    data.setValue(QFileInfo(url).canonicalFilePath());
    QString fname = db.sqlDb()->driver()->formatValue(data,false);

    QSqlQuery tracksQuery("SELECT id,filename,trackname, \
       number,length,artist_name,genre_name,album_name,year,last_played, \
       albumgain,albumpeakgain,trackgain,trackpeakgain,playcount,rating \
       FROM view_tracks WHERE filename="+fname+" LIMIT 1;",*db.sqlDb());

    if (tracksQuery.first()) {
      MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());

      //Debug::debug() << " Build MediaItem FROM DATABASE ok";
      media->id         =  tracksQuery.value(0).toInt();
      media->url        =  url;
      media->name       =  tracksQuery.value(1).toString();
      media->title      =  tracksQuery.value(2).toString();
      media->num        =  tracksQuery.value(3).toUInt();
      media->duration   =  tracksQuery.value(4).toInt();
      media->artist     =  tracksQuery.value(5).toString();
      media->genre      =  tracksQuery.value(6).toString();
      media->album      =  tracksQuery.value(7).toString();
      media->year       =  tracksQuery.value(8).toUInt();
      media->lastPlayed =  tracksQuery.value(9).toInt();
      media->albumGain  =  tracksQuery.value(10).value<qreal>();
      media->albumPeak  =  tracksQuery.value(11).value<qreal>();
      media->trackGain  =  tracksQuery.value(12).value<qreal>();
      media->trackPeak  =  tracksQuery.value(13).value<qreal>();
      media->playcount  =  tracksQuery.value(14).toInt();
      media->rating     =  tracksQuery.value(15).toFloat();

      //! default state value
      media->isPlaying    =  false;
      media->isBroken     =  false;
      media->isPlayed     =  false;
      media->isStopAfter  =  false;

      return media;
    }
    //Debug::debug() << " Build MediaItem FROM DATABASE not found " << url;

  } // end connect Db

  return MEDIA::TrackPtr(0);
}

/*******************************************************************************
    MEDIA::FromDataBase
    -> with trackid
*******************************************************************************/
MEDIA::TrackPtr MEDIA::FromDataBase(int trackId)
{
  //Debug::debug() << " Media Item from db -> track id " << trackId;

  //! search track info into database
  Database db;

  //! Try database connection
  if (db.connect()) {

    QSqlQuery tracksQuery("SELECT id,filename,trackname, \
       number,length,artist_name,genre_name,album_name,year,last_played, \
       albumgain,albumpeakgain,trackgain,trackpeakgain,playcount,rating \
       FROM view_tracks WHERE id="+QString::number(trackId)+" LIMIT 1;",*db.sqlDb());

    if (tracksQuery.first()) {
      MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());

      //Debug::debug() << " Build MediaItem FROM DATABASE ok";
      media->id         =  tracksQuery.value(0).toInt();
      media->url        =  tracksQuery.value(1).toString();
      media->name       =  tracksQuery.value(1).toString();
      media->title      =  tracksQuery.value(2).toString();
      media->num        =  tracksQuery.value(3).toUInt();
      media->duration   =  tracksQuery.value(4).toInt();
      media->artist     =  tracksQuery.value(5).toString();
      media->genre      =  tracksQuery.value(6).toString();
      media->album      =  tracksQuery.value(7).toString();
      media->year       =  tracksQuery.value(8).toUInt();
      media->lastPlayed =  tracksQuery.value(9).toInt();
      media->albumGain  =  tracksQuery.value(10).value<qreal>();
      media->albumPeak  =  tracksQuery.value(11).value<qreal>();
      media->trackGain  =  tracksQuery.value(12).value<qreal>();
      media->trackPeak  =  tracksQuery.value(13).value<qreal>();
      media->playcount  =  tracksQuery.value(14).toInt();
      media->rating     =  tracksQuery.value(15).toFloat();

      //! default state value
      media->isPlaying    =  false;
      media->isBroken     =  false;
      media->isPlayed     =  false;
      media->isStopAfter  =  false;

      return media;
    }

  } // end connect Db

  return MEDIA::TrackPtr(0);
}

/*******************************************************************************
    MEDIA::coverName
*******************************************************************************/
QString MEDIA::coverName(const QString& artist,const QString& album)
{
    if( (!artist.isEmpty()) && (!album.isEmpty()) )
    {
      QCryptographicHash hash(QCryptographicHash::Sha1); // or MD5
      hash.addData(artist.toUtf8().constData());
      hash.addData(album.toUtf8().constData());

      return QString(hash.result().toHex() + ".png");
    }

    return QString();
}


/*******************************************************************************
    MEDIA::LoadImageFromFile
*******************************************************************************/
QImage MEDIA::LoadImageFromFile(const QString& filename, QSize size)
{
    QImage image = QImage::fromData( MEDIA::LoadCoverByteArrayFromFile(filename) );

    if(image.isNull())
      return QImage();
    
    if(size != image.size()) {
        image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
  
    return image;
}



/*******************************************************************************
    MEDIA::LoadCoverFromFile
*******************************************************************************/
QPixmap MEDIA::LoadCoverFromFile(const QString& filename, QSize size)
{
    QImage image = QImage::fromData( MEDIA::LoadCoverByteArrayFromFile(filename) );

    if(image.isNull())
      return QPixmap();
    
    if(size != image.size()) {
        image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
  
    return QPixmap::fromImage( image );
}

/*******************************************************************************
    MEDIA::LoadCoverByteArrayFromFile
*******************************************************************************/
QByteArray MEDIA::LoadCoverByteArrayFromFile(const QString& filename)
{
    if (filename.isEmpty())
      return QByteArray();
    
    TagLib::FileRef ref(QFile::encodeName(filename).constData());

    if (ref.isNull() || !ref.file())
      return QByteArray();
    
    /*-----------------------------------------------------------*/
    /* MP3                                                       */
    /* ----------------------------------------------------------*/    
    TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(ref.file());
    if (file && file->ID3v2Tag())
    {      
      TagLib::ID3v2::FrameList apic_frames = file->ID3v2Tag()->frameListMap()["APIC"];
      if (apic_frames.isEmpty())
        return QByteArray();

      if (apic_frames.size() != 1) 
      {
        TagLib::ID3v2::FrameList::Iterator it = apic_frames.begin();
        for (; it != apic_frames.end(); ++it) 
        {
            // This must be dynamic_cast<>, TagLib will return UnknownFrame in APIC for encrypted frames.
            TagLib::ID3v2::AttachedPictureFrame *frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(*it);

            // Both thumbnail and full size should use FrontCover, as FileIcon may be too small even for thumbnail.
            if (frame && frame->type() != TagLib::ID3v2::AttachedPictureFrame::FrontCover)
              continue;

            return QByteArray((const char*) frame->picture().data(), frame->picture().size());
        }
      }

      // If we get here we failed to pick a picture, or there was only one, so just use the first picture.
      TagLib::ID3v2::AttachedPictureFrame *frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(apic_frames.front());
      return QByteArray((const char*) frame->picture().data(), frame->picture().size());
    }

    /*-----------------------------------------------------------*/
    /* Ogg vorbis/speex                                          */
    /* ----------------------------------------------------------*/    
    TagLib::Ogg::XiphComment* xiph_comment = dynamic_cast<TagLib::Ogg::XiphComment*>(ref.file()->tag());

    if (xiph_comment)
    {
      TagLib::Ogg::FieldListMap map = xiph_comment->fieldListMap();

      // Ogg lacks a definitive standard for embedding cover art, but it seems
      // b64 encoding a field called COVERART is the general convention
      if (!map.contains("COVERART"))
        return QByteArray();

      return QByteArray::fromBase64(map["COVERART"].toString().toCString());
    }
  

    /*-----------------------------------------------------------*/
    /* FLAC                                                      */
    /* ----------------------------------------------------------*/
#ifdef TAGLIB_HAS_FLAC_PICTURELIST
    TagLib::FLAC::File* flac_file = dynamic_cast<TagLib::FLAC::File*>(ref.file());
    if (flac_file && flac_file->xiphComment()) 
    {
        const TagLib::List<TagLib::FLAC::Picture*> picturelist = flac_file->pictureList();
        for( TagLib::List<TagLib::FLAC::Picture*>::ConstIterator it = picturelist.begin(); it != picturelist.end(); it++ )
        {
            TagLib::FLAC::Picture* picture = *it;

            if( ( picture->type() == TagLib::FLAC::Picture::FrontCover || picture->type() == TagLib::FLAC::Picture::Other ))
            {
              return QByteArray(picture->data().data(), picture->data().size());
            }
        }
    }
#endif  

    /*-----------------------------------------------------------*/
    /* MP4/AAC                                                   */
    /* ----------------------------------------------------------*/
    TagLib::MP4::File* aac_file = dynamic_cast<TagLib::MP4::File*>(ref.file());
    if (aac_file) 
    {
      TagLib::MP4::Tag* tag = aac_file->tag();
      const TagLib::MP4::ItemListMap& items = tag->itemListMap();
      TagLib::MP4::ItemListMap::ConstIterator it = items.find("covr");
      if (it != items.end()) 
      {
        const TagLib::MP4::CoverArtList& art_list = it->second.toCoverArtList();

        if (!art_list.isEmpty()) 
        {
          // Just take the first one for now
          const TagLib::MP4::CoverArt& art = art_list.front();
          return QByteArray(art.data().data(), art.data().size());
        }
      }
    }

    return QByteArray();
}

/*******************************************************************************
    MEDIA::isLocal
*******************************************************************************/
bool MEDIA::isLocal(const QString& url)
{
    if (url.contains(QRegExp("^[a-z]{3,}:"))) {
      if (QUrl(url).scheme() == "file")
        return true;
      else
        return false;
    }
    return true;
}

/*******************************************************************************
    Media Filter
*******************************************************************************/
bool MEDIA::isAudioFile(const QString& url)
{
    return mediaFilter.contains(QFileInfo(url).suffix().toLower());
}

bool MEDIA::isPlaylistFile(const QString& url)
{
    return playlistFilter.contains(QFileInfo(url).suffix().toLower());
}

bool MEDIA::isCueFile(const QString& url)
{
    const QStringList cue_extension = QStringList() << "cue";
    return cue_extension.contains(QFileInfo(url).suffix().toLower());
}


bool MEDIA::isShoutCastUrl(const QString& url)
{
    return url.contains("pls?id");
}

bool MEDIA::isTuneInUrl(const QString& url)
{
    return url.contains("ashx?id");
}

/*******************************************************************************
    Compare/Sorting utilities
*******************************************************************************/
bool MEDIA::compareTrackItemGenre(const TrackPtr mi1, const TrackPtr mi2)
{
    const QString s1 = mi1->genre + mi1->album + QString::number(mi1->disc_number);
    const QString s2 = mi2->genre + mi2->album + QString::number(mi2->disc_number);

    return (s1 < s2);
}

bool MEDIA::compareAlbumItemYear(const AlbumPtr mi1, const AlbumPtr mi2)
{
    return mi1->year > mi2->year;
}

bool MEDIA::compareStreamName(const TrackPtr mi1, const TrackPtr mi2)
{
    return mi1->name.toLower() < mi2->name.toLower();
}

bool MEDIA::compareStreamCategorie(const TrackPtr mi1, const TrackPtr mi2)
{
    return mi1->categorie.toLower() <= mi2->categorie.toLower();
}

bool MEDIA::compareAlbumItemPlaycount(const AlbumPtr mi1, const AlbumPtr mi2)
{
    return mi1->playcount > mi2->playcount;
}

bool MEDIA::compareArtistItemPlaycount(const ArtistPtr mi1, const ArtistPtr mi2)
{
    return mi1->playcount > mi2->playcount;
}

bool MEDIA::compareAlbumItemRating(const AlbumPtr mi1, const AlbumPtr mi2)
{
   return mi1->rating > mi2->rating;
}

bool MEDIA::compareArtistItemRating(const ArtistPtr mi1, const ArtistPtr mi2)
{
   return mi1->rating > mi2->rating;
}


void MEDIA::registerTrackPlaying(MEDIA::TrackPtr tk, bool isPlaying) 
{
    MEDIA::MediaPtr media = tk;
    
    do
    {
      switch (media->type()) {
        case TYPE_ARTIST   : MEDIA::ArtistPtr::staticCast(media)->isPlaying   = isPlaying; break;
        case TYPE_ALBUM    : MEDIA::AlbumPtr::staticCast(media)->isPlaying    = isPlaying; break;
        case TYPE_PLAYLIST : MEDIA::PlaylistPtr::staticCast(media)->isPlaying = isPlaying; break;
        case TYPE_STREAM   :
        case TYPE_TRACK    : MEDIA::TrackPtr::staticCast(media)->isPlaying    = isPlaying; 
                             MEDIA::TrackPtr::staticCast(media)->isPlayed     = isPlaying; 
                             
                             if(isPlaying)
                               MEDIA::TrackPtr::staticCast(media)->isBroken     = false; 
                             break;
        default: break;
      }
    
      media  = media->parent();
    } while(media);
}

void MEDIA::registerTrackBroken(MEDIA::TrackPtr tk, bool isBroken)
{
    MEDIA::MediaPtr media = tk;
    
    do
    {
      if (media->type() == TYPE_STREAM || media->type() == TYPE_TRACK ) 
      {
           MEDIA::TrackPtr::staticCast(media)->isBroken     = isBroken; 
           MEDIA::TrackPtr::staticCast(media)->isPlaying    = false; 
      }
    
      media  = media->parent();
    } while(media);
}
