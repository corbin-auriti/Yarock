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

#include "playlistdbwriter.h"
#include "playqueue_model.h"

#include "core/database/database.h"
#include "core/mediaitem/mediaitem.h"
#include "debug.h"

//Qt
#include <QStringList>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>
#include <QtSql/QSqlResult>

#include <QDateTime>
#include <QFileInfo>
#include <QCryptographicHash>

/*
********************************************************************************
*                                                                              *
*    Class PlaylistDbWriter                                                    *
*                                                                              *
********************************************************************************
*/
PlaylistDbWriter::PlaylistDbWriter()
{
    _isRunning     = false;
    _playlist_name = QString();
    _database_id   = -1;
}


//! ----------------------- PlaylistDbWriter::saveToDatabase -------------------
void PlaylistDbWriter::saveToDatabase(const QString& playlist_name, int bd_id)
{
    _playlist_name = playlist_name;
    _database_id   = bd_id;
}


//! ----------------------- run ------------------------------------------------
void PlaylistDbWriter::run()
{
    _isRunning     = true;
    
    //! playlist data for Database
    QDateTime date   = QDateTime::currentDateTime();
    uint mtime       = date.toTime_t();
    QString pname    = "playlist-" + date.toString("dd-MM-yyyy-hh:mm:ss");
    QString fname    = QString(QCryptographicHash::hash(pname.toUtf8().constData(), QCryptographicHash::Sha1).toHex());
    int favorite     = 0;

    //! set real playlist name from user
    if(!_playlist_name.isEmpty()) {
      pname = _playlist_name;
    }


    QSqlField data("col",QVariant::String);

    Database db;
    if (!db.connect())
      return;

    if(_database_id != -1)
    {
      QSqlQuery cleanQuery1("DELETE FROM `playlists` WHERE `id`="+QString::number(_database_id)+";", *db.sqlDb());
      QSqlQuery cleanQuery2("DELETE FROM `playlist_items` WHERE `playlist_id` NOT IN (SELECT `id` FROM `playlists`);", *db.sqlDb());
    }


    Debug::debug() << "    [PlaylistDbWriter] insert playlist: " << pname;

    /*-----------------------------------------------------------*/
    /* PLAYLIST part in database                                 */
    /* ----------------------------------------------------------*/    
    {
      QSqlQuery query(*db.sqlDb());
      query.prepare("INSERT INTO `playlists`(`filename`,`name`,`type`,`favorite`,`mtime`)" \
                    "VALUES(?," \
                    "       ?," \
                    "       ?," \
                    "       ?," \
                    "       ?);");
      query.addBindValue(fname);
      query.addBindValue(pname);
      query.addBindValue((int) T_DATABASE);
      query.addBindValue(favorite);
      query.addBindValue(mtime);
      query.exec();
    }

    
    /*-----------------------------------------------------------*/
    /* PLAYLIST ITEMS part in database                           */
    /* ----------------------------------------------------------*/
    data.setValue(fname);
    QString fnameDB = db.sqlDb()->driver()->formatValue(data,false);

    for (int i = 0; i < m_model->rowCount(QModelIndex()); i++) 
    {

        const QString item_url   = m_model->trackAt(i)->url;
        const QString item_name  = MEDIA::isLocal(m_model->trackAt(i)->url) ? m_model->trackAt(i)->title : m_model->trackAt(i)->name;
       
        QSqlQuery itemQuery(*db.sqlDb());
//         Debug::debug() << "    [PlaylistDbWriter] insert playlist item url: " << item_url;
//         Debug::debug() << "    [PlaylistDbWriter] insert playlist item name: " << m_model->trackAt(i)->name;
//         Debug::debug() << "    [PlaylistDbWriter] insert playlist item title: " << m_model->trackAt(i)->title;

        itemQuery.prepare("INSERT INTO `playlist_items`(`url`,`name`,`playlist_id`)" \
                          "VALUES(?," \
                          "       ?," \
                          "       (SELECT `id` FROM `playlists` WHERE `filename`="+fnameDB+"));");
        itemQuery.addBindValue(item_url);
        itemQuery.addBindValue(item_name);
        itemQuery.exec();
    }

    _isRunning     = false;
    _playlist_name = QString();

    emit playlistSaved();
}
