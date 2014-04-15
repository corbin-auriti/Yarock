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

#include "browser_view.h"
#include "bottom_container.h"

#include "models/local/local_track_model.h"
#include "models/local/local_playlist_model.h"
#include "models/local/histo_model.h"
#include "models/stream/stream_model.h"

// scene
#include "views/local/local_scene.h"
#include "views/stream/stream_scene.h"
#include "views/context/context_scene.h"
#include "views/filesystem/file_scene.h"
#include "views/settings/settings_scene.h"
#include "views/about/about_scene.h"

// core
#include "threadmanager.h"
#include "engine.h"

// widget 
#include "widgets/main/menumodel.h"
#include "widgets/main/centraltoolbar.h"
#include "widgets/statuswidget.h"

#include "settings.h"
#include "global_actions.h"
#include "debug.h"

/*
********************************************************************************
*                                                                              *
*    BrowserView                                                               *
*                                                                              *
********************************************************************************
*/
BrowserView::BrowserView(QWidget *parent) : QGraphicsView(parent)
{
    //! widget setup
    this->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    this->setAutoFillBackground(false);

    //! GraphicView Setup
    this->setFrameShape(QFrame::NoFrame);
    this->setFrameShadow(QFrame::Plain);

    this->setCacheMode(QGraphicsView::CacheNone);
    this->setDragMode(QGraphicsView::NoDrag);
    this->setRenderHint(QPainter::Antialiasing);
    this->setOptimizationFlags(  QGraphicsView::DontAdjustForAntialiasing
                                   | QGraphicsView::DontClipPainter
                                   | QGraphicsView::DontSavePainterState);
    this->setResizeAnchor(QGraphicsView::NoAnchor);
    this->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    this->setAlignment(Qt::AlignLeft | Qt::AlignTop );

    this->setDragMode(QGraphicsView::NoDrag);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    is_started = false;
    
    /* scroll bar */
    m_scrollbar = this->verticalScrollBar();
    m_scrollbar->setSingleStep(10);
    m_scrollbar->setPageStep(30);
    m_scrollbar->setContextMenuPolicy(Qt::NoContextMenu);
    m_scrollbar->installEventFilter(this);
    m_scrollbar->setMinimum( 0 );

    /* file scene */
    FileScene* file_scene = new FileScene(this);
    m_scenes.insert(VIEW::ViewFileSystem,  file_scene);
    connect(file_scene, SIGNAL(load_directory(const QString&)), this, SLOT(slot_on_load_new_data(const QString&)));

    /* about scene */    
    AboutScene* about_scene = new AboutScene(this);
    m_scenes.insert(VIEW::ViewAbout,  about_scene);
    
    
    /* initialization        */  
    m_browser_params_idx = -1;

    /* connections */
    connect(CentralToolBar::instance(), SIGNAL(explorerFilterActivated(const QString&)),this, SLOT(slot_on_search_changed(const QString&)));
    connect(ThreadManager::instance(),  SIGNAL(modelPopulationFinished(E_MODEL_TYPE)), this, SLOT(slot_on_model_populated(E_MODEL_TYPE)));
    
    
    connect(ACTIONS()->value(BROWSER_PREV), SIGNAL(triggered()), this, SLOT(slot_on_history_prev_activated()));
    connect(ACTIONS()->value(BROWSER_NEXT), SIGNAL(triggered()), this, SLOT(slot_on_history_next_activated()));

    connect(MenuModel::instance(),SIGNAL(modelItemActivated(QModelIndex)),this, SLOT(slot_on_menu_index_changed(QModelIndex)));    
    
    connect (m_scrollbar, SIGNAL(actionTriggered(int)), this, SLOT(slot_check_slider(int)));
      
    connect(ACTIONS()->value(BROWSER_JUMP_TO_ARTIST), SIGNAL(triggered()), SLOT(slot_jump_to_media()));
    connect(ACTIONS()->value(BROWSER_JUMP_TO_ALBUM), SIGNAL(triggered()), SLOT(slot_jump_to_media()));
    connect(ACTIONS()->value(BROWSER_JUMP_TO_TRACK), SIGNAL(triggered()), SLOT(slot_jump_to_media()));
    connect(ACTIONS()->value(BROWSER_JUMP_TO_MEDIA), SIGNAL(triggered()), SLOT(slot_jump_to_media()));
    
    /* connections */
    setViewportMargins(0,0,0,0);
    
    /* bottom widget container */  
    m_bottomWidget = new BottomContainer();

    this->viewport()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding );
    
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    gridLayout->addWidget(this->viewport(),0,0);
    gridLayout->addWidget(m_bottomWidget,1,0);

    this->setLayout(gridLayout);    

    show_bottom_panel(0);
    
    m_menu = 0;
}

