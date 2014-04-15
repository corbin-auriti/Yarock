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

#include "views/item_button.h"
#include "views/item_common.h"
#include "views/local/local_item.h"

#include "models/local/local_track_model.h"
#include "models/local/local_playlist_model.h"
#include "models/local/histo_model.h"
#include "playqueue/playqueue_model.h"
#include "playqueue/virtual_playqueue.h"

#include "core/mediaitem/mediaitem.h"
#include "covers/covercache.h"

#include "core/database/database.h"
#include "core/database/databasemanager.h"
#include "core/history/histomanager.h"

#include "widgets/dialogs/mediaitem_edit_dialog.h"
#include "smartplaylist/smartplaylist.h"
#include "playqueue/playlisteditor.h"

#include "threadmanager.h"
#include "settings.h"
#include "utilities.h"
#include "global_actions.h"
#include "debug.h"
#include "filedialog.h"

#include <QSqlQuery>
#include <QtGui>

/*
********************************************************************************
*                                                                              *
*    Class LocalScene                                                          *
*                                                                              *
********************************************************************************
*/
LocalScene::LocalScene(QWidget *parent) : SceneBase(parent)
{
    //! set model
    m_localTrackModel     = LocalTrackModel::instance();
    m_localPlaylistModel  = LocalPlaylistModel::instance();
    m_histoModel          = HistoModel::instance();

    connect(VirtualPlayqueue::instance(), SIGNAL(signal_playing_status_change()),  this, SLOT(update()));
}

/*******************************************************************************
    initScene
*******************************************************************************/
void LocalScene::initScene()
{
    m_infosize          = 0;
    m_mouseGrabbedItem  = 0;
    item_count          = 0;
    
    //! graphic item context menu
    m_graphic_item_menu = new GraphicsItemMenu(0);
    m_graphic_item_menu->setBrowserView(this->parentView());
    QObject::connect(m_graphic_item_menu, SIGNAL(menu_action_triggered(ENUM_ACTION_ITEM_MENU)), this, SLOT(slot_contextmenu_triggered(ENUM_ACTION_ITEM_MENU)), Qt::DirectConnection);

    /*  scene actions */
    m_actions.insert("album_grid", new QAction(QIcon(),QString(tr("view grid")),this));
    m_actions.insert("playlist_tracks", new QAction(QIcon(),QString(tr("view by tracks")),this));
    m_actions.insert("new_playlist",new QAction(QIcon(":/images/add_32x32.png"),QString(tr("new playlist")),this));
    m_actions.insert("new_smart",new QAction(QIcon(":/images/add_32x32.png"),QString(tr("new smart playlist")),this));
    m_actions.insert("cover", ACTIONS()->value(TASK_COVER_SEARCH));
    m_actions.insert("reload_histo", new QAction(QIcon(":/images/rebuild.png"),QString(tr("reload history")),this));
    m_actions.insert("clear_histo", new QAction(QIcon(),QString(tr("clear history")),this));
    
    m_actions.value("album_grid")->setCheckable(true);
    m_actions.value("album_grid")->setChecked( SETTINGS()->_album_view_type == 0);
    
    m_actions.value("playlist_tracks")->setCheckable(true);
    m_actions.value("playlist_tracks")->setChecked(SETTINGS()->_playlist_view_type ==1);

    connect(m_actions.value("album_grid"), SIGNAL(triggered()), this, SLOT(slot_change_view_settings()));
    connect(m_actions.value("playlist_tracks"), SIGNAL(triggered()), this, SLOT(slot_change_view_settings()));
    connect(m_actions.value("new_playlist"), SIGNAL(triggered()), this, SLOT(slot_create_new_playlist()));
    connect(m_actions.value("new_smart"), SIGNAL(triggered()), this, SLOT(slot_create_new_smart_playlist()));
    connect(m_actions.value("reload_histo"), SIGNAL(triggered()), this, SLOT(populateScene()));
    connect(m_actions.value("clear_histo"), SIGNAL(triggered()), this, SLOT(slot_clear_history()));
        
    //! gestion du double click par action globale et data
    ACTIONS()->insert(BROWSER_ITEM_RATING_CLICK, new QAction(this));
    ACTIONS()->insert(BROWSER_LOCAL_ITEM_MOUSE_MOVE, new QAction(this));
    
    connect(ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE), SIGNAL(triggered()), this, SLOT(slot_item_mouseMove()), Qt::DirectConnection);
    connect(ACTIONS()->value(BROWSER_ITEM_RATING_CLICK), SIGNAL(triggered()), this, SLOT(slot_item_ratingclick()), Qt::DirectConnection);
    
    setInit(true);
}


/*******************************************************************************
    actions
*******************************************************************************/
QList<QAction *> LocalScene::actions() 
{
    QList<QAction*> list;
   
    switch(this->mode())
    {
      case VIEW::ViewAlbum:
        list << m_actions.value("album_grid");
      case VIEW::ViewArtist:
      case VIEW::ViewGenre:
      case VIEW::ViewYear:
      case VIEW::ViewTrack:
      case VIEW::ViewDashBoard:
        list << m_actions.value("cover");
        break;

      case VIEW::ViewHistory:
        list << m_actions.value("reload_histo");
        list << m_actions.value("clear_histo");
      break;
      
      case VIEW::ViewPlaylist:
        list << m_actions.value("playlist_tracks");
        list << m_actions.value("new_playlist");
        break;
      case VIEW::ViewSmartPlaylist:
        list << m_actions.value("new_smart");
        break;
      default:break;
    }
  
    return list;
}

    

/*******************************************************************************
     setFilter
*******************************************************************************/
void LocalScene::setFilter(const QString& filter)
{
    m_localTrackModel->setFilter(filter);
    m_localPlaylistModel->setFilter(filter);
    m_histoModel->setFilter(filter);
}

/*******************************************************************************
     resizeScene
*******************************************************************************/
void LocalScene::resizeScene()
{
    //Debug::debug() << "   [LocalScene] resizeScene";   
    int new_item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;

    if(item_count != new_item_count)  
    {
      populateScene();
    }
    else
    {
      update();
    }
}

/*******************************************************************************
     slot_change_view_settings
*******************************************************************************/
void LocalScene::slot_change_view_settings()
{
    SETTINGS()->_album_view_type    = m_actions.value("album_grid")->isChecked() ? 0 : 1;
    SETTINGS()->_playlist_view_type = m_actions.value("playlist_tracks")->isChecked() ? 1 : 0;
    
    populateScene();
}

