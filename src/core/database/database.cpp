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

#include "database.h"
#include "databasemanager.h"
#include "smartplaylist.h"
#include "debug.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QFile>
#include <QThread>
#include <QMap>

static QMap<QString, QSqlDatabase *> map_sqldb;

/*
********************************************************************************
*                                                                              *
*    Class Database                                                            *
*                                                                              *
********************************************************************************
*/
Database::Database()
{
    m_name           = DatabaseManager::instance()->DB_NAME;
    m_database_path  = DatabaseManager::instance()->DB_FILE();
    m_revision       = DatabaseManager::instance()->DB_REVISION();
}


Database::~Database()
{
    //Debug::debug() << "[Database] delete";
}

void Database::close()
{
    // USE Database::close static fonction before switching collection database
    Debug::debug() << "[Database] closing";
    foreach(QString connection, map_sqldb.keys())
    {
        //Debug::debug() << "- Database -> closing connection :" << connection;
        QSqlDatabase::database( connection ).close();
        delete map_sqldb.take(connection);
        QSqlDatabase::removeDatabase(connection);
    }
    Debug::debug() << "[Database] closing OK";
}

QSqlDatabase* Database::sqlDb()
{
    return map_sqldb[m_connection_name];
}



/*******************************************************************************
    database connection
*******************************************************************************/
bool Database::connect(bool create)
{
    m_connection_name =    QString("%1").arg(
      reinterpret_cast<quint64>(QThread::currentThread())
    );
    //Debug::debug() << "[Database] connect : " << m_connection_name;

    if (QSqlDatabase::contains(m_connection_name)) {
        //Debug::debug() << "- Database -> Connection with name " << m_connection_name << " exists";
        return true;
    }

    if(!create && !QFile::exists(m_database_path))
      return false;


    //! Sql Database creation
    Debug::debug() << "[Database] create new db";
    QSqlDatabase* sqldb = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE",m_connection_name));
    map_sqldb[m_connection_name] = sqldb;
    sqldb->setDatabaseName( m_database_path );

    if (!sqldb->open()) {
        Debug::warning() << "[Database] Failed to establish " << sqldb->connectionName() << " connection to database!";
        Debug::warning() << "[Database] Reason: " << sqldb->lastError().text();
        return false;
    }

    QSqlQuery query(*sqldb);
    query.exec("PRAGMA synchronous = OFF");
    query.exec("PRAGMA journal_mode = MEMORY");
    query.exec("PRAGMA auto_vacuum = FULL");

    //Debug::debug() << "- Database -> create db OK";
    return true;
}

