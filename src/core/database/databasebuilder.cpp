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
#include "core/database/databasebuilder.h"
#include "core/database/database.h"
#include "core/database/databasemanager.h"

#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/playlist_parser.h"

#include "utilities.h"
#include "debug.h"

// taglib
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>
#include <taglib/mpegfile.h>

// Qt
#include <QtCore>
#include <QImage>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>

#include <QFuture>
#include <QFutureWatcher>


/*
********************************************************************************
*                                                                              *
*    Class DataBaseBuilder                                                     *
*                                                                              *
********************************************************************************
*/
DataBaseBuilder::DataBaseBuilder()
{
    m_exit            = false;
}

/*******************************************************************************
   DataBaseBuilder::readFsFiles
*******************************************************************************/
void DataBaseBuilder::readFsFiles()
{
  const QStringList filters = QStringList()
  /* Audio */    << "*.mp3"  << "*.ogg" << "*.wav" << "*.flac" << "*.m4a" << "*.aac"
  /* Playlist */ << "*.m3u" << "*.m3u8" << "*.pls" << "*.xspf";

  foreach(const QString& root_dir, m_folders)
  {
    QDir dir(root_dir);
    dir.setNameFilters(filters);

    QDirIterator it(dir,QDirIterator::Subdirectories);
    while(it.hasNext())
    {
      it.next();
      if (!it.fileInfo().isDir())
      {
         QString file_path = it.fileInfo().absoluteFilePath().toUtf8();
         if(!m_fs_files.contains(file_path))
           m_fs_files << file_path;
      }
    }
  }
  Debug::debug() << "file count :" << m_fs_files.count();
}



/*******************************************************************************
   DataBaseBuilder::rebuildFolder
     -> User entry point : add folder to parse
*******************************************************************************/
void DataBaseBuilder::rebuildFolder(QStringList folder)
{
    m_folders.clear();
    m_db_files.clear();
    m_fs_files.clear();
    m_folders.append(folder);
}

/*******************************************************************************
   DataBaseBuilder::run
*******************************************************************************/
void DataBaseBuilder::run()
{
    if (m_folders.isEmpty()) return;
    int idxCount   = 0;
    int fileCount  = 0;

    Database db;
    if (!db.connect()) return;
    m_sqlDb = db.sqlDb();

    Debug::debug() << "- DataBaseBuilder -> starting Database update";

    /*-----------------------------------------------------------*/
    /* Get files from filesystem                                 */
    /* ----------------------------------------------------------*/
    readFsFiles();
    fileCount = m_fs_files.count();

    /*-----------------------------------------------------------*/
    /* Get files from database                                   */
    /* ----------------------------------------------------------*/
    QSqlQuery trackQuery("SELECT filename, mtime FROM tracks;",*m_sqlDb);
    while (trackQuery.next())
      m_db_files.insert(trackQuery.value(0).toString(),trackQuery.value(1).toUInt());

    QSqlQuery playlistQuery("SELECT filename, mtime FROM playlists WHERE type=1;",*m_sqlDb);
    while (playlistQuery.next())
      m_db_files.insert(playlistQuery.value(0).toString(),playlistQuery.value(1).toUInt());

    /*-----------------------------------------------------------*/
    /* Update database                                           */
    /* ----------------------------------------------------------*/
    //On SQLite --> it's MUCH faster to have everything in one transaction
    //with only one disk write than to commit every each insert individually
    QSqlQuery("BEGIN TRANSACTION;",*m_sqlDb);

    foreach(const QString& filepath, m_fs_files)
    {
      if(m_exit)
        break;

      //! If the file is NOT in database then insert
      if (!m_db_files.contains(filepath) )
      {
        if (MEDIA::isAudioFile(filepath) )
          insertTrack(filepath);
        else
          insertPlaylist(filepath);
      }
      //! If the file is in database but has another mtime then update it
      else if (m_db_files[filepath] != QFileInfo(filepath).lastModified().toTime_t())
      {
        if (MEDIA::isAudioFile(filepath) )
          updateTrack(filepath);
        else
          updatePlaylist(filepath);
       }

       m_db_files.remove(filepath);

       //! signal progress
       if(fileCount > 0) {
         int percent = 100 - ((fileCount - ++idxCount) * 100 / fileCount);
         emit buildingProgress(percent);
       }
    } // end foreach file in filesystem


    //! Get files that are in DB but not on filesystem
    QHashIterator<QString, uint> i(m_db_files);
    while (i.hasNext()) {
        i.next();
        if( MEDIA::isAudioFile(i.key()) )
          removeTrack(i.key());
        else
          removePlaylist(i.key());
    }

    m_db_files.clear();

    // Check for interprets/albums/genres... that are not used anymore
    cleanUpDatabase();

    // Store last update time
    QSqlQuery q("UPDATE `db_attribute` SET `value`=:date WHERE `name`=lastUpdate;",*m_sqlDb);
    q.bindValue(":date", QDateTime::currentDateTime().toTime_t());
    q.exec();

    // Now write all data to the disk
    QSqlQuery("COMMIT TRANSACTION;",*m_sqlDb);

    Debug::debug() << "- DataBaseBuilder -> end Database update";
    if(!m_exit)
      emit buildingFinished();
}