/*******************************************************************************
    populateScene
*******************************************************************************/
void LocalScene::populateScene()
{
    Debug::debug() << "   [LocalScene] PopulateScene";

    //! clear scene and delete all items
    clear();
    
    /* si model est vide et database est en cours de construction */
    if(m_localTrackModel->isEmpty() && ThreadManager::instance()->isDbRunning())
    {
      populateLocalSceneBuilding();
    }    
    else 
    {
      switch(mode())
      {
        case VIEW::ViewAlbum          : populateAlbumScene();    break;
        case VIEW::ViewArtist         : populateArtistScene();   break;
        case VIEW::ViewTrack          : populateTrackScene();    break;
        case VIEW::ViewGenre          : populateGenreScene();    break;
        case VIEW::ViewYear           : populateYearScene();     break;
        case VIEW::ViewFavorite       : populateFavoriteScene(); break;
        case VIEW::ViewPlaylist       : populatePlaylistScene(); break;
        case VIEW::ViewSmartPlaylist  : populatePlaylistSmartScene(); break;
        case VIEW::ViewHistory        : populateHistoScene();     break;
        case VIEW::ViewDashBoard      : populateDashBoardScene(); break;
        default: break;
      }
    }

    //! we need to ajust SceneRect
    setSceneRect ( itemsBoundingRect().adjusted(0, -10, 0, 40) );
}


void LocalScene::populateLocalSceneBuilding()
{
    //! header item
    HeaderItem* header = new HeaderItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    //header->setText( tr("Local media") );
    header->setPos(0, 5);
    addItem(header);    

    LoadingGraphicItem *info = new LoadingGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    info->_text = tr("Updating music database");
    info->setPos( 0 , 70);
    addItem(info);
}



void LocalScene::populateAlbumScene()
{
    if( VIEW::ViewAlbum_Type(SETTINGS()->_album_view_type) == VIEW::album_grid)
      populateAlbumGridScene();
    else
      populateAlbumExtendedScene();
}

void LocalScene::populatePlaylistScene()
{
    if( VIEW::ViewPlaylist_Type(SETTINGS()->_playlist_view_type) == VIEW::playlist_overview)
      populatePlaylistOverviewScene();
    else
      populatePlaylistByTrackScene();
}

void LocalScene::populateAlbumExtendedScene()
{
    int albumRow  = 0;
    int artistRow = 0;
    int Column    = 0;
    m_infosize    = 0;

    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;

    //! header item
    HeaderItem* header = new HeaderItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    header->setText( tr("Albums") );
    header->setPos(0, 5);
    addItem(header);    
    
    artistRow++;

    //! artist loop
    for ( int i = 0; i < m_localTrackModel->rootItem()->childCount(); i++ )
    {
      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast( m_localTrackModel->rootItem()->child(i) );

      if(!m_localTrackModel->isArtistFiltered(artist) ) continue;

      CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      category->m_name = artist->name;
      category->setPos( 0 ,10 + artistRow*50 + albumRow*170);

      addItem(category);

      Column = 0;
      artistRow++;

      //! album loop
      for (int j = 0; j < artist->childCount(); j++)
      {
        MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast( artist->child(j) );
        if(!m_localTrackModel->isAlbumFiltered(album) ) continue;

        m_infosize++;

        AlbumGraphicItem_v2 *album_item = new AlbumGraphicItem_v2();
        album_item->media = album;
        album_item->setPos(4+160*Column, artistRow*50 + albumRow*170);
        addItem(album_item);


        if(Column < (item_count-1)) {
          Column++;
        }
        else {
          Column = 0;
          albumRow++;
        }
      }
      if(Column>0) albumRow++;
    }

    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + artistRow*50 + albumRow*20);
      addItem(info);
    }
}

void LocalScene::populateAlbumGridScene()
{
    int albumRow      = 0;
    int Column        = 0;
    m_infosize        = 0;
    int categorieRow  = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;

    //! header item
    HeaderItem* header = new HeaderItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    header->setText( tr("Albums") );
    header->setPos(0, 5);
    addItem(header);    
    
    categorieRow++;

    CategorieGraphicItem *cat = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    cat->m_name = tr("All Albums");
    cat->setPos( 0 ,10 + categorieRow*50);
    categorieRow++;
    addItem(cat);


    //! artist loop
    for ( int i = 0; i < m_localTrackModel->rootItem()->childCount(); i++ )
    {
      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(m_localTrackModel->rootItem()->child(i));

      if(!m_localTrackModel->isArtistFiltered(artist) ) continue;

      //! album loop
      for (int j = 0; j < artist->childCount(); j++)
      {
        MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast( artist->child(j) );
        if(!m_localTrackModel->isAlbumFiltered(album) ) continue;

        m_infosize++;

        AlbumGraphicItem *album_item = new AlbumGraphicItem();
        album_item->media = album;
        album_item->setPos(4+160*Column, 10 + albumRow*170 + categorieRow*50);
        addItem(album_item);

        if(Column < (item_count-1)) {
          Column++;
        }
        else {
          Column = 0;
          albumRow++;
        }
      }
    }

    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 ,10 + categorieRow*50);
      addItem(info);
    }
}



void LocalScene::populateArtistScene()
{
    int artistRow    = 0;
    int categorieRow = 0;
    int Column       = 0;
    m_infosize       = 0;
    int idx          = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;

    //! header item
    HeaderItem* header = new HeaderItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    header->setText( tr("Artists") );
    header->setPos(0, 5);
    addItem(header);    

    categorieRow++;
        
    //! artist loop
    QChar start_char;
    for (int i=0 ; i < m_localTrackModel->rootItem()->childCount(); i++ )
    {
        MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(m_localTrackModel->rootItem()->child(i));
        if(! m_localTrackModel->isArtistFiltered(artist) ) continue;

        QChar current_char = artist->name.at(0).toLower();

        if(start_char !=  current_char) {
          //! new category
          start_char = current_char;
          if(idx>0) artistRow++;

          CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
          category->m_name = QString(start_char);
          category->setPos( 0 , 10 + categorieRow*50 + artistRow*165);
          addItem(category);

          Column = 0;
          categorieRow++;
        }

        //! add new artist item
        m_infosize++;
        idx++;

        //! New Artist Item
        ArtistGraphicItem *artist_item = new ArtistGraphicItem();
        artist_item->media = artist;
        artist_item->setPos( 4 + 160*Column , categorieRow*50 + artistRow*165);
        addItem(artist_item);

        //! ALBUM COVER LOOP
        artist->album_covers.clear();
        for(int j = artist->childCount()-1 ; j >= 0; j--) {
          MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(artist->child(j));
          if(!m_localTrackModel->isAlbumFiltered(album) ) continue;

          artist->album_covers.prepend(album);

          //!WARNING limite de l'affichage à 6 cover max
          if(artist->album_covers.size() >=6) break;
        }

        if(Column < (item_count-1)) {
          Column++;
        }
        else {
          Column = 0;
          artistRow++;
          idx    = 0;
        }

    } // end for artist loop

    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50);
      addItem(info);
    }
}