/*******************************************************************************
    database creation
*******************************************************************************/
void Database::create()
{
    Debug::debug() << "[Database] Initializing database structure";

    //QSqlQuery query(*_sqlDb);
    QSqlQuery query(*map_sqldb[m_connection_name]);

    Debug::debug() << query.exec("CREATE TABLE `genres` ("              \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                 "    `genre` TEXT NOT NULL);");

    Debug::debug() << query.exec("CREATE TABLE `artists` ("             \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                 "    `name` TEXT NOT NULL,"                            \
                 "    `favorite` INTEGER DEFAULT 0, "                   \
                 "    `playcount` INTEGER DEFAULT 0, "                  \
                 "    `rating` INTEGER);");

   Debug::debug() << query.exec("CREATE TABLE `albums` ("               \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                 "    `name` TEXT NOT NULL,"                            \
                 "    `artist_id` INTEGER NOT NULL,"                    \
                 "    `cover` TEXT,"                                    \
                 "    `year` INTEGER NOT NULL,"                         \
                 "    `disc` INTEGER DEFAULT 0, "                       \
                 "    `favorite` INTEGER DEFAULT 0, "                   \
                 "    `playcount` INTEGER DEFAULT 0, "                  \
                 "    `rating` INTEGER);");

   Debug::debug() << query.exec("CREATE TABLE `tracks` ("               \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                 "    `filename` TEXT NOT NULL,"                        \
                 "    `trackname` TEXT NOT NULL,"                       \
                 "    `number` INTEGER,"                                \
                 "    `length` INTEGER NULL,"                           \
                 "    `artist_id` INTEGER NOT NULL,"                    \
                 "    `album_id` INTEGER NOT NULL,"                     \
                 "    `genre_id` INTEGER NOT NULL,"                     \
                 "    `year_id` INTEGER NOT NULL,"                      \
                 "    `mtime` INTEGER, "                                \
                 "    `playcount` INTEGER DEFAULT 0,"                   \
                 "    `rating`  INTEGER DEFAULT 0,"                     \
                 "    `albumgain` REAL NULL,"                           \
                 "    `albumpeakgain` REAL NULL,"                       \
                 "    `trackgain` REAL NULL,"                           \
                 "    `trackpeakgain` REAL NULL);");

    Debug::debug() << query.exec("CREATE TABLE `years` (" \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                 "    `year` INTEGER NOT NULL);");

    Debug::debug() << query.exec("CREATE TABLE `db_attribute` (" \
                 "    `name` VARCHAR(255)," \
                 "    `value` TEXT);");

    Debug::debug() << "[Database] insert database revision";

    Debug::debug() << query.exec("INSERT INTO db_attribute (name, value) values ('version', "+QString::number(m_revision)+");");
    Debug::debug() << query.exec("INSERT INTO db_attribute (name, value) values ('lastUpdate', 0);");


    //! HISTO
    query.exec("CREATE TABLE `histo` ("                                 \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                 "    `url` TEXT NOT NULL,"                             \
                 "    `name` TEXT NOT NULL,"                            \
                 "    `date` INTEGER);");


     Debug::debug() << query.exec("CREATE VIEW `view_histo` AS"              \
                 "    SELECT `histo`.`id`,"                                  \
                 "           `histo`.`url`,"                                 \
                 "           `histo`.`name`,"                                \
                 "           `histo`.`date`,"                                \
                 "           `tracks`.`id` AS `track_id`,"                   \
                 "           `tracks`.`trackname`,"                          \
                 "           `tracks`.`number`,"                             \
                 "           `tracks`.`length`,"                             \
                 "           `tracks`.`playcount`,"                          \
                 "           `tracks`.`albumgain`,"                          \
                 "           `tracks`.`albumpeakgain`,"                      \
                 "           `tracks`.`trackgain`,"                          \
                 "           `tracks`.`trackpeakgain`,"                      \
                 "           `artists`.`name` AS `artist_name`,"             \
                 "           `albums`.`name` AS `album_name`,"               \
                 "           `genres`.`genre` AS `genre_name`,"              \
                 "           `years`.`year`"                                 \
                 "    FROM `histo`"                                          \
                 "    LEFT OUTER JOIN `tracks` ON `histo`.`url` = `tracks`.`filename`" \
                 "    LEFT JOIN `artists` ON `tracks`.`artist_id` = `artists`.`id`" \
                 "    LEFT JOIN `albums` ON `tracks`.`album_id` = `albums`.`id`" \
                 "    LEFT JOIN `genres` ON `tracks`.`genre_id` = `genres`.`id`" \
                 "    LEFT JOIN `years` ON `tracks`.`year_id` = `years`.`id`;");

    //! VIEW_TRACKS
    Debug::debug() << query.exec("CREATE VIEW `view_tracks` AS"                \
                 "    SELECT `tracks`.`id`,"                                   \
                 "           `tracks`.`filename`,"                             \
                 "           `tracks`.`trackname`,"                            \
                 "           `tracks`.`number`,"                               \
                 "           `tracks`.`length`,"                               \
                 "           `tracks`.`playcount`,"                            \
                 "           `tracks`.`rating`,"                               \
                 "           `tracks`.`artist_id`,"                            \
                 "           `tracks`.`album_id`,"                             \
                 "           `tracks`.`genre_id`,"                             \
                 "           `tracks`.`albumgain`,"                            \
                 "           `tracks`.`albumpeakgain`,"                        \
                 "           `tracks`.`trackgain`,"                            \
                 "           `tracks`.`trackpeakgain`,"                        \
                 "           `artists`.`name` AS `artist_name`,"               \
                 "           `artists`.`favorite` AS `artist_favorite`,"       \
                 "           `artists`.`playcount` AS `artist_playcount`,"     \
                 "           `artists`.`rating` AS `artist_rating`,"           \
                 "           `albums`.`name` AS `album_name`,"                 \
                 "           `albums`.`year` AS `album_year`,"                 \
                 "           `albums`.`cover` AS `album_cover`,"               \
                 "           `albums`.`favorite` AS `album_favorite`,"         \
                 "           `albums`.`playcount` AS `album_playcount`,"       \
                 "           `albums`.`rating` AS `album_rating`,"             \
                 "           `albums`.`disc` AS `album_disc`,"                 \
                 "           `genres`.`genre` AS `genre_name`,"                \
                 "           `years`.`year`,"                                  \
                 "           `histo`.`date` AS `last_played`"                  \
                 "    FROM `tracks`"                                           \
                 "    LEFT JOIN `artists` ON `tracks`.`artist_id` = `artists`.`id`" \
                 "    LEFT JOIN `albums` ON `tracks`.`album_id` = `albums`.`id`"    \
                 "    LEFT JOIN `genres` ON `tracks`.`genre_id` = `genres`.`id`"    \
                 "    LEFT JOIN `years` ON `tracks`.`year_id` = `years`.`id`"       \
                 "    LEFT JOIN `histo` ON `tracks`.`filename` = `histo`.`url`;");


     Debug::debug() << query.exec("CREATE VIEW `view_albums` AS"               \
                 "    SELECT `albums`.`id`,"                                   \
                 "           `albums`.`name`,"                                 \
                 "           `albums`.`year`,"                                 \
                 "           `albums`.`cover`,"                                \
                 "           `albums`.`favorite`,"                             \
                 "           `albums`.`playcount`,"                            \
                 "           `artists`.`name` AS `artist_name`,"               \
                 "           `artists`.`favorite` AS `artist_favorite`,"       \
                 "           `artists`.`playcount` AS `artist_playcount`"      \
                 "    FROM `albums`"                                           \
                 "    LEFT JOIN `artists` ON `albums`.`artist_id` = `artists`.`id`");



    //! playlist part
    // type playlist (0 = T_DATABASE, 1 = T_FILE, 2 = T_SMART)
    query.exec("CREATE TABLE `playlists` ("                                  \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"      \
                 "    `filename` TEXT NOT NULL,"                             \
                 "    `name` TEXT NOT NULL,"                                 \
                 "    `type` INTEGER,"                                       \
                 "    `favorite` INTEGER,"                                   \
                 "    `mtime` INTEGER);");

    query.exec("CREATE TABLE `playlist_items` ("                             \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"      \
                 "    `url` TEXT NOT NULL,"                                  \
                 "    `name` TEXT NOT NULL,"                                 \
                 "    `playlist_id` INTEGER NOT NULL);");

     Debug::debug() << query.exec("CREATE VIEW `view_playlists` AS"          \
                 "    SELECT `playlist_items`.`url`,"                        \
                 "           `playlist_items`.`name`,"                       \
                 "           `playlists`.`id` AS `playlist_id`,"             \
                 "           `playlists`.`filename` AS `playlist_filename`," \
                 "           `playlists`.`name` AS `playlist_name`,"         \
                 "           `playlists`.`type` AS `playlist_type`,"         \
                 "           `playlists`.`favorite` AS `playlist_favorite`," \
                 "           `playlists`.`mtime` AS `playlist_mtime`,"       \
                 "           `tracks`.`id` AS `track_id`"                    \
                 "    FROM `playlist_items`"                                 \
                 "    LEFT JOIN `playlists` ON `playlist_items`.`playlist_id` = `playlists`.`id`" \
                 "    LEFT OUTER JOIN `tracks` ON `playlist_items`.`url` = `tracks`.`filename`;");

    // type playlist (0 = T_DATABASE, 1 = T_FILE, 2 = T_SMART)
    query.exec("CREATE TABLE `smart_playlists` ("                           \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"     \
                 "    `name` TEXT NOT NULL,"                                \
                 "    `icon` TEXT NOT NULL,"                                \
                 "    `rules` TEXT NOT NULL,"                               \
                 "    `type` INTEGER,"                                      \
                 "    `favorite` INTEGER);");

    //! Smart Playlist
    SmartPlaylist::createDatabase(map_sqldb[m_connection_name]);
}

