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
#include "item_menu.h"

#include "local_item.h"
#include "stream_item.h"

#include "local_track_model.h"
#include "local_playlist_model.h"
#include "stream_model.h"
#include "playqueue/virtual_playqueue.h"

#include "mediaitem.h"
#include "global_actions.h"
#include "settings.h"

#include "debug.h"

#include <QtGui>



/*
********************************************************************************
*                                                                              *
*    Class GraphicsItemMenu                                                    *
*                                                                              *
********************************************************************************
*/
GraphicsItemMenu::GraphicsItemMenu(QWidget *parent) : QMenu(parent)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    this->setAutoFillBackground(true);
    this->setContentsMargins(0,0,0,0);
    this->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint );
    
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    this->setPalette(palette);
 
    //! creates menu actions
    m_map_actions.insert(ALBUM_PLAY,          new QAction(QIcon(":/images/media-play.png"), tr("Play"), 0));
    m_map_actions.insert(ALBUM_LOAD_COVER,    new QAction(QIcon(":/images/album.png"), tr("Load cover art from file"), 0));
    m_map_actions.insert(ALBUM_LASTFM_COVER,  new QAction(QIcon(":/images/download-48x48.png"), tr("Download cover art"), 0));
    m_map_actions.insert(ALBUM_REMOVE_COVER,  new QAction(QIcon::fromTheme("edit-delete"), tr("Remove cover art"), 0));
    m_map_actions.insert(ALBUM_EDIT,          new QAction(QIcon(":/images/edit-48x48.png"), tr("Edit"), 0));
    m_map_actions.insert(ALBUM_QUEUE_END,     new QAction(QIcon(":/images/playqueue_enqueue_end-48x48.png"), tr("Enqueue to playqueue"), 0));
    m_map_actions.insert(ALBUM_QUEUE_NEW,     new QAction(QIcon(":/images/playqueue_enqueue_new-48x48.png"), tr("Enqueue as new playqueue"), 0));
    m_map_actions.insert(ALBUM_FAVORITE,      new QAction(QIcon(":/images/favorites-48x48.png"), tr("Add to favorites"), 0));

    m_map_actions.insert(GENRE_PLAY,          new QAction(QIcon(":/images/media-play.png"), tr("Play"), 0));
    m_map_actions.insert(GENRE_QUEUE_END,     new QAction(QIcon(":/images/playqueue_enqueue_end-48x48.png"), tr("Enqueue to playqueue"), 0));
    m_map_actions.insert(GENRE_QUEUE_NEW,     new QAction(QIcon(":/images/playqueue_enqueue_new-48x48.png"), tr("Enqueue as new playqueue"), 0));
    m_map_actions.insert(GENRE_EDIT,          new QAction(QIcon(":/images/edit-48x48.png"), tr("Edit"), 0));

    m_map_actions.insert(ARTIST_PLAY,         new QAction(QIcon(":/images/media-play.png"), tr("Play"), 0));
    m_map_actions.insert(ARTIST_EDIT,         new QAction(QIcon(":/images/edit-48x48.png"), tr("Edit"), 0));
    m_map_actions.insert(ARTIST_QUEUE_END,    new QAction(QIcon(":/images/playqueue_enqueue_end-48x48.png"), tr("Enqueue to playqueue"), 0));
    m_map_actions.insert(ARTIST_QUEUE_NEW,    new QAction(QIcon(":/images/playqueue_enqueue_new-48x48.png"), tr("Enqueue as new playqueue"), 0));
    m_map_actions.insert(ARTIST_FAVORITE,     new QAction(QIcon(":/images/favorites-48x48.png"), tr("Add to favorites"), 0));

    m_map_actions.insert(PLAYLIST_PLAY,       new QAction(QIcon(":/images/media-play.png"), tr("Play"), 0));
    m_map_actions.insert(PLAYLIST_EDIT,       new QAction(QIcon(":/images/edit-48x48.png"), tr("Edit"), 0));
    m_map_actions.insert(PLAYLIST_QUEUE_END,  new QAction(QIcon(":/images/playqueue_enqueue_end-48x48.png"), tr("Enqueue to playqueue"), 0));
    m_map_actions.insert(PLAYLIST_QUEUE_NEW,  new QAction(QIcon(":/images/playqueue_enqueue_new-48x48.png"), tr("Enqueue as new playqueue"), 0));
    m_map_actions.insert(PLAYLIST_FAVORITE,   new QAction(QIcon(":/images/favorites-48x48.png"), tr("Add to favorites"), 0));
    m_map_actions.insert(PLAYLIST_REMOVE,     new QAction(QIcon::fromTheme("edit-delete"),tr("&Remove playlist from disk"), 0));

    m_map_actions.insert(TRACK_PLAY,          new QAction(QIcon(":/images/media-play.png"), tr("Play"), 0));
    m_map_actions.insert(TRACK_EDIT,          new QAction(QIcon(":/images/edit-48x48.png"), tr("Edit"), 0));
    m_map_actions.insert(TRACK_QUEUE_END,     new QAction(QIcon(":/images/playqueue_enqueue_end-48x48.png"), tr("Enqueue to playqueue"), 0));
    m_map_actions.insert(TRACK_QUEUE_NEW,     new QAction(QIcon(":/images/playqueue_enqueue_new-48x48.png"), tr("Enqueue as new playqueue"), 0));

    m_map_actions.insert(STREAM_PLAY,          new QAction(QIcon(":/images/media-play.png"), tr("Play"), 0));
    m_map_actions.insert(STREAM_EDIT,          new QAction(QIcon(":/images/edit-48x48.png"), tr("Edit"), 0));
    m_map_actions.insert(STREAM_QUEUE_END,     new QAction(QIcon(":/images/playqueue_enqueue_end-48x48.png"), tr("Enqueue to playqueue"), 0));
    m_map_actions.insert(STREAM_QUEUE_NEW,     new QAction(QIcon(":/images/playqueue_enqueue_new-48x48.png"), tr("Enqueue as new playqueue"), 0));
    m_map_actions.insert(STREAM_FAVORITE,      new QAction(QIcon(":/images/favorites-48x48.png"), tr("Add to favorites"), 0));

    m_map_actions.insert(SELECTION_PLAY,       new QAction(QIcon(":/images/media-play.png"), tr("Play"), 0));
    m_map_actions.insert(SELECTION_QUEUE_END,  new QAction(QIcon(":/images/playqueue_enqueue_end-48x48.png"), tr("Enqueue to playqueue"), 0));
    m_map_actions.insert(SELECTION_QUEUE_NEW,  new QAction(QIcon(":/images/playqueue_enqueue_new-48x48.png"), tr("Enqueue as new playqueue"), 0));


    // connect each actions
    foreach(QAction * action, m_map_actions.values() ) {
      QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_actionsTriggered()));
    }
    
}    