void LocalScene::populateTrackScene()
{
    int artistRow     = 0;
    int albumRow      = 0;
    int trackRow      = 0;
    int trackPerAlbum = 0;
    int offset        = 0;
    m_infosize        = 0;

    //! header item
    HeaderItem* header = new HeaderItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    header->setText( tr("Tracks") );
    header->setPos(0, 5);
    addItem(header);
    
    artistRow++;

    //! artist loop
    for ( int i = 0; i < m_localTrackModel->rootItem()->childCount(); i++ )
    {
      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(m_localTrackModel->rootItem()->child(i));

      if(!m_localTrackModel->isArtistFiltered(artist) ) continue;

      CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      category->m_name = artist->name;
      category->setPos( 0 ,artistRow*50 + trackRow*20 + albumRow*30 + offset );
      addItem(category);

      artistRow++;

      //! album loop
      for (int j = 0; j < artist->childCount(); j++)
      {
        MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(artist->child(j));

        if(!m_localTrackModel->isAlbumFiltered(album)) continue;

        AlbumGraphicItem_v2 *album_item = new AlbumGraphicItem_v2();
        album_item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        album_item->media = album;
        album_item->setPos(4, artistRow*50 + trackRow*20 + albumRow*30 + offset );
        addItem(album_item);

        trackPerAlbum = 0;

        //! track loop
        int disc_number = 0;
        bool m_isGrouping = DatabaseManager::instance()->DB_PARAM().groupAlbums;
        for (int k = 0; k < album->childCount(); k++)
        {
          MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(album->child(k));
          if(!m_localTrackModel->isTrackFiltered(track)) continue;

          if(m_isGrouping && album->isMultiset() && (disc_number != track->disc_number)) 
          {
            disc_number = track->disc_number;
            
            QGraphicsTextItem* text_item = new QGraphicsTextItem(QString(tr("disc %1")).arg(disc_number));
            text_item->setDefaultTextColor(QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
            text_item->setFont( QFont("Arial", 10, QFont::Bold) );

            text_item->setPos(160, artistRow*50 + trackRow*20 + albumRow*30 + offset);
            addItem(text_item);

            trackRow++;
          }

          TrackGraphicItem_v2 *track_item = new TrackGraphicItem_v2();
          track_item->media = track;
          track_item->setPos(155, artistRow*50 + trackRow*20 + albumRow*30 + offset);

          //PATCH (-20 => fix alignement of the scene)
          track_item->_width = parentView()->width()-155-20;

          addItem(track_item);

          trackRow++;
          trackPerAlbum++;
          m_infosize++;
        }
        albumRow++;

        if( trackPerAlbum < 8 ) offset = offset + (8 - trackPerAlbum)*20;
      }
    }

    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + artistRow*50);
      addItem(info);
    }
}



void LocalScene::populateGenreScene()
{
    int categorieRow = 0;
    int albumRow     = 0;
    int Column       = 0;
    m_infosize       = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;

    //! header item
    HeaderItem* header = new HeaderItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    header->setText( tr("Albums by genre") );
    header->setPos(0, 5);
    addItem(header);    

    categorieRow++;

    QString s_genre     = "";
    MEDIA::MediaPtr     media;
    

    foreach (MEDIA::TrackPtr track, m_localTrackModel->trackByGenre)
    {
      //! add filtre
      if(s_genre == track->genre && media == track->parent()) continue;
      if(!m_localTrackModel->isTrackFiltered(track)) continue;

      if(media != track->parent())
      {
        /* ------- New Genre ------- */
        if(s_genre != track->genre)
        {
          s_genre = track->genre;

          if(Column>0) albumRow++;

          CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
          category->m_name = s_genre;
          category->setPos( 0 , 10 + categorieRow*50 + albumRow*170);
          addItem(category);

          m_infosize++; // on compte les categorie = genre
          categorieRow++;
          Column     = 0;
        }

        /* ------- New Album ------- */
        media = track->parent();

        AlbumGenreGraphicItem *album_item = new AlbumGenreGraphicItem();
        album_item->media    = MEDIA::AlbumPtr::staticCast(track->parent());
        album_item->_genre   = s_genre;
        album_item->setPos(4+160*Column, categorieRow*50 + albumRow*170);
        addItem(album_item);

        if(Column < (item_count-1)) {
          Column++;
        }
        else {
          Column = 0;
          albumRow++;
        }
      }
    } // fin foreach track

    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50);
      addItem(info);
    }
}


void LocalScene::populateYearScene()
{
    int categorieRow = 0;
    int albumRow     = 0;
    int Column       = 0;
    m_infosize       = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;

    //! header item
    HeaderItem* header = new HeaderItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    header->setText( tr("Albums by year") );
    header->setPos(0, 5);
    addItem(header);    

    categorieRow++;

    int year         = -1;
    int idx          = 0;

    QList<MEDIA::AlbumPtr> list_album_by_year = m_localTrackModel->albumItemList;

    //! Sort Media Album Item list By Year
    qSort(list_album_by_year.begin(), list_album_by_year.end(),MEDIA::compareAlbumItemYear);

    foreach (MEDIA::AlbumPtr album, list_album_by_year)
    {
      //! add filtre
      if(!m_localTrackModel->isAlbumFiltered(album)) continue;

      if(year != album->year) {
        //! new year
        year = album->year;

        if(idx>0) albumRow++;

        CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
        category->m_name = QString::number(year);
        category->setPos( 0 , 10 + categorieRow*50 + albumRow*170);
        addItem(category);

        m_infosize++; // on compte les categorie = genre
        categorieRow++;
        Column     = 0;
        idx        = 0;
      }

      AlbumGraphicItem *album_item = new AlbumGraphicItem();
      album_item->media  = album;
      album_item->setPos(4+160*Column, categorieRow*50 + albumRow*170);
      addItem(album_item);

      if(Column < (item_count-1)) {
        Column++;
        idx++;
      }
      else {
        Column = 0;
        albumRow++;
        idx=0;
      }
    } // fin foreach track


    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50);
      addItem(info);
    }
}



