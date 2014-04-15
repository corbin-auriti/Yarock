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

#include "playlistwriter.h"
#include "playqueue_model.h"

#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/playlist_parser.h"
#include "core/database/databasemanager.h"
#include "core/database/database.h"

#include "debug.h"

//Qt
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>
#include <QtSql/QSqlResult>

#include <QFileInfo>
#include <QStringList>
#include <QDateTime>

/*
********************************************************************************
*                                                                              *
*    Class PlaylistWriter                                                      *
*                                                                              *
********************************************************************************
*/
PlaylistWriter::PlaylistWriter()
{
    m_fileToSave    = QString();
    m_isRunning     = false;
}

//! ----------------------- PlaylistWriter::run --------------------------------
void PlaylistWriter::run()
{
    if (m_fileToSave.isEmpty()) return;
    m_isRunning     = true;
    
    if(MEDIA::isPlaylistFile(m_fileToSave)) 
    {
        //! save to file
        MEDIA::PlaylistToFile( m_fileToSave, m_model->tracks() );

        //! check if we need to update database
        QString path = QFileInfo(m_fileToSave).absolutePath();
        QStringList listDir = QStringList() << DatabaseManager::instance()->DB_PARAM().sourcePathList;

        bool dbChange = false;
        foreach(const QString& s, listDir) {
          if(s.contains(path)) {
            dbChange = true;
            break;
          }
        }

        //! playlist is in collection directory --> add or update into database
        if(dbChange) {
          updateDatabase(m_model->tracks());
          emit playlistSaved();
        }
    } // fin Media is playlist

    m_isRunning     = false;
}


//! ----------------------- PlaylistWriter::saveToFile -------------------------
void PlaylistWriter::saveToFile(const QString& filename)
{
    m_fileToSave = filename;
}


//! ----------------------- PlaylistWriter::updateDatabase ---------------------
void PlaylistWriter::updateDatabase(QList<MEDIA::TrackPtr> list)
{
    Database db;
    if (!db.connect()) return;

    QFileInfo fileInfo(m_fileToSave);
    QString fname = fileInfo.filePath().toUtf8();
    QString pname = fileInfo.baseName();
    uint mtime    = fileInfo.lastModified().toTime_t();

    //! delete playlist and playlist items
    QSqlField data("col",QVariant::String);
    data.setValue(fname);
    QString fnameDb = db.sqlDb()->driver()->formatValue(data,false);

    {
    QSqlQuery cleanQuery1("DELETE FROM `playlists` WHERE `filename`="+fnameDb+";", *db.sqlDb());
    QSqlQuery cleanQuery2("DELETE FROM `playlist_items` WHERE `playlist_id` NOT IN (SELECT `id` FROM `playlists`);", *db.sqlDb());
    }

    //! add playlist
    Debug::debug() << "    [PlaylistWriter] insert playlist into db" << m_fileToSave;

    int favorite = 0;

    /*-----------------------------------------------------------*/
    /* PLAYLIST part in database                                 */
    /* ----------------------------------------------------------*/ 
    {
        QSqlQuery playlistQuery(*db.sqlDb());
        playlistQuery.prepare("INSERT INTO `playlists`(`filename`,`name`,`type`,`favorite`, `mtime`)" \
                      "VALUES(?," \
                      "       ?," \
                      "       ?," \
                      "       ?," \
                      "       ?);");
        playlistQuery.addBindValue(fname);
        playlistQuery.addBindValue(pname);
        playlistQuery.addBindValue((int) T_FILE);
        playlistQuery.addBindValue(favorite);
        playlistQuery.addBindValue(mtime);
        playlistQuery.exec();
    }

    /*-----------------------------------------------------------*/
    /* PLAYLIST ITEMS part in database                           */
    /* ----------------------------------------------------------*/
    foreach(MEDIA::TrackPtr track, list)
    {
      QString url       = track->url;
      QString name      = QFileInfo(url).baseName();

      //! Playlist Item part in database
      QSqlQuery itemQuery(*db.sqlDb());
      Debug::debug() << "--- PlaylistWriter -> insert playlistitem url:" << url;

      itemQuery.prepare("INSERT INTO `playlist_items`(`url`,`name`,`playlist_id`)" \
                        "VALUES(?," \
                        "       ?," \
                        "       (SELECT `id` FROM `playlists` WHERE `filename`="+fnameDb+"));");
      itemQuery.addBindValue(url);
      itemQuery.addBindValue(name);
      itemQuery.exec();
    }
}
