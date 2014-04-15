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

#include "settings_scene.h"
#include "settings_widget.h"

#include "threadmanager.h"

#include "views/item_button.h"
#include "views/item_common.h"
#include "views/item_menu.h"

#include "debug.h"

// Qt
#include <QGraphicsProxyWidget>
#include <QtCore>

/*
********************************************************************************
*                                                                              *
*    Class SettingsScene                                                       *
*                                                                              *
********************************************************************************
*/
SettingsScene::SettingsScene(QWidget* parent) : SceneBase(parent)
{
}

/*******************************************************************************
    initScene
*******************************************************************************/
void SettingsScene::initScene()
{
    // create pages
    m_pages[SETTINGS::GENERAL]   = new PageGeneral(parentView());
    m_pages[SETTINGS::PLAYER]    = new PagePlayer(parentView());
    m_pages[SETTINGS::LIBRARY]   = new PageLibrary(parentView());
    m_pages[SETTINGS::SHORTCUT]  = new PageShortcut(parentView());
    m_pages[SETTINGS::SCROBBLER] = new PageScrobbler(parentView());
    m_pages[SETTINGS::SONGINFO]  = new PageSongInfo(parentView());

    connect(m_pages[SETTINGS::GENERAL], SIGNAL(layout_changed()), this, SLOT(populateScene()));
    connect(m_pages[SETTINGS::PLAYER], SIGNAL(layout_changed()), this, SLOT(populateScene()));
    connect(m_pages[SETTINGS::LIBRARY], SIGNAL(layout_changed()), this, SLOT(populateScene()));
    connect(m_pages[SETTINGS::SHORTCUT], SIGNAL(layout_changed()), this, SLOT(populateScene()));
    connect(m_pages[SETTINGS::SCROBBLER], SIGNAL(layout_changed()), this, SLOT(populateScene()));
    connect(m_pages[SETTINGS::SONGINFO], SIGNAL(layout_changed()), this, SLOT(populateScene()));

    m_header = new HeaderItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    m_header->setText( tr("Settings") );

    /* bottom widget */
    m_bottomWidget  = new BottomWidget();
    connect(m_bottomWidget, SIGNAL(save_clicked()), this, SLOT(slot_apply_settings()));
    connect(m_bottomWidget, SIGNAL(cancel_clicked()), this, SLOT(slot_cancel_settings()));
    
    
    this->addItem(m_header);
    this->addItem(m_pages.value(SETTINGS::GENERAL));
    this->addItem(m_pages.value(SETTINGS::PLAYER));
    this->addItem(m_pages.value(SETTINGS::LIBRARY));
    this->addItem(m_pages.value(SETTINGS::SHORTCUT));
    this->addItem(m_pages.value(SETTINGS::SCROBBLER));
    this->addItem(m_pages.value(SETTINGS::SONGINFO));

    /* first init => always restore settings */
    restore_settings();

    /* il faut interdire la modification des parametres de collection si un database builder est en cours */
    connect(ThreadManager::instance(), SIGNAL(dbBuildStart()), this, SLOT(slot_dbBuilder_stateChange()));
    connect(ThreadManager::instance(), SIGNAL(dbBuildFinished()), this, SLOT(slot_dbBuilder_stateChange()));
    
    setInit(true);
}


/*******************************************************************************
    slot_dbBuilder_stateChange
*******************************************************************************/
void SettingsScene::slot_dbBuilder_stateChange()
{
    //Debug::debug() << "   [SettingsScene] slot_dbBuilder_stateChange";
    m_pages[SETTINGS::LIBRARY]->setEnabled( !ThreadManager::instance()->isDbRunning() );
}


/*******************************************************************************
    resizeScene
*******************************************************************************/
void SettingsScene::resizeScene()
{
    populateScene();
} 

/*******************************************************************************
    populateScene
*******************************************************************************/
void SettingsScene::populateScene()
{
    Debug::debug() << "   [SettingsScene] populateScene";    
 
    /* header item */    
    int Ypos = 0;
    m_header->setPos(0, 5);
    Ypos = Ypos + 40;

    /* SETTINGS::GENERAL */ 
    m_pages.value(SETTINGS::GENERAL)->setPos(0,Ypos);

    static_cast<PageGeneral*>(m_pages[SETTINGS::GENERAL])->doLayout();

    Ypos += static_cast<PageGeneral*>(m_pages[SETTINGS::GENERAL])->size().height();

    /* SETTINGS::PLAYER */ 
    m_pages.value(SETTINGS::PLAYER)->setPos(0,Ypos);
    
    static_cast<PagePlayer*>(m_pages[SETTINGS::PLAYER])->doLayout();

    Ypos += static_cast<PagePlayer*>(m_pages[SETTINGS::PLAYER])->size().height();

    /* SETTINGS::LIBRARY */ 
    m_pages.value(SETTINGS::LIBRARY)->setPos(0,Ypos);

    static_cast<PageLibrary*>(m_pages[SETTINGS::LIBRARY])->doLayout();
    
    Ypos += static_cast<PageLibrary*>(m_pages[SETTINGS::LIBRARY])->size().height();

    /* SETTINGS::SHORTCUT */ 
    m_pages.value(SETTINGS::SHORTCUT)->setPos(0,Ypos);

    static_cast<PageShortcut*>(m_pages[SETTINGS::SHORTCUT])->doLayout();
    
    Ypos += static_cast<PageShortcut*>(m_pages[SETTINGS::SHORTCUT])->size().height();

    /* SETTINGS::SONGINFO */ 
    m_pages.value(SETTINGS::SONGINFO)->setPos(0,Ypos);

    static_cast<PageSongInfo*>(m_pages[SETTINGS::SONGINFO])->doLayout();
    
    Ypos += static_cast<PageSongInfo*>(m_pages[SETTINGS::SONGINFO])->size().height();
    
    /* SETTINGS::SCROBBLER */ 
    m_pages.value(SETTINGS::SCROBBLER)->setPos(0,Ypos);

    static_cast<PageScrobbler*>(m_pages[SETTINGS::SCROBBLER])->doLayout();
    
    /* ajust SceneRect */
    setSceneRect ( itemsBoundingRect().adjusted(0, -10, 0, 40) );
}