/*******************************************************************************
    set scene method
*******************************************************************************/
void BrowserView::setLocalScene(LocalScene *scene)
{
    /* register local scene */
    m_scenes.insert(VIEW::ViewArtist,         scene );
    m_scenes.insert(VIEW::ViewAlbum,          scene );
    m_scenes.insert(VIEW::ViewTrack,          scene );
    m_scenes.insert(VIEW::ViewGenre,          scene );
    m_scenes.insert(VIEW::ViewYear,           scene );
    m_scenes.insert(VIEW::ViewDashBoard,      scene );
    m_scenes.insert(VIEW::ViewHistory,        scene );
    m_scenes.insert(VIEW::ViewPlaylist,       scene );
    m_scenes.insert(VIEW::ViewSmartPlaylist,  scene );
    m_scenes.insert(VIEW::ViewFavorite,       scene );
}

void BrowserView::setStreamScene(StreamScene *scene)
{
    m_scenes.insert(VIEW::ViewDirble,         scene );
    m_scenes.insert(VIEW::ViewShoutCast,      scene );
    m_scenes.insert(VIEW::ViewTuneIn,         scene );
    m_scenes.insert(VIEW::ViewFavoriteRadio,  scene );    
}

void BrowserView::setContextScene(ContextScene *scene)
{
    m_scenes.insert(VIEW::ViewContext,  scene );      
}

void BrowserView::setSettingsScene(SettingsScene *scene)
{
    m_scenes.insert(VIEW::ViewSettings, scene );      
}

/*******************************************************************************
   Save / Restore settings
*******************************************************************************/
void BrowserView::restore_view()
{
   Debug::debug() << "  [BrowserView] restore_view";
  
   BrowserParam param;

   param.mode   = VIEW::Id(SETTINGS()->_viewMode);
   param.filter = QString();
   
   if(param.mode == VIEW::ViewFileSystem)
     param.data   = QVariant( SETTINGS()->_filesystem_path );
   else
     param.data   = QVariant();

   add_history_entry(param);

   switch_view(param);   
  
   is_started = true;
}


/*******************************************************************************
   Changing view slots 
*******************************************************************************/
/* slot triggered when use enter the search field */
void BrowserView::slot_on_search_changed(const QString& filter)
{           
    Debug::debug() << "  [BrowserView] slot_on_search_changed  " << filter;

    BrowserParam current_param;
    if(m_browser_params_idx != -1)
      current_param = m_browser_params.at(m_browser_params_idx);

  
    BrowserParam param = BrowserParam(
           VIEW::Id(SETTINGS()->_viewMode),
           filter,
           current_param.data);
    
    add_history_entry(param);

    switch_view(param);
}

/* slot triggered when action from menu is activated */
void BrowserView::slot_on_menu_index_changed(QModelIndex idx)
{
    if(!idx.isValid()) return;
   
    VIEW::Id m_mode    = VIEW::Id (idx.data(ViewModeRole).toInt());
    QVariant       m_data    = idx.data(FileRole);   
    QString        m_filter  = CentralToolBar::instance()->explorerFilter();

    BrowserParam param = BrowserParam(m_mode,m_filter,m_data);
    
    add_history_entry(param);

    switch_view(param);
}