// public
void GraphicsItemMenu::appendItem(QGraphicsItem* item)
{
    m_items.clear();
    m_items << item;
}

void GraphicsItemMenu::appendItems(QList<QGraphicsItem*> items)
{
    m_items.clear();
    m_items << items;
}

void GraphicsItemMenu::updateMenu(bool isSelection)
{
    Debug::debug() << "GraphicsItemMenu::updateMenu";
  
    /* delete action_widget */
    this->clear();
  
    is_selection = isSelection;
  
    if(m_items.isEmpty()) return;


    /* main widget*/
    QWidget* main_widget = new QWidget();
    
    
    /*-------------------------------------------------*/
    /* Selected items                                  */
    /* ------------------------------------------------*/
    if( is_selection )
    {
      QString text;
      switch(m_items.first()->type()) {
        case GraphicsItem::ArtistType     : text = QString(tr("%1 artists")).arg(m_items.count());break;
        case GraphicsItem::AlbumGenreType :
        case GraphicsItem::AlbumType      : text = QString(tr("%1 albums")).arg(m_items.count());break;
        case GraphicsItem::TrackType      : text = QString(tr("%1 tracks")).arg(m_items.count());break;
        case GraphicsItem::StreamType     : text = QString(tr("%1 streams")).arg(m_items.count());break;
        case GraphicsItem::PlaylistType   : text = QString(tr("%1 playlist")).arg(m_items.count());break;
        default: break;
      }    
    
      QPalette palette;
      palette.setColor(QPalette::WindowText, SETTINGS()->_baseColor);

      QLabel* ui_label = new QLabel(this);
      ui_label->setFont(QFont("Arial",14,QFont::Normal));
      ui_label->setPalette(palette);    
    
      ui_label->setText(text);
      ui_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
      

      /* tool bar content */
      QToolBar *tool_bar = new QToolBar(main_widget);
      tool_bar->setIconSize( QSize( 16, 16 ) );
      tool_bar->addAction( m_map_actions.value(SELECTION_PLAY) );
      tool_bar->addAction( m_map_actions.value(SELECTION_QUEUE_END) );
      tool_bar->addAction( m_map_actions.value(SELECTION_QUEUE_NEW) );

      QHBoxLayout * hbox = new QHBoxLayout();
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
      hbox->addWidget(tool_bar);
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

      /* main layout */
      QVBoxLayout * main_box = new QVBoxLayout();
      main_box->setSpacing(2);
      main_box->setContentsMargins(0,0,0,0);
      main_box->addWidget(ui_label);
      main_box->addLayout(hbox);
      main_widget->setLayout(main_box);
    }
    /*-------------------------------------------------*/
    /* AlbumType                                       */
    /* ------------------------------------------------*/
    else if( m_items.first()->type() == GraphicsItem::AlbumType )
    {
      scene = new QGraphicsScene(main_widget);
      view = new QGraphicsView(main_widget);
    
      /* QGraphicsView setup */  
      view->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
      view->setAutoFillBackground(false);
      view->setFrameShape(QFrame::NoFrame);
      view->setFrameShadow(QFrame::Plain);

      view->setCacheMode(QGraphicsView::CacheNone);
      view->setDragMode(QGraphicsView::NoDrag);
      view->setRenderHint(QPainter::Antialiasing);
      
      view->setOptimizationFlags(  QGraphicsView::DontAdjustForAntialiasing
                                 | QGraphicsView::DontClipPainter
                                 | QGraphicsView::DontSavePainterState);
      view->setResizeAnchor(QGraphicsView::NoAnchor);
      view->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
      view->setAlignment(Qt::AlignLeft | Qt::AlignTop );

      view->setScene(scene);    
      scene->installEventFilter(this);

      /* get media data */
      AlbumGraphicItem *item = static_cast<AlbumGraphicItem*>(m_items.first());

      MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(item->media);

      int yPos = 0;

      AlbumGraphicItem_v4 *aiv2 = new AlbumGraphicItem_v4();
      aiv2->setFlag(QGraphicsItem::ItemIsSelectable, false);
      aiv2->media = album;
      aiv2->setPos(4, yPos);
      scene->addItem(aiv2);

      QList<MEDIA::TrackPtr> tracks = LocalTrackModel::instance()->getItemChildrenTracks(item->media);
    
      foreach(MEDIA::TrackPtr track, tracks) 
      {
          TrackGraphicItem_v2 *track_item = new TrackGraphicItem_v2();
          track_item->setFlag(QGraphicsItem::ItemIsSelectable, false);

          track_item->media = track;
          track_item->setPos(160, yPos);
          yPos += 20;

          scene->addItem(track_item);
      }    
    
      QPalette palette;
      palette.setColor(QPalette::WindowText, SETTINGS()->_baseColor);

      QLabel* ui_label = new QLabel(this);
      ui_label->setFont(QFont("Arial",14,QFont::Normal));
      ui_label->setPalette(palette);    
    
      ui_label->setText(album->name);
      ui_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    
      /* tool bar content */
      QToolBar *tool_bar = new QToolBar(main_widget);
      tool_bar->setIconSize( QSize( 16, 16 ) );
      tool_bar->addAction( m_map_actions.value(ALBUM_PLAY) );
      tool_bar->addAction( m_map_actions.value(ALBUM_QUEUE_END) );
      tool_bar->addAction( m_map_actions.value(ALBUM_QUEUE_NEW) );
      tool_bar->addSeparator();
      tool_bar->addAction( m_map_actions.value(ALBUM_LOAD_COVER) );
      tool_bar->addAction( m_map_actions.value(ALBUM_LASTFM_COVER) );
      tool_bar->addAction( m_map_actions.value(ALBUM_REMOVE_COVER) );
      tool_bar->addAction( m_map_actions.value(ALBUM_EDIT) );
      tool_bar->addSeparator();
      tool_bar->addAction( m_map_actions.value(ALBUM_FAVORITE) );
      tool_bar->addSeparator();
      
      tool_bar->addAction( ACTIONS()->value(BROWSER_JUMP_TO_MEDIA) );
      ACTIONS()->value(BROWSER_JUMP_TO_MEDIA)->setText(tr("jump to tracks"));
      QVariant v;
      v.setValue(static_cast<MEDIA::MediaPtr>(tracks.first()));
      ACTIONS()->value(BROWSER_JUMP_TO_MEDIA)->setData(v);
      
      
      QHBoxLayout * hbox = new QHBoxLayout();
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
      hbox->addWidget(tool_bar);
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

      if(item->media->isFavorite)
        m_map_actions.value(ALBUM_FAVORITE)->setText( tr("Remove from favorites") );
      else
        m_map_actions.value(ALBUM_FAVORITE)->setText( tr("Add to favorites") );

      /* main layout */
      QVBoxLayout * main_box = new QVBoxLayout();
      main_box->setSpacing(0);
      main_box->setContentsMargins(0,0,0,0);
      main_box->addWidget(ui_label);
      main_box->addWidget(view);
      main_box->addLayout(hbox);
      main_widget->setLayout(main_box);
    }
    /*-------------------------------------------------*/
    /* AlbumGenreType                                  */
    /* ------------------------------------------------*/
    else if( m_items.first()->type() == GraphicsItem::AlbumGenreType )
    {
      scene = new QGraphicsScene(main_widget);
      view = new QGraphicsView(main_widget);
    
      /* QGraphicsView setup */  
      view->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
      view->setAutoFillBackground(false);
      view->setFrameShape(QFrame::NoFrame);
      view->setFrameShadow(QFrame::Plain);

      view->setCacheMode(QGraphicsView::CacheNone);
      view->setDragMode(QGraphicsView::NoDrag);
      view->setRenderHint(QPainter::Antialiasing);
      view->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing
                                   | QGraphicsView::DontClipPainter
                                   | QGraphicsView::DontSavePainterState);
      view->setResizeAnchor(QGraphicsView::NoAnchor);
      view->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
      view->setAlignment(Qt::AlignLeft | Qt::AlignTop );

      view->setScene(scene);    
      scene->installEventFilter(this);

      /* get media data */
      AlbumGenreGraphicItem *item = static_cast<AlbumGenreGraphicItem*>(m_items.first());

      MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(item->media);

      int yPos = 0;

      AlbumGraphicItem_v4 *aiv2 = new AlbumGraphicItem_v4();
      aiv2->setFlag(QGraphicsItem::ItemIsSelectable, false);
      aiv2->media = album;
      aiv2->setPos(4, yPos);
      scene->addItem(aiv2);

      QList<MEDIA::TrackPtr> tracks = LocalTrackModel::instance()->getAlbumChildrenTracksGenre(item->media,item->_genre);
    
      foreach(MEDIA::TrackPtr track, tracks) 
      {
          TrackGraphicItem_v2 *track_item = new TrackGraphicItem_v2();
          track_item->setFlag(QGraphicsItem::ItemIsSelectable, false);

          track_item->media = track;
          track_item->setPos(160, yPos);
          yPos += 20;

          scene->addItem(track_item);
      }    
    
      QPalette palette;
      palette.setColor(QPalette::WindowText, SETTINGS()->_baseColor);

      QLabel* ui_label = new QLabel(this);
      ui_label->setFont(QFont("Arial",14,QFont::Normal));
      ui_label->setPalette(palette);    
    
      ui_label->setText(album->name);
      ui_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);      
      
      
      /* tool bar content */
      QToolBar *tool_bar = new QToolBar(main_widget);
      tool_bar->setIconSize( QSize( 16, 16 ) );
      tool_bar->addAction( m_map_actions.value(GENRE_PLAY) );
      tool_bar->addAction( m_map_actions.value(GENRE_QUEUE_END) );
      tool_bar->addAction( m_map_actions.value(GENRE_QUEUE_NEW) );
      tool_bar->addSeparator();
      tool_bar->addAction( m_map_actions.value(ALBUM_LOAD_COVER) );
      tool_bar->addAction( m_map_actions.value(ALBUM_LASTFM_COVER) );
      tool_bar->addAction( m_map_actions.value(ALBUM_REMOVE_COVER) );
      tool_bar->addAction( m_map_actions.value(GENRE_EDIT) );
      tool_bar->addSeparator();

      tool_bar->addAction( ACTIONS()->value(BROWSER_JUMP_TO_MEDIA) );
      ACTIONS()->value(BROWSER_JUMP_TO_MEDIA)->setText(tr("jump to tracks"));
      QVariant v;
      v.setValue(static_cast<MEDIA::MediaPtr>(tracks.first()));
      ACTIONS()->value(BROWSER_JUMP_TO_MEDIA)->setData(v);

      
      QHBoxLayout * hbox = new QHBoxLayout();
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
      hbox->addWidget(tool_bar);
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

      /* main layout */
      QVBoxLayout * main_box = new QVBoxLayout();
      main_box->setSpacing(0);
      main_box->setContentsMargins(0,0,0,0);
      main_box->addWidget(ui_label);
      main_box->addWidget(view);
      main_box->addLayout(hbox);
      main_widget->setLayout(main_box);
    }
    /*-------------------------------------------------*/
    /* ArtistType                                      */
    /* ------------------------------------------------*/
    else if( m_items.first()->type() == GraphicsItem::ArtistType )
    {
      //Debug::debug() << "GraphicsItemMenu::ArtistType";
      scene = new QGraphicsScene(main_widget);
      view = new QGraphicsView(main_widget);
    
      /* QGraphicsView setup */  
      view->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
      view->setAutoFillBackground(false);
      view->setFrameShape(QFrame::NoFrame);
      view->setFrameShadow(QFrame::Plain);

      view->setCacheMode(QGraphicsView::CacheNone);
      view->setDragMode(QGraphicsView::NoDrag);
      view->setRenderHint(QPainter::Antialiasing);
      view->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing
                                   | QGraphicsView::DontClipPainter
                                   | QGraphicsView::DontSavePainterState);
      view->setResizeAnchor(QGraphicsView::NoAnchor);
      view->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
      view->setAlignment(Qt::AlignLeft | Qt::AlignTop );

      view->setScene(scene);
      scene->installEventFilter(this);
    
      /* get media data */
      ArtistGraphicItem *item = static_cast<ArtistGraphicItem*>(m_items.first());
      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(item->media);
    
      int Column   = 0;
      int albumRow = 0;
      int max_item_per_colum = (m_browserview->width()-40)/155;
    
      view->setMaximumHeight(m_browserview->height() - 40 );
    
      //Debug::debug() << "max_item_per_colum " << max_item_per_colum;
      for (int i=0 ; i < artist->childCount(); i++ )
      {    
        MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(artist->child(i));

        AlbumGraphicItem_v2 *album_item = new AlbumGraphicItem_v2();
        album_item->media = artist->child(i);
        album_item->setPos(4+155*Column, albumRow*155);
        album_item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        scene->addItem(album_item);

        if(Column < max_item_per_colum) {
          Column++;
        }
        else {
          Column = 0;
          albumRow++;
        }
      }
    
      QPalette palette;
      palette.setColor(QPalette::WindowText, SETTINGS()->_baseColor);

      QLabel* ui_label = new QLabel(this);
      ui_label->setFont(QFont("Arial",14,QFont::Normal));
      ui_label->setPalette(palette);    
      ui_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);    
      ui_label->setText(artist->name);
       

      /* tool bar content */
      QToolBar *tool_bar = new QToolBar(main_widget);
      tool_bar->setIconSize( QSize( 16, 16 ) );
      tool_bar->addAction( m_map_actions.value(ARTIST_PLAY) );
      tool_bar->addAction( m_map_actions.value(ARTIST_QUEUE_END) );
      tool_bar->addAction( m_map_actions.value(ARTIST_QUEUE_NEW) );
      tool_bar->addSeparator();
      tool_bar->addAction( m_map_actions.value(ARTIST_EDIT) );
      tool_bar->addSeparator();
      tool_bar->addAction( m_map_actions.value(ARTIST_FAVORITE) );
      tool_bar->addSeparator();

      tool_bar->addAction( ACTIONS()->value(BROWSER_JUMP_TO_MEDIA) );
      ACTIONS()->value(BROWSER_JUMP_TO_MEDIA)->setText(tr("jump to albums"));
      QVariant v;
      v.setValue(static_cast<MEDIA::MediaPtr>(artist->child(0)));
      ACTIONS()->value(BROWSER_JUMP_TO_MEDIA)->setData(v);

      QHBoxLayout * hbox = new QHBoxLayout();
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
      hbox->addWidget(tool_bar);
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));


      if(item->media->isFavorite)
        m_map_actions.value(ARTIST_FAVORITE)->setText( tr("Remove from favorites") );
      else
        m_map_actions.value(ARTIST_FAVORITE)->setText( tr("Add to favorites") );

      /* main layout */
      QVBoxLayout * main_box = new QVBoxLayout();
      main_box->setSpacing(0);
      main_box->setContentsMargins(0,0,0,0);
      main_box->addWidget(ui_label);
      main_box->addWidget(view);
      main_box->addLayout(hbox);
      main_widget->setLayout(main_box);
    }
    /*-------------------------------------------------*/
    /* TrackType                                       */
    /* ------------------------------------------------*/
    else if( m_items.first()->type() == GraphicsItem::TrackType )
    {
      /* get media data */
      TrackGraphicItem *item = static_cast<TrackGraphicItem*>(m_items.first());
      MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(item->media);

      QPalette palette;
      palette.setColor(QPalette::WindowText, SETTINGS()->_baseColor);

      QLabel* ui_label = new QLabel(this);
      ui_label->setFont(QFont("Arial",14,QFont::Normal));
      ui_label->setPalette(palette);    
      ui_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
      
      /* hack for stream in playlist item */
      if( MEDIA::isLocal(item->media->url) )
        ui_label->setText(track->title);      
      else
        ui_label->setText(track->name); /* for stream */
      
      /* tool bar content */
      QToolBar *tool_bar = new QToolBar(main_widget);
      tool_bar->setIconSize( QSize( 16, 16 ) );
      tool_bar->addAction( m_map_actions.value(TRACK_PLAY) );

      if(item->media->id != -1)
        tool_bar->addAction( m_map_actions.value(TRACK_EDIT) );

      tool_bar->addAction( m_map_actions.value(TRACK_QUEUE_END) );
      tool_bar->addAction( m_map_actions.value(TRACK_QUEUE_NEW) );

      QHBoxLayout * hbox = new QHBoxLayout();
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
      hbox->addWidget(tool_bar);
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

      /* main layout */
      QVBoxLayout * main_box = new QVBoxLayout();
      main_box->setSpacing(0);
      main_box->setContentsMargins(0,0,0,0);
      main_box->addWidget(ui_label);
      main_box->addLayout(hbox);
      main_widget->setLayout(main_box);
    }
    /*-------------------------------------------------*/
    /* PlaylistType                                    */
    /* ------------------------------------------------*/
    else if( m_items.first()->type() == GraphicsItem::PlaylistType )
    {
      /* get media data */
      PlaylistGraphicItem *item = static_cast<PlaylistGraphicItem*>(m_items.first());
      MEDIA::PlaylistPtr playlist = MEDIA::PlaylistPtr::staticCast(item->media);
      
      QPalette palette;
      palette.setColor(QPalette::WindowText, SETTINGS()->_baseColor);
      
      QLabel* ui_label = new QLabel(this);
      ui_label->setFont(QFont("Arial",14,QFont::Normal));
      ui_label->setPalette(palette);    
      ui_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);    
      ui_label->setText(playlist->name);
      
      QToolBar *tool_bar = new QToolBar(main_widget);
      tool_bar->setIconSize( QSize( 16, 16 ) );
      tool_bar->addAction( m_map_actions.value(PLAYLIST_PLAY) );
      tool_bar->addAction( m_map_actions.value(PLAYLIST_QUEUE_END) );
      tool_bar->addAction( m_map_actions.value(PLAYLIST_QUEUE_NEW) );
      tool_bar->addSeparator();
      tool_bar->addAction( m_map_actions.value(PLAYLIST_REMOVE) );
      tool_bar->addAction( m_map_actions.value(PLAYLIST_EDIT) );
      tool_bar->addAction( m_map_actions.value(PLAYLIST_FAVORITE) );

      if(item->media->isFavorite)
        m_map_actions.value(PLAYLIST_FAVORITE)->setText( tr("Remove from favorites") );
      else
        m_map_actions.value(PLAYLIST_FAVORITE)->setText( tr("Add to favorites") );

      QHBoxLayout * hbox = new QHBoxLayout();
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
      hbox->addWidget(tool_bar);
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

      /* main layout */
      QVBoxLayout * main_box = new QVBoxLayout();
      main_box->setSpacing(0);
      main_box->setContentsMargins(0,0,0,0);
      main_box->addWidget(ui_label);
      main_box->addLayout(hbox);
      main_widget->setLayout(main_box);
    }
    /*-------------------------------------------------*/
    /* StreamType                                      */
    /* ------------------------------------------------*/
    else if( m_items.first()->type() == GraphicsItem::StreamType )
    {
      /* get media data */
      StreamGraphicItem *item = static_cast<StreamGraphicItem*>(m_items.first());
      MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast(item->media);

      QPalette palette;
      palette.setColor(QPalette::WindowText, SETTINGS()->_baseColor);
      
      QLabel* ui_label = new QLabel(this);
      ui_label->setFont(QFont("Arial",14,QFont::Normal));
      ui_label->setPalette(palette);    
      ui_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);    
      ui_label->setText(stream->name);
      
      
      QToolBar *tool_bar = new QToolBar(main_widget);
      tool_bar->setIconSize( QSize( 16, 16 ) );
      tool_bar->addAction( m_map_actions.value(STREAM_PLAY) );
      tool_bar->addAction( m_map_actions.value(STREAM_QUEUE_END) );
      tool_bar->addAction( m_map_actions.value(STREAM_QUEUE_NEW) );
      tool_bar->addAction( m_map_actions.value(STREAM_FAVORITE) );

      if(m_view_mode == VIEW::ViewFavoriteRadio)
        tool_bar->addAction( m_map_actions.value(STREAM_EDIT) );

      
      if(item->media->isFavorite)
        m_map_actions.value(STREAM_FAVORITE)->setText( tr("Remove from favorites") );
      else
        m_map_actions.value(STREAM_FAVORITE)->setText( tr("Add to favorites") );


      QHBoxLayout * hbox = new QHBoxLayout();
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
      hbox->addWidget(tool_bar);
      hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

      /* main layout */
      QVBoxLayout * main_box = new QVBoxLayout();
      main_box->setSpacing(0);
      main_box->setContentsMargins(0,0,0,0);
      main_box->addWidget(ui_label);
      main_box->addLayout(hbox);
      main_widget->setLayout(main_box);
    }

    QWidgetAction* action_widget = new QWidgetAction(this);
    action_widget->setDefaultWidget( main_widget );
    this->addAction( action_widget );  
}