void LocalScene::populateFavoriteScene()
{
    Debug::debug() << "## LocalScene::populateFavoriteScene";
    
    int categorieRow = 0;
    int artistRow    = 0;
    int albumRow     = 0;
    int Column       = 0;
    m_infosize        = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;

    //! header item
    HeaderItem* header = new HeaderItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    header->setText( tr("Favorites") );
    header->setPos(0, 5);
    addItem(header);    
    
    categorieRow++;
    
    CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    category->m_name = tr("Artists");
    category->setPos( 0 , 10 + categorieRow*50 + artistRow*165);
    addItem(category);

    categorieRow++;


    /*-----------------------------------------------*/
    /* Favorite Artists                              */
    /* ----------------------------------------------*/
    for ( int i = 0; i < m_localTrackModel->rootItem()->childCount(); i++ ) {

      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(m_localTrackModel->rootItem()->child(i));

      if(!artist->isFavorite) continue;
      if(!m_localTrackModel->isArtistFiltered(artist)) continue;

      m_infosize++; // we count artist favorite item

      //! New Artist Item
      ArtistGraphicItem *artist_item = new ArtistGraphicItem();
      artist_item->media = artist;
      artist_item->setPos( 4 + 160*Column , categorieRow*50 + artistRow*165);
      addItem(artist_item);

      if(Column < (item_count-1)) {
        Column++;
      }
      else {
        Column = 0;
        artistRow++;
      }

      //! ALBUM COVER LOOP
      artist->album_covers.clear();
      for(int j = artist->childCount()-1 ; j >= 0; j--) {
          MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(artist->child(j));
          if(!m_localTrackModel->isAlbumFiltered(album) ) continue;

          artist->album_covers.prepend(album);

          //!WARNING limite de l'affichage à 6 cover max
          if(artist->album_covers.size() >=6) break;
      }
    } // end artist loop

    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50 + artistRow*165);
      addItem(info);
      categorieRow++;
    }


    /*-----------------------------------------------*/
    /* Favorite Albums                               */
    /* ----------------------------------------------*/
    if (Column > 0) artistRow++;

    CategorieGraphicItem *category2 = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    category2->m_name = tr("Albums");
    category2->setPos( 0 , 10 + categorieRow*50 + artistRow*165);
    addItem(category2);

    Column = 0;
    categorieRow++;
    int oldInfoSize = m_infosize;
    foreach (MEDIA::AlbumPtr album, m_localTrackModel->albumItemList)
    {
        if(!album->isFavorite)  continue;
        if(!m_localTrackModel->isAlbumFiltered(album)) continue;

        m_infosize++; // we count album favorite item

        AlbumGraphicItem *album_item = new AlbumGraphicItem();
        album_item->media = album;
        album_item->setPos(4+160*Column, categorieRow*50 + artistRow*165 + albumRow*170);
        addItem(album_item);

        if(Column < (item_count-1)) {
          Column++;
        }
        else {
          Column = 0;
          albumRow++;
        }
    } // end album loop

    if(oldInfoSize == m_infosize) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text   = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50 + artistRow*165);
      addItem(info);
    }
}



void LocalScene::populatePlaylistOverviewScene()
{
    int categorieRow = 0;
    int playlistRow  = 0;
    int Column       = 0;
    m_infosize       = 0;
    int idx    = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;

    //! header item
    HeaderItem* header = new HeaderItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    header->setText( tr("Playlists") );
    header->setPos(0, 5);
    addItem(header);
    
    categorieRow++;

    /*-----------------------------------------------*/
    /* Playlist T_DATABASE                           */
    /* ----------------------------------------------*/
    CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    category->m_name = tr("Playlist files from internal database");
    category->setPos( 0 ,10 + categorieRow*50 + playlistRow*150);
    addItem(category);
    categorieRow++;

    //! playlist loop for T_DATABASE playlist
    int i = 0;
    for ( ; i < m_localPlaylistModel->rootItem()->childCount(); i++ )
    {
      MEDIA::PlaylistPtr playlist = MEDIA::PlaylistPtr::staticCast(m_localPlaylistModel->rootItem()->child(i));

      //WARNING drop smart playlist for this view
      if(playlist->p_type != T_DATABASE) break;

      if(!m_localPlaylistModel->isPlaylistFiltered(playlist) ) continue;

      m_infosize++;
      idx++; // nouvelle ligne

      PlaylistGraphicItem *playlist_item = new PlaylistGraphicItem();
      playlist_item->media = playlist;
      playlist_item->setPos(4+160*Column, categorieRow*50 + playlistRow*150);
      addItem(playlist_item);

      if(Column < (item_count-1)) {
        Column++;
      }
      else {
        Column = 0;
        playlistRow++;
        idx = 0;
      }
    }

    if(m_infosize == 0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50 + playlistRow*150);
      addItem(info);
      categorieRow++;
    }

    int oldInfoSize = m_infosize;

    if(idx>0) playlistRow++;
    Column = 0;

    /*-----------------------------------------------*/
    /* Playlist T_FILE                               */
    /* ----------------------------------------------*/
    CategorieGraphicItem *category2 = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    category2->m_name = tr("Playlist files from user collection");
    category2->setPos( 0 ,10 + categorieRow*50 + playlistRow*150);
    addItem(category2);
    categorieRow++;

    //! playlist loop for T_DATABASE playlist
    for ( ; i < m_localPlaylistModel->rootItem()->childCount(); i++ )
    {
      MEDIA::PlaylistPtr playlist = MEDIA::PlaylistPtr::staticCast(m_localPlaylistModel->rootItem()->child(i));

      //WARNING drop smart playlist for this view
      if(playlist->p_type != T_FILE) break;

      if(!m_localPlaylistModel->isPlaylistFiltered(playlist) ) continue;

      m_infosize++;
      idx++; // nouvelle ligne

      PlaylistGraphicItem *playlist_item = new PlaylistGraphicItem();
      playlist_item->media = playlist;
      playlist_item->setPos(4+160*Column, categorieRow*50 + playlistRow*150);
      addItem(playlist_item);

      if(Column < (item_count-1)) {
        Column++;
      }
      else {
        Column = 0;
        playlistRow++;
        idx = 0;
      }
    }

    if(m_infosize == oldInfoSize) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50 + playlistRow*150);
      addItem(info);
    }
}