/* slot used by FileScene to load a directory inside view */
void BrowserView::slot_on_load_new_data(const QString& data)
{
    Debug::debug() << "  [BrowserView] slot_on_load_new_data data" << data;

    BrowserParam param = BrowserParam(
          VIEW::Id(SETTINGS()->_viewMode),
          CentralToolBar::instance()->explorerFilter(),
          QVariant( data ));
    
    add_history_entry( param );
       
    switch_view(param);
}

void BrowserView::active_view(VIEW::Id m, QString f, QVariant d)
{
    BrowserParam param = BrowserParam(m, f, d);

    add_history_entry( param );
    switch_view(param);
}

    
/*******************************************************************************
    slot_on_model_populated
*******************************************************************************/
/* slot used by model to notify update  */
void BrowserView::slot_on_model_populated(E_MODEL_TYPE model)
{
    if(!is_started) return;
    Debug::debug() << "  [BrowserView] slot_on_model_populated ";
    //! no need to update graphics view if the viewmode is not impacted
    BrowserParam param = m_browser_params.at(m_browser_params_idx);
    
    switch(param.mode)
    {
      case VIEW::ViewAlbum     :
      case VIEW::ViewArtist    :
      case VIEW::ViewTrack     :
      case VIEW::ViewGenre     :
      case VIEW::ViewYear      :
      case VIEW::ViewFavorite  :
      case VIEW::ViewDashBoard : 
      case VIEW::ViewHistory   : 
        if(model == MODEL_COLLECTION)
          switch_view(param);
      break;

      case VIEW::ViewPlaylist      :
      case VIEW::ViewSmartPlaylist :
        if(model == MODEL_PLAYLIST)
         switch_view(param);
      break;
      default : break;
    }
}

/*******************************************************************************
    resizeEvent
*******************************************************************************/
void BrowserView::resizeEvent( QResizeEvent * event)
{
    Debug::debug() << "  [BrowserView] resizeEvent";
    if(!is_started) return;
    
    SceneBase * scene = m_scenes.value( VIEW::Id(SETTINGS()->_viewMode) );
    if( scene->isInit() )
      scene->resizeScene();
     
     event->accept();
}

/*******************************************************************************
    eventFilter
*******************************************************************************/
bool BrowserView::eventFilter(QObject *obj, QEvent *ev)
{
    if(!is_started) return false;

    //Debug::debug() << "BrowserView eventFilter  obj" << obj;
    int type = ev->type();
    QWidget *wid = qobject_cast<QWidget*>(obj);

    if (obj == this)
    {
        return false;
    }

    if (wid && (wid == m_scrollbar ))
    {
      if(type == QEvent::Hide || type == QEvent::Show) 
      {
        SceneBase * scene = m_scenes.value( VIEW::Id(SETTINGS()->_viewMode) );
        if( scene->isInit() )
          scene->resizeScene();
      }
    }

    return QWidget::eventFilter(obj, ev);
}