/*******************************************************************************
    setData (set settings page)
*******************************************************************************/
void SettingsScene::setData(const QVariant& data)
{
    if(data.isNull())
      return;
  
    SETTINGS::SETTINGS_PAGES page = SETTINGS::SETTINGS_PAGES(data.toInt());

    static_cast<PageGeneral*>(m_pages[SETTINGS::GENERAL])->setContentVisible( page == SETTINGS::GENERAL );
    static_cast<PagePlayer*>(m_pages[SETTINGS::PLAYER])->setContentVisible( page == SETTINGS::PLAYER );
    static_cast<PageLibrary*>(m_pages[SETTINGS::LIBRARY])->setContentVisible( page == SETTINGS::LIBRARY );
    static_cast<PageShortcut*>(m_pages[SETTINGS::SHORTCUT])->setContentVisible( page == SETTINGS::SHORTCUT );
    static_cast<PageScrobbler*>(m_pages[SETTINGS::SCROBBLER])->setContentVisible( page == SETTINGS::SCROBBLER );
    static_cast<PageSongInfo*>(m_pages[SETTINGS::SONGINFO])->setContentVisible( page == SETTINGS::SONGINFO );
}


/*******************************************************************************
    slot_cancel_settings
*******************************************************************************/
void SettingsScene::slot_cancel_settings()
{
    //Debug::debug() << "   [SettingsScene] slot_cancel_settings";
    restore_settings();
}

/*******************************************************************************
    restore_settings
*******************************************************************************/
void SettingsScene::restore_settings()
{
    Debug::debug() << "   [SettingsScene] restore_settings";
    static_cast<PageGeneral*>(m_pages[SETTINGS::GENERAL])->restoreSettings();
    static_cast<PagePlayer*>(m_pages[SETTINGS::PLAYER])->restoreSettings();
    static_cast<PageLibrary*>(m_pages[SETTINGS::LIBRARY])->restoreSettings();
    static_cast<PageShortcut*>(m_pages[SETTINGS::SHORTCUT])->restoreSettings();
    static_cast<PageScrobbler*>(m_pages[SETTINGS::SCROBBLER])->restoreSettings();
    static_cast<PageSongInfo*>(m_pages[SETTINGS::SONGINFO])->restoreSettings();


    m_result.isSystrayChanged       =  false;
    m_result.isDbusChanged          =  false;
    m_result.isMprisChanged         =  false;
    m_result.isLibraryChanged       =  false;
    m_result.isViewChanged          =  false;
    m_result.isShorcutChanged       =  false;
    m_result.isScrobblerChanged     =  false;
}

/*******************************************************************************
    slot_apply_settings
*******************************************************************************/
void SettingsScene::slot_apply_settings()
{
    Debug::debug() << "   [SettingsScene] slot_apply_settings";

    //! save setting
    m_result.isSystrayChanged       =  static_cast<PageGeneral*>(m_pages[SETTINGS::GENERAL])->isSystrayChanged();
    m_result.isDbusChanged          =  static_cast<PageGeneral*>(m_pages[SETTINGS::GENERAL])->isDbusChanged();
    m_result.isMprisChanged         =  static_cast<PageGeneral*>(m_pages[SETTINGS::GENERAL])->isMprisChanged();
    m_result.isLibraryChanged       =  static_cast<PageLibrary*>(m_pages[SETTINGS::LIBRARY])->isLibraryChanged();
    m_result.isViewChanged          =  static_cast<PageLibrary*>(m_pages[SETTINGS::LIBRARY])->isViewChanged();
    m_result.isShorcutChanged       =  static_cast<PageShortcut*>(m_pages[SETTINGS::SHORTCUT])->isChanged();
    m_result.isScrobblerChanged     =  static_cast<PageScrobbler*>(m_pages[SETTINGS::SCROBBLER])->isChanged();

    static_cast<PageGeneral*>(m_pages[SETTINGS::GENERAL])->saveSettings();
    static_cast<PagePlayer*>(m_pages[SETTINGS::PLAYER])->saveSettings();
    static_cast<PageLibrary*>(m_pages[SETTINGS::LIBRARY])->saveSettings();
    static_cast<PageShortcut*>(m_pages[SETTINGS::SHORTCUT])->saveSettings();
    static_cast<PageScrobbler*>(m_pages[SETTINGS::SCROBBLER])->saveSettings();
    static_cast<PageSongInfo*>(m_pages[SETTINGS::SONGINFO])->saveSettings();

    emit settings_saved();
}

