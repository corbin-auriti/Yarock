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

#include "smartplaylist.h"

#include "core/mediasearch/media_search.h"
#include "core/mediasearch/media_search_engine.h"
#include "core/mediasearch/media_search_dialog.h"
#include "dialog_base.h"

#include "core/database/database.h"
#include "debug.h"


// Qt
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>
#include <QtSql/QSqlResult>

#include <QString>
#include <QDataStream>


/*
********************************************************************************
*                                                                              *
*    Class SmartPlaylist                                                       *
*                                                                              *
********************************************************************************
*/
SmartPlaylist::SmartPlaylist(QObject *parent)  : QObject( parent )
{

}

/*******************************************************************************
 createDatabase
*******************************************************************************/
void SmartPlaylist::createDatabase(QSqlDatabase *sqlDB)
{

const QStringList p_names = QStringList()
            << tr("50 Random tracks")
            << tr("50 last played")
            << tr("50 most played")
            << tr("ever played")
            << tr("never played")
            << tr("top rated tracks");

const QStringList p_icon = QStringList()
            << ":/images/media-smartplaylist1-110x110.png"
            << ":/images/media-smartplaylist2-110x110.png"
            << ":/images/media-smartplaylist4-110x110.png"
            << ":/images/media-smartplaylist4-110x110.png"
            << ":/images/media-smartplaylist4-110x110.png"
            << ":/images/media-smartplaylist3-110x110.png";


  /*
   *  MediaSearch(SearchType                 type,
   *              SearchQueryList            terms,
   *              SortType                   sort_type,
   *              SearchQuery::Search_Field  sort_field,
   *              int limit = 50);
   */

QList<MediaSearch> list_search = QList<MediaSearch>()
  << MediaSearch(MediaSearch::Type_All, SearchQueryList(), MediaSearch::Sort_Random, SearchQuery::field_track_trackname, 50)

  << MediaSearch(MediaSearch::Type_All, SearchQueryList(), MediaSearch::Sort_FieldDesc, SearchQuery::field_track_lastPlayed,  50)

  << MediaSearch(MediaSearch::Type_All, SearchQueryList(), MediaSearch::Sort_FieldDesc, SearchQuery::field_track_playcount,  50)

  << MediaSearch( MediaSearch::Type_And,
                  SearchQueryList() << SearchQuery(SearchQuery::field_track_playcount, SearchQuery::op_GreaterThan, 0),
                  MediaSearch::Sort_Random, SearchQuery::field_track_trackname,  -1)

  << MediaSearch( MediaSearch::Type_And,
                  SearchQueryList() << SearchQuery(SearchQuery::field_track_playcount, SearchQuery::op_Equals, 0),
                  MediaSearch::Sort_Random, SearchQuery::field_track_trackname,  500)

  << MediaSearch( MediaSearch::Type_And,
                  SearchQueryList() << SearchQuery(SearchQuery::field_track_rating, SearchQuery::op_GreaterThan, 0.5),
                  MediaSearch::Sort_FieldDesc, SearchQuery::field_artist_name,  -1);




    QSqlQuery query_1("DELETE FROM `smart_playlists`;", *sqlDB);

    for (int i=0; i < p_names.size(); i++)
    {
      Debug::debug() << "--- SMART_PLAYLIST::createDatabase -> insert smart playlist :" << p_names.at(i);

      QVariant variant_search = MediaSearch::toQVariant( list_search.at(i) );
      Debug::debug() << "--- SMART_PLAYLIST::createDatabase -> insert variant_search :" << variant_search;

      QSqlQuery query(*sqlDB);
      query.prepare("INSERT INTO `smart_playlists`(`name`,`icon`,`rules`,`type`,`favorite`)" \
                      "VALUES(?," \
                      "       ?," \
                      "       ?," \
                      "       ?," \
                      "       ?);");
      query.addBindValue( p_names.at(i) );
      query.addBindValue( p_icon.at(i) );
      query.addBindValue( variant_search );
      query.addBindValue( (int) T_SMART );
      query.addBindValue( 0 );
      Debug::debug() << "query exec " << query.exec();
    }
}



/*******************************************************************************
 SmartPlaylist::mediaItem
*******************************************************************************/
QList<MEDIA::TrackPtr> SmartPlaylist::mediaItem(QVariant search_variant)
{
    //Debug::debug() << "SmartPlaylist--> get mediaitem start : " << QTime::currentTime().toString();

    MediaSearch media_search = MediaSearch::fromQVariant(search_variant);

    SearchEngine* search_engine = new SearchEngine();
    search_engine->init_search_engine(media_search);
    search_engine->doSearch();

    //Debug::debug() << "SmartPlaylist--> get mediaitem end : " << QTime::currentTime().toString();
    return  search_engine->result();
}



/*******************************************************************************
 SmartPlaylist::edit_dialog
*******************************************************************************/
bool SmartPlaylist::edit_dialog(MEDIA::PlaylistPtr smart_playlist)
{
    //Debug::debug() << "SmartPlaylist--> edit_dialog";

    const QVariant search_variant = smart_playlist->rules;
    const int dbId                = smart_playlist->id;
    const QString name            = smart_playlist->name;

    MediaSearch media_search = MediaSearch::fromQVariant(search_variant);

    //! open smart media search dialog
    Media_Search_Dialog* dialog = new Media_Search_Dialog();
    dialog->set_search(media_search);

    dialog->exec(); // blocking call (modal dialog)

    if( dialog->result() == QDialog::Accepted)
    {
      //! get new media search
      MediaSearch new_media_search                = dialog->get_search();

      const QVariant new_v_search = MediaSearch::toQVariant(new_media_search);

      //! set new media search rules values
      smart_playlist->rules = new_v_search;

      //! edit in database
      Database db;
      if (!db.connect()) return false;

      QSqlQuery query("UPDATE `smart_playlists` "       \
                      "SET `rules`=(?) WHERE `id`="+QString::number(dbId)+";",*db.sqlDb());
      query.addBindValue(new_v_search);
      Debug::debug() << "query exec " << query.exec();
      return true;
    }

    return false;
}

/*******************************************************************************
 SmartPlaylist::create_dialog
*******************************************************************************/
bool SmartPlaylist::create_dialog()
{
    QString name;

    //! open smart media search dialog
    Media_Search_Dialog* dialog = new Media_Search_Dialog();

    dialog->exec(); // blocking call (modal dialog)

    if( dialog->result() == QDialog::Accepted)
    {
      //! get new media search
      MediaSearch media_search     = dialog->get_search();
      const QVariant new_v_search  = MediaSearch::toQVariant(media_search);

      //! open dialog for a playlist name
      DialogInput input(0, tr("Save smart playlist") , tr("Playlist name"));
      input.setFixedSize(480,140);
    

      if(input.exec() == QDialog::Accepted) {
        QString name = input.editValue();
         //! database save
         Database db;
         if (!db.connect())
           return false;

         QSqlQuery query(*db.sqlDb());

         query.prepare("INSERT INTO `smart_playlists`(`name`,`icon`,`rules`,`type`,`favorite`)" \
                      "VALUES(?,?,?,?,?);");

         query.addBindValue(name);
         query.addBindValue(":/images/media-smartplaylist4-110x110.png");
         query.addBindValue(new_v_search);
         query.addBindValue((int) T_SMART);
         query.addBindValue(0);
         Debug::debug() << "query exec " << query.exec();

         return true;
      }
    }

    return false;
}