/*******************************************************************************
    switch_view
*******************************************************************************/
void BrowserView::switch_view(BrowserParam& param)
{
    Debug::debug() << "  [BrowserView] switch_view param.mode " << param.mode;
    //Debug::debug() << "  [BrowserView] switch_view param.filter" << param.filter;
    //Debug::debug() << "  [BrowserView] switch_view param.data" << param.data;

    /* save current state */
    SETTINGS()->_viewMode = param.mode;

    CentralToolBar::instance()->showHideGoUp(param.mode);
    
    /* handle scene switch view */
    SceneBase * scene = m_scenes[ param.mode ];

    if( !scene->isInit() )
      scene->initScene();
          
    scene->setMode( param.mode );
    scene->setFilter( param.filter );
    scene->setData( param.data );
    scene->populateScene();
    this->setFocus();
    
    show_bottom_panel( m_scenes[param.mode]->bottomWidget() );

    CentralToolBar::instance()->setExplorerFilterText(param.filter);
    

    switch( VIEW::typeForView(VIEW::Id(param.mode)) ) {
      case VIEW::LOCAL      : setScene(static_cast<LocalScene*>(m_scenes[param.mode]));    break;
      case VIEW::RADIO      : setScene(static_cast<StreamScene*>(m_scenes[param.mode]));   break;
      case VIEW::CONTEXT    : setScene(static_cast<ContextScene*>(m_scenes[param.mode]));  break;
      case VIEW::SETTING    : setScene(static_cast<SettingsScene*>(m_scenes[param.mode])); break;
      case VIEW::FILESYSTEM : setScene(static_cast<FileScene*>(m_scenes[param.mode]));     break;
      case VIEW::ABOUT      : setScene(static_cast<AboutScene*>(m_scenes[param.mode]));    break;
    }

    /* restore scroll position */
    m_scrollbar->setSliderPosition(param.scroll);

    /* status widget update */
    do_statuswidget_update();    
}

    
/*******************************************************************************
    slot_jump_to_media
*******************************************************************************/
void BrowserView::slot_jump_to_media()
{
    //Debug::debug() << "  [BrowserView] slot_jump_to_media";
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action) return;

    MEDIA::MediaPtr media;
    if(action == ACTIONS()->value(BROWSER_JUMP_TO_MEDIA))
    {
      media = qvariant_cast<MEDIA::MediaPtr>( (ACTIONS()->value(BROWSER_JUMP_TO_MEDIA))->data() );
      jump_to_media(media);
    }
    else 
    {
      media = Engine::instance()->playingTrack();
      
      if(!media) return;
      
      if(!media->parent()) return;
      
      MEDIA::MediaPtr album = media->parent();      
      MEDIA::MediaPtr artist = album->parent();
      

      if(action == ACTIONS()->value(BROWSER_JUMP_TO_TRACK))
        jump_to_media(media);
      else if(action == ACTIONS()->value(BROWSER_JUMP_TO_ALBUM))
        jump_to_media(album);      
      else if(action == ACTIONS()->value(BROWSER_JUMP_TO_ARTIST))
        jump_to_media(artist);
    }
}


void BrowserView::jump_to_media(MEDIA::MediaPtr media)
{
    VIEW::Id mode;

    if(!media)
      return;
    
    if(media->type() == TYPE_ARTIST)
      mode = VIEW::ViewArtist;
    else if(media->type() == TYPE_ALBUM)
      mode = VIEW::ViewAlbum;
    else if(media->type() == TYPE_TRACK)
      mode = VIEW::ViewTrack;
    else return;  
      
    CentralToolBar::instance()->setExplorerFilterText("");
    m_scenes[VIEW::ViewArtist]->setFilter("");
    m_scenes[VIEW::ViewTuneIn]->setFilter("");
    m_scenes[VIEW::ViewFileSystem]->setFilter("");
      
    active_view(mode,"", QVariant());

    /* localise item */
    QPoint p = static_cast<LocalScene*>(m_scenes[mode])->get_item_position(media);

    if(!p.isNull()) 
      this->verticalScrollBar()->setSliderPosition( p.y() - 40 );
}


/*******************************************************************************
    Browser History 
*******************************************************************************/
void BrowserView::add_history_entry(BrowserParam& param)
{
    //Debug::debug() << "  [BrowserView] add_history_entry";

    /* save scroll position */
    if(m_browser_params_idx == 0) 
    {
      BrowserParam current_param = m_browser_params[0];
      current_param.scroll = m_scrollbar->sliderPosition();
      scrolls[ current_param.mode ] = current_param.scroll;

      if(scrolls.contains(param.mode))
        param.scroll = scrolls.value( param.mode );      
    }
    
    /* nouveau déclenchement --> on supprime les next */
    for(int i = 0; i < m_browser_params_idx; i++)
      m_browser_params.removeAt(i);

    m_browser_params.prepend(param);
    m_browser_params_idx = 0;

    /* limite de la taille de l'historique de navigation */
    if(m_browser_params.size() >= 30 && m_browser_params_idx != m_browser_params.size() -1)
      m_browser_params.takeLast();
    
    ACTIONS()->value(BROWSER_PREV)->setEnabled(m_browser_params_idx < m_browser_params.size() -1);
    ACTIONS()->value(BROWSER_NEXT)->setEnabled(m_browser_params_idx > 0);    
} 