/*******************************************************************************
    slot_actionsTriggered  (PRIVATE SLOTS)
*******************************************************************************/
void GraphicsItemMenu::slot_actionsTriggered()
{
    this->hide();
    QAction *action = qobject_cast<QAction *>(sender());
    if(action)
      emit menu_action_triggered( (ENUM_ACTION_ITEM_MENU)m_map_actions.key(action) );
}


/*******************************************************************************
    eventFilter
*******************************************************************************/
bool GraphicsItemMenu::eventFilter(QObject *obj, QEvent *ev)
{
    //Debug::debug() << "BrowserView eventFilter  obj" << obj;

    if (obj == this)
    {
        return false;
    }
    QGraphicsScene *wid = qobject_cast<QGraphicsScene*>(obj);

    if (wid && (wid == scene) && (ev->type() == QEvent::GraphicsSceneMouseDoubleClick) )
    {
        //Debug::debug() << "###### qgraphics view event GraphicsSceneMouseDoubleClick";
        QGraphicsItem* item = scene->itemAt(static_cast<QGraphicsSceneMouseEvent*>(ev)->scenePos());

        if(item) {
          if(item->type() == GraphicsItem::TrackType)
          {
              TrackGraphicItem *track_item = static_cast<TrackGraphicItem*>(item);
              VirtualPlayqueue::instance()->addTrackAndPlay(static_cast<MEDIA::TrackPtr>(track_item->media));
          }
          else if(item->type() == GraphicsItem::AlbumType)
          {
              AlbumGraphicItem *album_item = static_cast<AlbumGraphicItem*>(item);

              QList<MEDIA::TrackPtr> tracks = LocalTrackModel::instance()->getItemChildrenTracks(album_item->media);
              if(!tracks.isEmpty())
                VirtualPlayqueue::instance()->addTracksAndPlayAt(tracks, 0);
          }
        }
    }

    return QWidget::eventFilter(obj, ev);
}