void LocalScene::populatePlaylistByTrackScene()
{
    int categorieRow  = 0;
    int playlistRow   = 0;
    int trackRow      = 0;
    int trackPerAlbum = 0;
    int offset        = 0;
    m_infosize         = 0;

   
    //! header item
    HeaderItem* header = new HeaderItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    header->setText( tr("Playlists") );
    header->setPos(0, 5);
    addItem(header);    

    categorieRow++;


    //! playlist loop
    for ( int i = 0; i < m_localPlaylistModel->rootItem()->childCount(); i++ )
    {
      MEDIA::PlaylistPtr playlist = MEDIA::PlaylistPtr::staticCast(m_localPlaylistModel->rootItem()->child(i));

      //WARNING drop smart playlist for this view
      if(playlist->p_type == T_SMART) break;

      if(!m_localPlaylistModel->isPlaylistFiltered(playlist) ) continue;

      CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      category->m_name = playlist->name;
      category->setPos( 0 , 10 + categorieRow*50 + playlistRow*30 + trackRow*20 + offset );
      addItem(category);

      categorieRow++;

      PlaylistGraphicItem *playlist_item = new PlaylistGraphicItem();
      playlist_item->setFlag(QGraphicsItem::ItemIsSelectable, false);
      playlist_item->media = playlist;
      playlist_item->setPos(10, categorieRow*50 + playlistRow*30 + trackRow*20 + offset );
      addItem(playlist_item);

      //! track loop
      trackPerAlbum = 0;
      for (int k = 0; k < playlist->childCount(); k++)
      {
        MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(playlist->child(k));
        if(!m_localPlaylistModel->isTrackFiltered(track)) continue;

        TrackGraphicItem *track_item = new TrackGraphicItem();
        track_item->media = track;
        track_item->setPos(155, categorieRow*50 + playlistRow*30 + trackRow*20 + offset);
        track_item->_width = parentView()->width()-155 -20;

        addItem(track_item);

        trackRow++;
        trackPerAlbum++;
        m_infosize++;
      }
      playlistRow++;
      if( trackPerAlbum < 6 ) offset = offset + (6 - trackPerAlbum)*25;
    }


    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50 + playlistRow*150);
      addItem(info);
    }
}



void LocalScene::populatePlaylistSmartScene()
{
    int categorieRow = 0;
    int playlistRow  = 0;
    int Column       = 0;
    m_infosize        = 0;
    int idx = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;

    //! header item
    HeaderItem* header = new HeaderItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    header->setText( tr("Smart playlists") );
    header->setPos(0, 5);
    addItem(header);    

    categorieRow++;

    CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    category->m_name = tr("Smart playlists");
    category->setPos( 0 ,10 + categorieRow*50 + playlistRow*150);
    addItem(category);
    categorieRow++;


    //! playlist loop
    for ( int i = 0; i < m_localPlaylistModel->rootItem()->childCount(); i++ )
    {
      MEDIA::PlaylistPtr playlist = MEDIA::PlaylistPtr::staticCast(m_localPlaylistModel->rootItem()->child(i));
      if(playlist->p_type != T_SMART)
        continue;

      m_infosize++;
      idx++; // nouvelle ligne

      PlaylistGraphicItem *playlist_item = new PlaylistGraphicItem();
      playlist_item->media = playlist;
      playlist_item->setPos(4+160*Column, categorieRow*50 + playlistRow*150);
      addItem(playlist_item);

      if(Column < (item_count-1)) {
        Column++;
      }
      else {
        Column = 0;
        playlistRow++;
        idx = 0;
      }
    }
}



void LocalScene::populateHistoScene()
{
    //Debug::debug() << "   [LocalScene] populateHistoScene";

    int categorieRow  = 0;
    int trackRow      = 0;
    m_infosize        = 0;

    m_histoModel->updateModel();

    //! header item
    HeaderItem* header = new HeaderItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    header->setText( tr("History") );
    header->setPos(0, 5);
    addItem(header);    

    categorieRow++;

    //! track loop from history model
    QDate oldDate, newDate;
    for ( int i = 0; i < m_histoModel->itemCount(); i++ )
    {
      if(!m_histoModel->isItemFiltered(i)) continue;

      MEDIA::TrackPtr track = m_histoModel->trackAt(i);

      newDate = QDateTime::fromTime_t(track->lastPlayed).date();

      if(oldDate != newDate)
      {
        oldDate = newDate;

        CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
        category->m_name = track->lastplayed_ago();
        category->setPos(0 , 10 + categorieRow*50 + trackRow*20);
        addItem(category);

        categorieRow++;
      }

      TrackGraphicItem_v3 *track_item = new TrackGraphicItem_v3();
      track_item->media = track;
      track_item->setPos(30, categorieRow*50 + trackRow*20);
      track_item->_width = parentView()->width()-50 -20;

      if(track->type() == TYPE_STREAM)
        track_item->setToolTip(track->url);

      addItem(track_item);
      m_infosize++;

      trackRow++;
    }

    if (m_infosize == 0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 10 ,10 + categorieRow*50);
      addItem(info);
    }
}

/*******************************************************************************
 Mouse and menu event
*******************************************************************************/
void LocalScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Debug::debug() << "   [LocalScene] contextMenuEvent";
    m_mouseGrabbedItem = this->itemAt(event->scenePos());
    if(!m_mouseGrabbedItem) {
      m_graphic_item_menu->hide();
      event->accept();
      
      return;
    }

    if(this->selectedItems().isEmpty() || !selectedItems().contains(m_mouseGrabbedItem))
    {
      m_graphic_item_menu->appendItem(m_mouseGrabbedItem);
      m_graphic_item_menu->updateMenu(false);
    }
    else if (selectedItems().count() == 1)
    {
      m_mouseGrabbedItem = selectedItems().first();
      clearSelection();
      m_graphic_item_menu->appendItem(m_mouseGrabbedItem);
      m_graphic_item_menu->updateMenu(false);
    }
    else 
    {
      m_graphic_item_menu->appendItems(selectedItems());
      m_graphic_item_menu->updateMenu(true);
    }

    m_graphic_item_menu->exec(event->screenPos());

    // no need to propagate to item
    event->accept();
}