/*******************************************************************************
   DataBaseBuilder::insertTrack
     -> MEDIA::FromLocalFile(fname) to get track metadata
     -> MEDIA::coverName(media) to get hash of covername
*******************************************************************************/
void DataBaseBuilder::insertTrack(const QString& filename)
{
    QFileInfo fileInfo(filename);

    QString fname = fileInfo.filePath().toUtf8();

    uint mtime = QFileInfo(filename).lastModified().toTime_t();

    Debug::debug() << "- DataBaseBuilder -> insert track :" << filename;

    //! storage localtion
    QString storageLocation = UTIL::CONFIGDIR + "/albums/";

    //! Read tag from URL file (with taglib)
    int disc_number = 0;
    MEDIA::TrackPtr track = MEDIA::FromLocalFile(fname, &disc_number);
    QString  cover_name   = track->coverName();


    //! GENRE part in database
    int id_genre = insertGenre( track->genre );

    //! YEAR part in database
    int id_year = insertYear( track->year );

    //! ARTIST part in database
    int id_artist = insertArtist( track->artist );

    //! ALBUM part in database
    int id_album = insertAlbum(
        track->album,
        id_artist,
        cover_name,
        track->year,
        disc_number
        );

    //! mise à jour du cover
    storeCoverArt(storageLocation + cover_name, track->url);

    if( DatabaseManager::instance()->DB_PARAM().checkCover )
      recupCoverArtFromDir(storageLocation + cover_name, track->url);

    //! TRACK part in database
    QSqlQuery query(*m_sqlDb);
    query.prepare("INSERT INTO `tracks`(`filename`,`trackname`,`number`,`length`,`artist_id`,`album_id`,`year_id`,`genre_id`,`mtime`,`playcount`,`rating`,`albumgain`,`albumpeakgain`,`trackgain`,`trackpeakgain`)" \
                  "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
    query.addBindValue(fname);
    query.addBindValue(track->title);
    query.addBindValue(track->num);
    query.addBindValue(track->duration);

    query.addBindValue(id_artist);
    query.addBindValue(id_album);
    query.addBindValue(id_year);
    query.addBindValue(id_genre);

    query.addBindValue(mtime);
    query.addBindValue(track->playcount);
    query.addBindValue(track->rating);
    query.addBindValue(track->albumGain);
    query.addBindValue(track->albumPeak);
    query.addBindValue(track->trackGain);
    query.addBindValue(track->trackPeak);
    query.exec();

    // delete media from memory
    if(track)
      track.reset();
}

/*******************************************************************************
   DataBaseBuilder::insertGenre
*******************************************************************************/
int DataBaseBuilder::insertGenre(const QString & genre)
{
    QSqlQuery q("", *m_sqlDb);
    q.prepare("SELECT `id` FROM `genres` WHERE `genre`=:val;");
    q.bindValue(":val", genre );
    q.exec();

    if ( !q.next() ) {
      q.prepare("INSERT INTO `genres`(`genre`) VALUES (:val);");
      q.bindValue(":val", genre);
      q.exec();

      if(q.numRowsAffected() < 1) return -1;
      q.prepare("SELECT `id` FROM `genres` WHERE `genre`=:val;");
      q.bindValue(":val", genre);
      q.exec();
      q.next();
    }
    return q.value(0).toString().toInt();
}

/*******************************************************************************
   DataBaseBuilder::insertYear
*******************************************************************************/
int DataBaseBuilder::insertYear(int year)
{
    QSqlQuery q("", *m_sqlDb);
    q.prepare("SELECT `id` FROM `years` WHERE `year`=:val;");
    q.bindValue(":val", year );
    q.exec();

    if ( !q.next() ) {
      q.prepare("INSERT INTO `years`(`year`) VALUES (:val);");
      q.bindValue(":val", year);
      q.exec();

      if(q.numRowsAffected() < 1) return -1;
      q.prepare("SELECT `id` FROM `years` WHERE `year`=:val;");
      q.bindValue(":val", year);
      q.exec();
      q.next();
    }
    return q.value(0).toString().toInt();
}

/*******************************************************************************
   DataBaseBuilder::insertArtist
*******************************************************************************/
int DataBaseBuilder::insertArtist(const QString & artist)
{
    QSqlQuery q("", *m_sqlDb);
    q.prepare("SELECT `id` FROM `artists` WHERE `name`=:val;");
    q.bindValue(":val", artist );
    q.exec();

    if ( !q.next() ) {
      q.prepare("INSERT INTO `artists`(`name`,`favorite`,`playcount`,`rating`) VALUES (:val,0,0,-1);");
      q.bindValue(":val", artist);
      q.exec();

      if(q.numRowsAffected() < 1) return -1;
      q.prepare("SELECT `id` FROM `artists` WHERE `name`=:val;");
      q.bindValue(":val", artist);
      q.exec();
      q.next();
    }
    return q.value(0).toString().toInt();
}

/*******************************************************************************
   DataBaseBuilder::insertAlbum
*******************************************************************************/
int DataBaseBuilder::insertAlbum(const QString & album, int artist_id,const QString & cover,int year,int disc)
{
    QSqlQuery q("", *m_sqlDb);
    q.prepare("SELECT `id` FROM `albums` WHERE `name`=:val AND `artist_id`=:id AND `disc`=:dn;");
    q.bindValue(":val", album );
    q.bindValue(":id", artist_id );
    q.bindValue(":dn", disc );
    q.exec();

    if ( !q.next() ) {
      q.prepare("INSERT INTO `albums`(`name`,`artist_id`,`cover`,`year`,`favorite`,`playcount`,`rating`,`disc`) VALUES (:val,:id,:cov,:y,0,0,-1,:dn);");
      q.bindValue(":val", album);
      q.bindValue(":id", artist_id );
      q.bindValue(":cov", cover );
      q.bindValue(":y", year );
      q.bindValue(":dn", disc );
      q.exec();

      if(q.numRowsAffected() < 1) return -1;
      q.prepare("SELECT `id` FROM `albums` WHERE `name`=:val AND `artist_id`=:id AND `disc`=:dn;");
      q.bindValue(":val", album );
      q.bindValue(":id", artist_id );
      q.bindValue(":dn", disc );
      q.exec();
      q.next();
    }
    return q.value(0).toString().toInt();
}


/*******************************************************************************
   DataBaseBuilder::storeCoverArt
*******************************************************************************/
void DataBaseBuilder::storeCoverArt(const QString& coverFilePath, const QString& trackFilename)
{
    //Debug::debug() << "- DataBaseBuilder -> storeCoverArt " << coverFilePath;

    //! check if cover art already exist
    QFile file(coverFilePath);
    if(file.exists()) return;

    //! get cover image from file
    QImage image = MEDIA::LoadImageFromFile(trackFilename);
    if( !image.isNull() )
      image.save(coverFilePath, "png", -1);
}

/*******************************************************************************
   DataBaseBuilder::recupCoverArtFromDir
*******************************************************************************/
void DataBaseBuilder::recupCoverArtFromDir(const QString& coverFilePath, const QString& trackFilename)
{
    //Debug::debug() << "- DataBaseBuilder -> recupCoverArtFromDir " << coverFilePath;

    //! check if coverArt already exist
    QFile file(coverFilePath);
    if(file.exists()) return ;

    //! search album art into file source directory
    const QStringList imageFilters = QStringList() << "*.jpg" << "*.png";
    QDir sourceDir(QFileInfo(trackFilename).absolutePath());

    sourceDir.setNameFilters(imageFilters);

    QStringList entryList = sourceDir.entryList(imageFilters, QDir::Files, QDir::Size);

    while(!entryList.isEmpty()) {
      //! I take the first one (the biggest one)
      //!WARNING simplification WARNING
      QString file = QFileInfo(trackFilename).absolutePath() + "/" + entryList.takeFirst();
      QImage image = QImage(file);
      //! check if not null image (occur when file is KO)
      if(!image.isNull()) {
        image = image.scaled(QSize(110, 110), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        //! check if save is OK
        if(image.save(coverFilePath, "png", -1))
          break;
      }
    }
}

/*******************************************************************************
   DataBaseBuilder::updateTrack
*******************************************************************************/
void DataBaseBuilder::updateTrack(const QString& filename)
{
    removeTrack(filename);
    insertTrack(filename);
}

/*******************************************************************************
   DataBaseBuilder::removeTrack
*******************************************************************************/
void DataBaseBuilder::removeTrack(const QString& filename)
{
    Debug::debug() << "- DataBaseBuilder -> Deleting track :" << filename;
    QFileInfo fileInfo(filename);
    QString fname = fileInfo.filePath().toUtf8();

    QSqlQuery query(*m_sqlDb);
    query.prepare("DELETE FROM `tracks` WHERE `filename`=?;");
    query.addBindValue(fname);
    query.exec();
}

/*******************************************************************************
   DataBaseBuilder::cleanUpDatabase
*******************************************************************************/
void DataBaseBuilder::cleanUpDatabase()
{
    {
      QSqlQuery query("DELETE FROM `albums` WHERE `id` NOT IN (SELECT `album_id` FROM `tracks` GROUP BY `album_id`);", *m_sqlDb);
    }
    {
      QSqlQuery query("DELETE FROM `genres` WHERE `id` NOT IN (SELECT `genre_id` FROM `tracks` GROUP BY `genre_id`);", *m_sqlDb);
    }
    {
      QSqlQuery query("DELETE FROM `artists` WHERE `id` NOT IN (SELECT `artist_id` FROM `tracks` GROUP BY `artist_id`);", *m_sqlDb);
    }
    {
      QSqlQuery query("DELETE FROM `years` WHERE `id` NOT IN (SELECT `year_id` FROM `tracks` GROUP BY `year_id`);", *m_sqlDb);
    }
    {
      QSqlQuery query("DELETE FROM `playlist_items` WHERE `playlist_id` NOT IN (SELECT `id` FROM `playlists`);", *m_sqlDb);
    }
}

/*******************************************************************************
   DataBaseBuilder::insertPlaylist
*******************************************************************************/
void DataBaseBuilder::insertPlaylist(const QString& filename)
{
    QFileInfo fileInfo(filename);
    QString fname = fileInfo.filePath().toUtf8();
    QString pname = fileInfo.baseName();
    uint mtime    = fileInfo.lastModified().toTime_t();

    Debug::debug() << "- DataBasePlsBuilder -> insert playlist :" << filename;

    int favorite = 0;

    QSqlQuery query(*m_sqlDb);
    query.prepare("INSERT INTO `playlists`(`filename`,`name`,`type`,`favorite`,`mtime`)" \
                  "VALUES(?," \
                  "       ?," \
                  "       ?," \
                  "       ?," \
                  "       ?);");
    query.addBindValue(fname);
    query.addBindValue(pname);
    query.addBindValue((int) T_FILE);
    query.addBindValue(favorite);
    query.addBindValue(mtime);
    query.exec();


    QList<MEDIA::TrackPtr> list =  MEDIA::PlaylistFromFile(filename);
    foreach (MEDIA::TrackPtr mi, list)
    {
      QString url           = mi->url;
      QString name          = QFileInfo(url).baseName();

      //! Playlist Item part in database
      Debug::debug() << "- DataBasePlsBuilder -> insert playlistitem url: " << url;

      query.prepare("INSERT INTO `playlist_items`(`url`,`name`,`playlist_id`)" \
                    "VALUES(?," \
                    "       ?," \
                    "       (SELECT `id` FROM `playlists` WHERE `filename`=?));");
      query.addBindValue(url);
      query.addBindValue(name);
      query.addBindValue(fname);
      query.exec();
    } // end foreach url into playlist
}

/*******************************************************************************
   DataBaseBuilder::updatePlaylist
*******************************************************************************/
void DataBaseBuilder::updatePlaylist(const QString& filename)
{
    removePlaylist(filename);
    insertPlaylist(filename);
}

/*******************************************************************************
   DataBaseBuilder::removePlaylist
*******************************************************************************/
void DataBaseBuilder::removePlaylist(const QString& filename)
{
    Debug::debug() << "- DataBasePlsBuilder -> Deleting playlist :" << filename;
    QFileInfo fileInfo(filename);
    QString fname = fileInfo.filePath().toUtf8();

    QSqlQuery query(*m_sqlDb);
    query.prepare("DELETE FROM `playlists` WHERE `filename`=?;");
    query.addBindValue(fname);
    query.exec();
}