void BrowserView::slot_on_history_prev_activated()
{
    if(m_browser_params_idx < m_browser_params.size() -1)
    {
      m_browser_params_idx++;

      ACTIONS()->value(BROWSER_PREV)->setEnabled(m_browser_params_idx < m_browser_params.size() -1);
      ACTIONS()->value(BROWSER_NEXT)->setEnabled(m_browser_params_idx > 0);

      BrowserParam param = m_browser_params.at(m_browser_params_idx);
      switch_view(param);
    }
}


void BrowserView::slot_on_history_next_activated()
{
    if(m_browser_params_idx > 0)
    {
      m_browser_params_idx--;

      ACTIONS()->value(BROWSER_PREV)->setEnabled(m_browser_params_idx < m_browser_params.size() -1);
      ACTIONS()->value(BROWSER_NEXT)->setEnabled(m_browser_params_idx > 0);

      BrowserParam param = m_browser_params.at(m_browser_params_idx);
      switch_view(param);      
    }
}


/*******************************************************************************
    show/hide bottom panel
*******************************************************************************/
void BrowserView::show_bottom_panel(QWidget* w)
{
    //Debug::debug() << "  [BrowserView] show_bottom_panel";

    m_bottomWidget->setContent( w );
   
    if(!w) 
    {
      m_bottomWidget->hide();
      setViewportMargins(0,0,0,0);
    }
    else
    {
      m_bottomWidget->show();  

      m_bottomWidget->resize(m_bottomWidget->width(),w->sizeHint().height() + 4);
      setViewportMargins(0,0,0,w->sizeHint().height() + 4);
    }
}

/*******************************************************************************
    contextMenuEvent
*******************************************************************************/
void BrowserView::contextMenuEvent ( QContextMenuEvent * event )
{
    //Debug::debug() << "  [BrowserView] contextMenuEvent ";
    SceneBase * scene = m_scenes[ VIEW::Id(SETTINGS()->_viewMode) ];
    
    if(scene->mouseGrabberItem()) 
    {
        QGraphicsView::contextMenuEvent(event) ;
    }
    else 
    {
       if(!m_menu)
         m_menu = new QMenu(this);
       
       m_menu->clear();
       m_menu->addActions(scene->actions());
       m_menu->addSeparator();
       m_menu->addAction(ACTIONS()->value(BROWSER_JUMP_TO_ARTIST));
       m_menu->addAction(ACTIONS()->value(BROWSER_JUMP_TO_ALBUM));
       m_menu->addAction(ACTIONS()->value(BROWSER_JUMP_TO_TRACK));       
       
       m_menu->popup(mapToGlobal(event->pos()));
       
       event->accept();
    }
}


/*******************************************************************************
    keyPressEvent
*******************************************************************************/
void BrowserView::keyPressEvent ( QKeyEvent * event )
{
    //Debug::debug() << "  [BrowserView] keyPressEvent " << event->key();
    switch(event->key())
    {
      case Qt::Key_Home  :
        if( Qt::ControlModifier == QApplication::keyboardModifiers() ) {
            this->verticalScrollBar()->setSliderPosition(0);
            event->accept();
        }
        break;

      case Qt::Key_End  :
        if( Qt::ControlModifier == QApplication::keyboardModifiers() ) {
            this->verticalScrollBar()->setSliderPosition(this->verticalScrollBar()->maximum());
            event->accept();
        }
        break;

      case Qt::Key_Left  :
      case Qt::Key_Right :
        event->ignore();
        break;      
      
      default : QGraphicsView::keyPressEvent(event);break;
    }
}