void LocalScene::slot_contextmenu_triggered(ENUM_ACTION_ITEM_MENU id)
{
    switch(id)
    {
      case ALBUM_PLAY          : playAlbum();              break;
      case ALBUM_LOAD_COVER    : loadCoverArtFromFile();   break;
      case ALBUM_LASTFM_COVER  : loadLastFmCover();        break;
      case ALBUM_REMOVE_COVER  : removeCoverArt();         break;
      case ALBUM_QUEUE_END     : enqueueAlbum(false);      break;
      case ALBUM_QUEUE_NEW     : enqueueAlbum(true);       break;
      case ALBUM_FAVORITE      : updateAlbumFavorite();    break;
      case GENRE_PLAY          : playAlbumGenre();         break;
      case GENRE_QUEUE_END     : enqueueAlbumGenre(false); break;
      case GENRE_QUEUE_NEW     : enqueueAlbumGenre(true);  break;
      case ARTIST_PLAY         : playArtist();             break;
      case ARTIST_QUEUE_END    : enqueueArtist(false);     break;
      case ARTIST_QUEUE_NEW    : enqueueArtist(true);      break;
      case ARTIST_FAVORITE     : updateArtistFavorite();   break;
      case PLAYLIST_PLAY       : playPlaylist();           break;
      case PLAYLIST_EDIT       : edit_playlist_dialog();   break;
      case PLAYLIST_QUEUE_END  : enqueuePlaylist(false);   break;
      case PLAYLIST_QUEUE_NEW  : enqueuePlaylist(true);    break;
      case PLAYLIST_FAVORITE   : updatePlaylistFavorite(); break;
      case PLAYLIST_REMOVE     : removePlaylistFromDisk(); break;
      case TRACK_PLAY          : playTrack();              break;
      case TRACK_QUEUE_END     : enqueueTrack(false);      break;
      case TRACK_QUEUE_NEW     : enqueueTrack(true);       break;

      case ALBUM_EDIT          :
      case GENRE_EDIT          :
      case ARTIST_EDIT         :
      case TRACK_EDIT          : edit_media_dialog();      break;

      case SELECTION_PLAY      : playSelected();break;
      case SELECTION_QUEUE_END : enqueueSelected(false);break;
      case SELECTION_QUEUE_NEW : enqueueSelected(true);break;

      default:break;
    }
}


void LocalScene::slot_item_mouseMove()
{
    Debug::debug() << "   [LocalScene] slot_item_mouseMove";

    // single drag and drop
    QGraphicsItem *gItem = qvariant_cast<QGraphicsItem *>( (ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE))->data() );

    if(this->selectedItems().isEmpty() || !selectedItems().contains(gItem))
    {
      switch(gItem->type()) {
        case GraphicsItem::AlbumType      : startAlbumsDrag(gItem);break;
        case GraphicsItem::AlbumGenreType : startAlbumsGenreDrag(gItem);break;
        case GraphicsItem::ArtistType     : startArtistsDrag(gItem);break;
        case GraphicsItem::TrackType      : startTracksDrag(gItem);break;
        case GraphicsItem::PlaylistType   : startPlaylistsDrag(gItem);break;
      }
    }
    // multiple drag and drop
    else
    {
      switch(gItem->type()) {
        case GraphicsItem::AlbumType      : startAlbumsDrag();break;
        case GraphicsItem::AlbumGenreType : startAlbumsGenreDrag();break;
        case GraphicsItem::ArtistType     : startArtistsDrag();break;
        case GraphicsItem::TrackType      : startTracksDrag();break;
        case GraphicsItem::PlaylistType   : startPlaylistsDrag();break;
      }
    }
}


void LocalScene::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
{
    //Debug::debug() << "   [LocalScene] mouseDoubleClickEvent";
    // A FAIRE EGALEMENT DANS LES AUTRES SCENE SI CELA FONCTIONNE
    m_mouseGrabbedItem = this->itemAt(event->scenePos());

    if(!m_mouseGrabbedItem) {
      event->ignore();
      return;
    }

    // ENQUEUE
    if(Qt::ControlModifier == QApplication::keyboardModifiers())
    {
      switch(m_mouseGrabbedItem->type()) {
        case GraphicsItem::AlbumType      : enqueueAlbum(false);break;
        case GraphicsItem::AlbumGenreType : enqueueAlbumGenre(false);break;
        case GraphicsItem::ArtistType     : enqueueArtist(false);break;
        case GraphicsItem::TrackType      : enqueueTrack(false);break;
        case GraphicsItem::PlaylistType   : enqueuePlaylist(false);break;
        default: event->ignore(); break;
      }
    }
    // PLAY ITEM
    else
    {
        switch(m_mouseGrabbedItem->type()) {
          case GraphicsItem::AlbumType      : playAlbum();break;
          case GraphicsItem::AlbumGenreType : playAlbumGenre();break;
          case GraphicsItem::ArtistType     : playArtist();break;
          case GraphicsItem::TrackType      : playTrack();break;
          case GraphicsItem::PlaylistType   : playPlaylist();break;
          default: event->ignore(); break;
        }
    }

    // no need to propagate to item
    event->accept();
}



/*******************************************************************************
    keyPressEvent
*******************************************************************************/
void LocalScene::keyPressEvent ( QKeyEvent * keyEvent )
{
//     Debug::debug() << "  [LocalScene] keyPressEvent " << keyEvent->key();

    if( keyEvent->key() == Qt::Key_Escape ) 
    {
       clearSelection();
       keyEvent->accept();
       return;      
    }
    else if( keyEvent->key() == Qt::Key_Return ) 
    {
       if( selectedItems().isEmpty() )
           playSelected();
//        else
//            playItem();
       
       keyEvent->accept();
       return;       
    }
    
    QGraphicsScene::keyPressEvent (keyEvent);
}

/*******************************************************************************
    slot_item_ratingclick
*******************************************************************************/
void LocalScene::slot_item_ratingclick()
{
    QList<QGraphicsView *> view = this->views();
    if( view.isEmpty() )
      return;

    //Debug::debug() << "   [LocalScene] slot_item_ratingclick";
    QGraphicsItem *gItem = qvariant_cast<QGraphicsItem *>( (ACTIONS()->value(BROWSER_ITEM_RATING_CLICK))->data() );

    switch(gItem->type()) {
      case GraphicsItem::AlbumType      :
      case GraphicsItem::AlbumGenreType : rateAlbum(gItem);break;
      case GraphicsItem::ArtistType     : rateArtist(gItem);break;
      case GraphicsItem::TrackType      : rateTrack(gItem);break;
      default: return;break;
    }
}

/*******************************************************************************
  LocalScene::sortedSelectedItem
*******************************************************************************/
QList<QGraphicsItem*> LocalScene::sortedSelectedItem()
{
    QList<QGraphicsItem*> selected = selectedItems();
    int count = selected.count();

    bool sort_ok = false;

    do
    {
      sort_ok = false;
      for(int i =0; i<=count-2; i++)
      {
        QGraphicsItem* gi1 = selected.at(i);
        QGraphicsItem* gi2 = selected.at(i+1);

        QPoint p1 = gi1->scenePos().toPoint();
        QPoint p2 = gi2->scenePos().toPoint();

        bool lessthan;
        if(p1.y() < p2.y())
          lessthan = true;
        else if (p1.y() > p2.y())
          lessthan = false;
        else if (p1.x() < p2.x())
          lessthan = true;
        else
          lessthan = false;

        if(!lessthan) {
          selected.swap(i,i+1);
          sort_ok = true;
        }
      } // end for
    } while(sort_ok);

    return selected;
}


/*******************************************************************************
  LocalScene::get_item_position
*******************************************************************************/
QPoint LocalScene::get_item_position(MEDIA::MediaPtr media)
{
    Debug::debug() << "   [LocalScene] get_item_position";
    QPoint point = QPoint(0,0);
    if(media->type() == TYPE_ARTIST && mode() == VIEW::ViewArtist)
    {
      foreach(QGraphicsItem* gItem, this->items()) {
        if(gItem->type() != GraphicsItem::ArtistType) continue;

        ArtistGraphicItem* artist_item = static_cast<ArtistGraphicItem*>(gItem);

        if(artist_item->media->id == MEDIA::ArtistPtr::staticCast(media)->id) {
          point = artist_item->scenePos().toPoint();
          break;
        }
      }
    }
    else if(media->type() == TYPE_ALBUM && mode() == VIEW::ViewAlbum)
    {
      foreach(QGraphicsItem* gItem, this->items()) {
        if(gItem->type() != GraphicsItem::AlbumType) continue;

        AlbumGraphicItem* album_item = static_cast<AlbumGraphicItem*>(gItem);

        if(album_item->media->id == MEDIA::AlbumPtr::staticCast(media)->id) {
          point = album_item->scenePos().toPoint();
          break;
        }
      }
    }
    else if(media->type() == TYPE_TRACK && mode() == VIEW::ViewTrack)
    {
      foreach(QGraphicsItem* gItem, this->items()) {
        if(gItem->type() != GraphicsItem::TrackType) continue;

        TrackGraphicItem* track_item = static_cast<TrackGraphicItem*>(gItem);

        if(track_item->media->id == MEDIA::TrackPtr::staticCast(media)->id) {
          point = track_item->scenePos().toPoint();
          break;
        }
      }
    }

    return point;
}


/*******************************************************************************
    album cover stuff
*******************************************************************************/
void LocalScene::loadCoverArtFromFile()
{
    //Debug::debug() << "   [LocalScene] loadCoverArtFromFile";
    if(!m_mouseGrabbedItem) return;
    AlbumGraphicItem *item = static_cast<AlbumGraphicItem*>(m_mouseGrabbedItem);

    //! open file dialog
    FileDialog fd(0, FileDialog::AddFile, tr("Select cover file..."));
    QStringList filters = QStringList() << tr("Files (*.png *.jpg)");
    fd.setFilters(filters);

    if(fd.exec() == QDialog::Accepted) 
    {
      QString filename  = fd.addFile();

      if( filename.isEmpty() ) return;

      const QString cover_path = UTIL::CONFIGDIR + "/albums/" + item->media->coverpath;

      //! get new cover file
      QImage image = QImage(filename);
      image = image.scaled(QSize(110, 110), Qt::KeepAspectRatio, Qt::SmoothTransformation);

      //! remove existing cover file
      if(QFile::exists(cover_path))
      {
        QFile::remove(cover_path);
        CoverCache::instance()->invalidate(item->media);
      }
      //! save new cover file
      image.save(cover_path, "png", -1);

      //! scene update
      item->update();
    }
}

void LocalScene::loadLastFmCover()
{
    //Debug::debug() << "   [LocalScene] loadLastFmCover";

    if(!m_mouseGrabbedItem) return;

    AlbumGraphicItem *item = static_cast<AlbumGraphicItem*>(m_mouseGrabbedItem);

    //! Get album Name and coverPath
    const QString album      =  item->media->name;
    MEDIA::ArtistPtr artist_ptr = MEDIA::ArtistPtr::staticCast(item->media->parent());
    const QString artist     =  artist_ptr->name;

    //! start search cover process
    ThreadManager::instance()->startCoverSearch(artist, album);
}

void LocalScene::removeCoverArt()
{
    //Debug::debug() << "   [LocalScene] RemoveCoverArtForAlbum";
    if(!m_mouseGrabbedItem) return;
    AlbumGraphicItem *item = static_cast<AlbumGraphicItem*>(m_mouseGrabbedItem);

    const QString cover_path =  UTIL::CONFIGDIR + "/albums/" + item->media->coverpath;

    //! remove existing cover file
    if(QFile::exists(cover_path))
    {
      QFile::remove(cover_path);
      CoverCache::instance()->invalidate(item->media);
      item->update();
    }
}


/*******************************************************************************
    Edit media dialog
*******************************************************************************/
void LocalScene::edit_media_dialog()
{
    //Debug::debug() << "   [LocalScene] edit_media_dialog ";
    MediaItem_Edit_Dialog *dialog = 0;

    if(!m_mouseGrabbedItem) return;

    if(m_mouseGrabbedItem->type()== GraphicsItem::ArtistType)
    {
      ArtistGraphicItem *item = static_cast<ArtistGraphicItem*>(m_mouseGrabbedItem);
      dialog = new MediaItem_Edit_Dialog(DIALOG_ARTIST);
      dialog->setMediaItem(item->media);
    }
    else if(m_mouseGrabbedItem->type()== GraphicsItem::AlbumType)
    {
      AlbumGraphicItem *item = static_cast<AlbumGraphicItem*>(m_mouseGrabbedItem);
      dialog = new MediaItem_Edit_Dialog(DIALOG_ALBUM);
      dialog->setMediaItem(item->media);
    }
    else if(m_mouseGrabbedItem->type()== GraphicsItem::AlbumGenreType)
    {
      AlbumGenreGraphicItem *item = static_cast<AlbumGenreGraphicItem*>(m_mouseGrabbedItem);
      dialog = new MediaItem_Edit_Dialog(DIALOG_GENRE);
      dialog->setGenre(item->_genre);
      dialog->setMediaItem(item->media);
    }
    else if(m_mouseGrabbedItem->type()== GraphicsItem::TrackType)
    {
      TrackGraphicItem *item = static_cast<TrackGraphicItem*>(m_mouseGrabbedItem);
      dialog = new MediaItem_Edit_Dialog(DIALOG_TRACK);
      dialog->setMediaItem(item->media);
    }

    if(dialog) {
      //! blocking call (modal dialog)
      dialog->exec();

      if( (dialog->result() == QDialog::Accepted) && dialog->isChanged()) {
        ThreadManager::instance()->populateLocalTrackModel();
      }
      dialog->deleteLater();
    }
}