/*******************************************************************************
    updateStatusWidget
*******************************************************************************/
void BrowserView::do_statuswidget_update()
{
    //Debug::debug() << "  [BrowserView] do_statuswidget_update";

    const int collectionSize = static_cast<LocalScene*>(m_scenes[VIEW::ViewArtist])->elementCount();
    const int radioSize      = static_cast<StreamScene*>(m_scenes[VIEW::ViewTuneIn])->elementCount();

    QString     text;

    switch( SETTINGS()->_viewMode )
    {
      case (VIEW::ViewContext)  : text = tr("Context");break;
      case (VIEW::ViewHistory)  : text = tr("History");break;
      case (VIEW::ViewAlbum)    : text = QString(tr("Collection : <b>%1</b> albums")).arg(QString::number(collectionSize)); break;
      case (VIEW::ViewArtist)   : text = QString(tr("Collection : <b>%1</b> artist")).arg(QString::number(collectionSize));break;
      case (VIEW::ViewTrack)    : text = QString(tr("Collection : <b>%1</b> tracks")).arg(QString::number(collectionSize));break;
      case (VIEW::ViewGenre)    : text = QString(tr("Collection : <b>%1</b> styles")).arg(QString::number(collectionSize));break;
      case (VIEW::ViewYear)     : text = QString(tr("Collection : <b>%1</b> years")).arg(QString::number(collectionSize));break;
      case (VIEW::ViewFavorite) : text = QString(tr("Collection : <b>%1</b> favorite item")).arg(QString::number(collectionSize));break;

      case (VIEW::ViewPlaylist) : 
      case (VIEW::ViewSmartPlaylist) :
        text = QString(tr("Playlist : <b>%1</b> playlists")).arg(QString::number(collectionSize));
        break;
      
      case (VIEW::ViewDirble)         :
      case (VIEW::ViewShoutCast)      :
      case (VIEW::ViewTuneIn)         :
      case (VIEW::ViewFavoriteRadio)  :
         text = QString(tr("Radio : <b>%1</b> streams")).arg(QString::number(radioSize));
         break;
      default: text = "";break;
    }

    if(!text.isEmpty())
      StatusWidget::instance()->startShortMessage(text, STATUS::TYPE_INFO, 2500);
}


/*******************************************************************************
    slot_check_slider
*******************************************************************************/
void BrowserView::slot_check_slider(int action)
{
    //Debug::debug() << "  [BrowserView]slot_showContextMenu slot_check_slider";
    if( action == QAbstractSlider::SliderPageStepAdd ||
        action == QAbstractSlider::SliderPageStepSub )
    {
        
      QList<QGraphicsItem*> categories;
      foreach(QGraphicsItem* gItem, this->scene()->items() )
        if(gItem->type() == GraphicsItem::CategoryType)
          categories << gItem;

      if( categories.isEmpty() ) return;
        
      int top = this->verticalScrollBar()->sliderPosition();
                  
      /* ------------ navigate go up ---------------- */
      if( action == QAbstractSlider::SliderPageStepSub) 
      {      
          for(int i=categories.size()-1; i>=0;i--)
          {
            QPoint itemPoint = categories.at(i)->scenePos().toPoint();
    
            if(top > itemPoint.y()) {
              this->verticalScrollBar()->setSliderPosition(itemPoint.y());
              break;
            }
          }
      }
      /* ------------ navigate go down ---------------*/      
      else
      {
          foreach (QGraphicsItem *item, categories)
          {
            QPoint itemPoint = (item->scenePos()).toPoint();

            if(top < itemPoint.y()) {
              this->verticalScrollBar()->setSliderPosition(itemPoint.y());
              break;
            }
          }      
      }
    }
}