/*******************************************************************************
    Update favorite status
*******************************************************************************/
void LocalScene::updateAlbumFavorite()
{
    if(!m_mouseGrabbedItem) return;
    AlbumGraphicItem *item = static_cast<AlbumGraphicItem*>(m_mouseGrabbedItem);

    QList<int> db_ids;
    if(item->media->isMultiset())
      db_ids << item->media->ids;
    else
      db_ids << item->media->id;
    
    bool isFavorite   = item->media->isFavorite;

    item->media->isFavorite = !isFavorite;

    Database db;
    if (!db.connect()) return;

    foreach (const int &id, db_ids) 
    {
      QSqlQuery q("",*db.sqlDb());
      q.prepare("UPDATE albums SET favorite=? WHERE id=?;");
      q.addBindValue( int(!isFavorite) );
      q.addBindValue( id );
      Debug::debug() << "database -> update album favorite :" << q.exec();
    }

    //! item update
    item->update();
}


void LocalScene::updateArtistFavorite()
{
    if(!m_mouseGrabbedItem) return;
    ArtistGraphicItem *item = static_cast<ArtistGraphicItem*>(m_mouseGrabbedItem);

    int id          = item->media->id;
    bool isFavorite = item->media->isFavorite;

    item->media->isFavorite = !isFavorite;

    Database db;
    if (!db.connect()) return;

    QSqlQuery q("",*db.sqlDb());
    q.prepare("UPDATE artists SET favorite=? WHERE id=?;");
    q.addBindValue( int(!isFavorite) );
    q.addBindValue( id );
    Debug::debug() << "database -> update artist favorite :" << q.exec();

    //! item update
    item->update();
}

/*******************************************************************************
    Playlist stuff
*******************************************************************************/
void LocalScene::removePlaylistFromDisk()
{
    if(!m_mouseGrabbedItem) return;
    PlaylistGraphicItem *item = static_cast<PlaylistGraphicItem*>(m_mouseGrabbedItem);

    //! get data
    const int id         = item->media->id;
    const QString fname  = item->media->url;
    const int type       = int(item->media->p_type);

    //Debug::debug() << "   [LocalScene] remove playlist item id" << dbId;

    //! playlist is a local file on disk --> remove file
    if(type == T_FILE) {
      QFile playlistFile(fname);
      if(playlistFile.exists())
        playlistFile.remove();
    }

    //! remove from database
    Database db;
    if (!db.connect()) return;

    if(type == T_DATABASE)
    {
      QSqlQuery q("",*db.sqlDb());
      q.prepare("DELETE FROM `playlists` WHERE `id`=?;");
      q.addBindValue( id );
      Debug::debug() << "database -> delete playlist :" << q.exec();
    
      q.prepare("DELETE FROM `playlist_items` WHERE `playlist_id` NOT IN (SELECT `id` FROM `playlists`);");
      Debug::debug() << "database -> delete playlist items :" << q.exec();
    }
    else //T_SMART
    {
      QSqlQuery q("",*db.sqlDb());
      q.prepare("DELETE FROM `smart_playlists` WHERE `id`=?;");
      q.addBindValue( id );
      Debug::debug() << "database -> delete smart playlist :" << q.exec();
    }

    ThreadManager::instance()->populateLocalPlaylistModel();
}


void LocalScene::edit_playlist_dialog()
{
    if(!m_mouseGrabbedItem) return;
    PlaylistGraphicItem *item = static_cast<PlaylistGraphicItem*>(m_mouseGrabbedItem);

    const int type       = int(item->media->p_type);

    if(type == T_DATABASE || type == T_FILE)
    {
      PlaylistEditor *editor = new PlaylistEditor(parentView());
      editor->setPlaylist(item->media);
      editor->show();

      connect(editor, SIGNAL(closed()), this, SLOT(slot_delete_playlist_editor()));
    }
    else //T_SMART
    {
      // open smart playlist editor with current smart playlist
      if( SmartPlaylist::edit_dialog( item->media ) )
        ThreadManager::instance()->populateLocalPlaylistModel();
    }
}

/*******************************************************************************
    Playlist creation
*******************************************************************************/
void LocalScene::slot_create_new_smart_playlist()
{
   // open smart playlist editor for a new one
   if( SmartPlaylist::create_dialog() )
     ThreadManager::instance()->populateLocalPlaylistModel();
}

void LocalScene::slot_create_new_playlist()
{
    PlaylistEditor *editor = new PlaylistEditor(parentView());
    editor->show();

    connect(editor, SIGNAL(closed()), this, SLOT(slot_delete_playlist_editor()));
}


void LocalScene::slot_delete_playlist_editor()
{
    PlaylistEditor *editor = qobject_cast<PlaylistEditor *>(sender());
    if(editor)
      editor->deleteLater();
}


void LocalScene::updatePlaylistFavorite()
{
    if(!m_mouseGrabbedItem) return;
    PlaylistGraphicItem *item = static_cast<PlaylistGraphicItem*>(m_mouseGrabbedItem);

    //! get data
    const int id         = item->media->id;
    bool isFavorite      = item->media->isFavorite;

    item->media->isFavorite = !isFavorite;

    Database db;
    if (!db.connect()) return;

    QSqlQuery q("",*db.sqlDb());
    if(item->media->p_type == T_DATABASE || item->media->p_type == T_FILE)
      q.prepare("UPDATE playlists SET favorite=? WHERE id=?;");
    else
      q.prepare("UPDATE smart_playlists SET favorite=? WHERE id=?;");

    q.addBindValue( int(!isFavorite) );
    q.addBindValue( id );
    Debug::debug() << "database -> update playlist favorite :" << q.exec();
    
    //! item update
    item->update();
}

/*******************************************************************************
    History model clear
*******************************************************************************/
void LocalScene::slot_clear_history()
{
    HistoManager::instance()->clearHistory();
    populateScene();
}

