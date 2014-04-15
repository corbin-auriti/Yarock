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

#include "settings_widget.h"

#include "views/item_common.h"
#include "views/item_button.h"

#include "global_shortcuts.h"
#include "core/player/engine.h"

#include "online/lastfm.h"
#include "infosystem/services/service_lyrics.h"


#include "utilities.h"
#include "settings.h"
#include "filedialog.h"
#include "debug.h"

#include <QColorDialog>

/*
********************************************************************************
*                                                                              *
*    Class PageGeneral                                                         *
*                                                                              *
********************************************************************************
*/
PageGeneral::PageGeneral(QWidget* parentView) : QGraphicsWidget(0)
{
    m_parent       = parentView;
    isOpen         = true;

    //! create Gui
    createGui();
}


//! ----------- createGui ------------------------------------------------------
void PageGeneral::createGui()
{
    m_title   = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->m_name   = tr("General");

    
    m_button = new ButtonItem();
    m_button->setPos(0,0);
    m_button->setColor(QApplication::palette().color(QPalette::Base));
    m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
    m_button->setParentItem(this);

    connect(m_button, SIGNAL(clicked()), this, SLOT(slot_on_titlebutton_clicked()));
    
    
    // main widget
    QWidget* main_widget = new QWidget();

    main_widget->setAttribute(Qt::WA_NoBackground, true);
    main_widget->setAutoFillBackground(true);

    QVBoxLayout* vl0 = new QVBoxLayout(main_widget);
    
    /*-------------------------------------------------*/
    /* General settings                                */
    /* ------------------------------------------------*/
    //! dbus & mpris parameter
    ui_check_enableMpris = new QCheckBox(main_widget);
    ui_check_enableMpris->setText(tr("Enable Mpris"));

    ui_check_enableDbus = new QCheckBox(main_widget);
    ui_check_enableDbus->setText( tr("Enable notification") );

    //! systray parameter
    ui_check_systray = new QCheckBox(main_widget);
    ui_check_systray->setText(tr("Minimize application to systray"));

    ui_check_hideAtStartup = new QCheckBox(main_widget);
    ui_check_hideAtStartup->setText(tr("Hide window at startup"));

    ui_color_button = new QPushButton(main_widget);
    ui_color_button->setMinimumWidth(150);
    ui_color_button->setMaximumWidth(150);
    ui_color_button->setFlat(false);
    
    QPalette  pal;
    pal.setColor( QPalette::Active, QPalette::Button, SETTINGS()->_baseColor );
    pal.setColor( QPalette::Inactive, QPalette::Button, SETTINGS()->_baseColor );
    ui_color_button->setPalette(pal);
    
    ui_color_button->setText(tr("Choose color"));
    connect(this->ui_color_button, SIGNAL(clicked()), this, SLOT(slot_color_button_clicked()));
    
    /*-------------------------------------------------*/
    /* Graphical Interface settings                    */
    /* ------------------------------------------------*/
    //! layout
    vl0->addWidget(ui_check_enableMpris);
    vl0->addWidget(ui_check_enableDbus);
    vl0->addWidget(ui_check_systray);
    vl0->addWidget(ui_check_hideAtStartup);
    vl0->addWidget(ui_color_button);

    // proxy widget
    proxy_widget = new QGraphicsProxyWidget( this );
    proxy_widget->setWidget( main_widget );
    proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    m_title->setParentItem(this);
    m_title->setPos(0,0);
}

void PageGeneral::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
    //Debug::debug() << "## PageGeneral::resizeEvent";
    doLayout();
}

void PageGeneral::doLayout()
{
    m_button->setPos(qobject_cast<QGraphicsView*> (m_parent)->viewport()->width()-40,0);
    proxy_widget->setPos(50,30);
}


QSize PageGeneral::size()
{
  if(proxy_widget->isVisible())
    return QSize(
      proxy_widget->geometry().size().width(),
      proxy_widget->geometry().size().height() + 50
      );
  else
    return QSize( proxy_widget->geometry().size().width(), 50);
}


//! ----------- saveSettings ---------------------------------------------------
void PageGeneral::saveSettings()
{
    Debug::debug() << "PageGeneral::saveSettings";
    SETTINGS()->_useTrayIcon         = this->ui_check_systray->isChecked();
    SETTINGS()->_hideAtStartup       = this->ui_check_hideAtStartup->isChecked();
    SETTINGS()->_useDbusNotification = this->ui_check_enableDbus->isChecked();
    SETTINGS()->_useMpris            = this->ui_check_enableMpris->isChecked();
}

//! ----------- restoreSettings ------------------------------------------------
void PageGeneral::restoreSettings()
{
    this->ui_check_systray->setChecked( SETTINGS()->_useTrayIcon );
    this->ui_check_hideAtStartup->setChecked( SETTINGS()->_hideAtStartup );
    this->ui_check_enableDbus->setChecked( SETTINGS()->_useDbusNotification );
    this->ui_check_enableMpris->setChecked( SETTINGS()->_useMpris );
}


bool PageGeneral::isSystrayChanged()
{
    return SETTINGS()->_useTrayIcon != ui_check_systray->isChecked();
}

bool PageGeneral::isDbusChanged()
{
    return SETTINGS()->_useDbusNotification != ui_check_enableDbus->isChecked();
}

bool PageGeneral::isMprisChanged()
{
    return SETTINGS()->_useMpris != ui_check_enableMpris->isChecked();
}

void PageGeneral::slot_color_button_clicked()
{
    QColor color = QColorDialog::getColor(SETTINGS()->_baseColor, m_parent);
    if (color.isValid())
    {
       SETTINGS()->_baseColor = color;
      
       QPalette  pal;
       pal.setColor( QPalette::Active, QPalette::Button, color );
       pal.setColor( QPalette::Inactive, QPalette::Button, color );
       ui_color_button->setPalette(pal);
    } 
}

void PageGeneral::setContentVisible(bool b)
{
    if(isOpen != b)
      slot_on_titlebutton_clicked();
}


void PageGeneral::slot_on_titlebutton_clicked()
{
    if(isOpen) {
      proxy_widget->hide();
      m_button->setPixmap(QPixmap(":/images/add_32x32.png"));
      m_button->update();
      isOpen = false;
    }
    else 
    {
      proxy_widget->show();
      m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
      m_button->update();
      isOpen = true;
    }
    
    emit layout_changed();
}

/*
********************************************************************************
*                                                                              *
*    Class PagePlayer                                                          *
*                                                                              *
********************************************************************************
*/
PagePlayer::PagePlayer(QWidget* parentView) : QGraphicsWidget(0)
{
    m_parent       = parentView;
    isOpen         = true;
    
    //! create Gui
    createGui();
}

//! ----------- createGui ------------------------------------------------------
void PagePlayer::createGui()
{
    m_title           = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->m_name   = tr("Player settings");

    m_button = new ButtonItem();
    m_button->setPos(0,0);
    m_button->setColor(QApplication::palette().color(QPalette::Base));
    m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
    m_button->setParentItem(this);

    connect(m_button, SIGNAL(clicked()), this, SLOT(slot_on_titlebutton_clicked()));    
    
    // main widget
    QWidget* main_widget = new QWidget();

    main_widget->setAttribute(Qt::WA_NoBackground, true);
    main_widget->setAutoFillBackground(true);

    /*-------------------------------------------------*/
    /* Player settings                                 */
    /* ------------------------------------------------*/
    
    ui_comboEngine = new QComboBox(main_widget);
    ui_comboEngine->setMaximumWidth(150);
    ui_comboEngine->setMinimumWidth(150);
    ui_comboEngine->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    
#ifdef ENABLE_VLC
    ui_comboEngine->addItem("vlc", QVariant::fromValue((int)ENGINE::VLC));
#endif
    
#ifdef ENABLE_PHONON
    ui_comboEngine->addItem("phonon",  QVariant::fromValue((int)ENGINE::PHONON));   
#endif
    ui_comboEngine->addItem("null engine",  QVariant::fromValue((int)ENGINE::NO_ENGINE));
    
    
    ui_stopOnPlayqueueClear = new QCheckBox(main_widget);
    ui_stopOnPlayqueueClear->setText(tr("Stop playing on playqueue clear"));

    ui_restartPlayingAtStartup = new QCheckBox(main_widget);
    ui_restartPlayingAtStartup->setText(tr("Restart playing at startup"));

    ui_restorePlayqueue = new QCheckBox(main_widget);
    ui_restorePlayqueue->setText(tr("Restore last playqueue content at startup"));
    
    ui_enable_replaygain = new QCheckBox(main_widget);
    ui_enable_replaygain->setText(tr("Use ReplayGain"));    
    connect(this->ui_enable_replaygain, SIGNAL(clicked()), this, SLOT(slot_enable_replaygain()));


    ui_comboRGMode = new QComboBox(main_widget);
    
    ui_comboRGMode->setMaximumWidth(150);
    ui_comboRGMode->setMinimumWidth(150);
    ui_comboRGMode->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    ui_comboRGMode->addItem("radio");   
    ui_comboRGMode->addItem("album");   
    
    QLabel *lbl1 = new QLabel(tr("Engine"), main_widget);
    lbl1->setFont(QFont("Arial",10,QFont::Bold));

    QLabel *lbl2 = new QLabel(tr("Replaygain"), main_widget);
    lbl2->setFont(QFont("Arial",10,QFont::Bold));
 
    QLabel *lbl3 = new QLabel(tr("Others"), main_widget);
    lbl3->setFont(QFont("Arial",10,QFont::Bold));
    
    QVBoxLayout* vl0 = new QVBoxLayout(main_widget);
    vl0->addWidget( lbl1 );
    vl0->addWidget( ui_comboEngine );
    vl0->addItem( new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding) );
    vl0->addWidget( lbl2 );
    vl0->addWidget(ui_enable_replaygain);
    vl0->addWidget(ui_comboRGMode);
    vl0->addItem( new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding) );
    vl0->addWidget( lbl3 );
    vl0->addWidget(ui_stopOnPlayqueueClear);
    vl0->addWidget(ui_restartPlayingAtStartup);
    vl0->addWidget(ui_restorePlayqueue);
    

    // proxy widget
    proxy_widget = new QGraphicsProxyWidget( this );
    proxy_widget->setWidget( main_widget );
    proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );


    m_title->setParentItem(this);
    m_title->setPos(0,0);
}

void PagePlayer::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
    //Debug::debug() << "## PagePlayer::resizeEvent";
    doLayout();
}

void PagePlayer::doLayout()
{
    //Debug::debug() << "## PagePlayer::doLayout";
    m_button->setPos(qobject_cast<QGraphicsView*> (m_parent)->viewport()->width()-40,0);
    proxy_widget->setPos(50,30);
}


QSize PagePlayer::size()
{
  if(proxy_widget->isVisible())
    return QSize(
      proxy_widget->geometry().size().width(),
      proxy_widget->geometry().size().height() + 50
      );
  else
    return QSize( proxy_widget->geometry().size().width(), 50);
}

void PagePlayer::saveSettings()
{
    Debug::debug() << "PagePlayer::saveSettings";

    SETTINGS()->_stopOnPlayqueueClear    = this->ui_stopOnPlayqueueClear->isChecked();
    SETTINGS()->_restartPlayingAtStartup = this->ui_restartPlayingAtStartup->isChecked();
    SETTINGS()->_restorePlayqueue        = this->ui_restorePlayqueue->isChecked();
    
    if(!ui_enable_replaygain->isChecked())
      SETTINGS()->_replaygain = 0;
    else if(ui_comboRGMode->currentIndex() == 0)
       SETTINGS()->_replaygain = 1;
    else
       SETTINGS()->_replaygain = 2;
    
    QVariant v  = ui_comboEngine->itemData(ui_comboEngine->currentIndex());
    SETTINGS()->_engine = v.toInt();
}

void PagePlayer::restoreSettings()
{
    this->ui_stopOnPlayqueueClear->setChecked( SETTINGS()->_stopOnPlayqueueClear );
    this->ui_restartPlayingAtStartup->setChecked( SETTINGS()->_restartPlayingAtStartup );
    this->ui_restorePlayqueue->setChecked( SETTINGS()->_restorePlayqueue );

    this->ui_enable_replaygain->setChecked( SETTINGS()->_replaygain != 0 );
    this->ui_comboRGMode->setCurrentIndex(SETTINGS()->_replaygain == 1 ? 0 : 1);
    this->ui_comboRGMode->setEnabled(ui_enable_replaygain->isChecked());

    int selectedIdx = -1; 
    switch(SETTINGS()->_engine) {
      case ENGINE::NO_ENGINE: selectedIdx = ui_comboEngine->findText("null engine");break;
      case ENGINE::VLC:       selectedIdx = ui_comboEngine->findText("vlc");break;
      case ENGINE::PHONON:    selectedIdx = ui_comboEngine->findText("phonon");break;
      default:break;
    }
    ui_comboEngine->setCurrentIndex(selectedIdx);
}

void PagePlayer::slot_enable_replaygain()
{
    ui_comboRGMode->setEnabled(ui_enable_replaygain->isChecked());
}


void PagePlayer::setContentVisible(bool b)
{
    if(isOpen != b)
      slot_on_titlebutton_clicked();
}

void PagePlayer::slot_on_titlebutton_clicked()
{
    if(isOpen) {
      proxy_widget->hide();
      m_button->setPixmap(QPixmap(":/images/add_32x32.png"));
      m_button->update();
      isOpen = false;
    }
    else 
    {
      proxy_widget->show();
      m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
      m_button->update();
      isOpen = true;
    }
    
    emit layout_changed();
}


/*
********************************************************************************
*                                                                              *
*    Class PageLibrary                                                         *
*                                                                              *
********************************************************************************
*/
PageLibrary::PageLibrary(QWidget* parentView) : QGraphicsWidget(0)
{
    m_parent       = parentView;
    isOpen         = true;
    
    //! create Gui
    createGui();

    //! connection
    connect(this->ui_add_path_button, SIGNAL(clicked()), this, SLOT(on_addPathButton_clicked()));
    connect(this->ui_remove_path_button, SIGNAL(clicked()), this, SLOT(on_removePathButton_clicked()));
    connect(this->ui_remove_all_button, SIGNAL(clicked()), this, SLOT(on_removeAllPathsButton_clicked()));

    connect(this->ui_auto_update, SIGNAL(stateChanged (int)), this, SLOT(slot_oncheckbox_clicked()));
    connect(this->ui_search_cover, SIGNAL(stateChanged (int)), this, SLOT(slot_oncheckbox_clicked()));
    connect(this->ui_group_albums, SIGNAL(stateChanged (int)), this, SLOT(slot_oncheckbox_clicked()));
    

    connect(this->ui_choose_db, SIGNAL(currentIndexChanged(QString)), SLOT(loadDatabaseParam(QString)));
    connect(this->ui_db_new_button, SIGNAL(clicked()), this, SLOT(newDatabaseParam()));
    connect(this->ui_db_del_button, SIGNAL(clicked()), this, SLOT(delDatabaseParam()));
    connect(this->ui_db_rename_button, SIGNAL(clicked()), this, SLOT(renameDatabaseParam()));

    connect(this->ui_enable_multiDb, SIGNAL(clicked()), this, SLOT(enableMultiDatabase()));


    enableMultiDatabase();
}

//! ----------- createGui ------------------------------------------------------
void PageLibrary::createGui()
{
    m_title           = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->m_name   = tr("Library settings");

    
    m_button = new ButtonItem();
    m_button->setPos(0,0);
    m_button->setColor(QApplication::palette().color(QPalette::Base));
    m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
    m_button->setParentItem(this);

    connect(m_button, SIGNAL(clicked()), this, SLOT(slot_on_titlebutton_clicked()));

    
    // main widget
    QWidget* main_widget = new QWidget();

    main_widget->setAttribute(Qt::WA_NoBackground, true);
    main_widget->setAutoFillBackground(true);

    /*-------------------------------------------------*/
    /* Libray settings                                 */
    /* ------------------------------------------------*/
    QVBoxLayout* verticalLayout = new QVBoxLayout(main_widget);

    //! label
    QLabel *lbl1 = new QLabel(tr("Database Name"), main_widget);
    lbl1->setFont(QFont("Arial",10,QFont::Bold));

    //! Check box --> enable Multi Databases
    ui_enable_multiDb = new QCheckBox(main_widget);
    ui_enable_multiDb->setText(tr("Enable Multi Database support"));

    //! horizontal Layout (database list + save + delete)
    ui_choose_db = new QComboBox(main_widget);
    
    ui_choose_db->setMaximumWidth(150);
    ui_choose_db->setMinimumWidth(150);
    ui_choose_db->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    ui_db_new_button = new QToolButton(main_widget);
    ui_db_new_button->setIcon(QIcon(":/images/add_32x32.png"));
    ui_db_new_button->setToolTip(tr("New database"));

    ui_db_del_button = new QToolButton(main_widget);
    ui_db_del_button->setIcon(QIcon::fromTheme("edit-delete"));
    ui_db_del_button->setToolTip(tr("Delete database"));

    ui_db_rename_button = new QToolButton(main_widget);
    ui_db_rename_button->setIcon(QIcon(":/images/rename-48x48.png"));
    ui_db_rename_button->setToolTip(tr("Rename database"));

    QHBoxLayout *hl1 = new QHBoxLayout();
    hl1->addWidget(ui_choose_db);
    hl1->addWidget(ui_db_new_button);
    hl1->addWidget(ui_db_del_button);
    hl1->addWidget(ui_db_rename_button);
    hl1->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    //! label
    QLabel *lbl2 = new QLabel(tr("Database Properties"), main_widget);
    lbl2->setFont(QFont("Arial",10,QFont::Bold));

    //! horizontal Layout (source path list + add + delete + delete all)
    ui_paths_list = new QListWidget(main_widget);
    ui_paths_list->setMinimumHeight(180);

    QHBoxLayout* hl2 = new QHBoxLayout();
    hl2->addWidget(ui_paths_list);

    //! vertical Layout (button Remove/Remove All/Add)
    ui_add_path_button = new QPushButton(main_widget);
    ui_add_path_button->setText(tr("Add ..."));

    ui_remove_path_button = new QPushButton(main_widget);
    ui_remove_path_button->setText(tr("Remove"));

    ui_remove_all_button = new QPushButton(main_widget);
    ui_remove_all_button->setText(tr("Remove all"));

    QVBoxLayout *vl2 = new QVBoxLayout();
    vl2->addWidget(ui_add_path_button);
    vl2->addWidget(ui_remove_path_button);
    vl2->addWidget(ui_remove_all_button);
    vl2->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    hl2->addLayout(vl2);

    //! Check box auto check database
    ui_auto_update = new QCheckBox(main_widget);
    ui_auto_update->setText(tr("Update collections automatically after start"));

    //! Check box --> search cover into directory
    ui_search_cover = new QCheckBox(main_widget);
    ui_search_cover->setText(tr("Search cover art from file directory"));

    //! Checl box : group multiset albums
    ui_group_albums = new QCheckBox(main_widget);
    ui_group_albums->setText(tr("Group multi disc albums as one album"));
    
    verticalLayout->addWidget(lbl1);
    verticalLayout->addWidget(ui_enable_multiDb);
    verticalLayout->addLayout(hl1);
    verticalLayout->addItem( new QSpacerItem(20, 30, QSizePolicy::Minimum, QSizePolicy::Fixed) );
    verticalLayout->addWidget(lbl2);
    verticalLayout->addLayout(hl2);
    verticalLayout->addWidget(ui_auto_update);
    verticalLayout->addWidget(ui_search_cover);
    verticalLayout->addWidget(ui_group_albums);


    // proxy widget
    proxy_widget = new QGraphicsProxyWidget( this );
    proxy_widget->setWidget( main_widget );
    proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );


    m_title->setParentItem(this);
    m_title->setPos(0,0);
}

bool PageLibrary::isViewChanged()
{
    return ( DatabaseManager::instance()->DB_PARAM().groupAlbums != ui_group_albums->isChecked() );
}
    
    
void PageLibrary::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
    //Debug::debug() << "## PageGeneral::resizeEvent";
    doLayout();
}

void PageLibrary::doLayout()
{
    //Debug::debug() << "## PageLibrary::doLayout";
    m_button->setPos(qobject_cast<QGraphicsView*> (m_parent)->viewport()->width()-40,0);
    proxy_widget->setPos(50,30);
}

QSize PageLibrary::size()
{
  if(proxy_widget->isVisible())
    return QSize(
      proxy_widget->geometry().size().width(),
      proxy_widget->geometry().size().height() + 50
      );
  else
    return QSize( proxy_widget->geometry().size().width(), 50);
}

//! ---- restoreSettings -------------------------------------------------------
void PageLibrary::restoreSettings()
{
    //! read settings from DatabaseManager
    DatabaseManager *dbManager = DatabaseManager::instance();
    Debug::debug() << "PageLibrary::restoreSettings dbManager->multiDb" << dbManager->multiDb;

    m_db_params.clear();
    ui_choose_db->clear();

    ui_enable_multiDb->setChecked(dbManager->multiDb);
    enableMultiDatabase();
    //Debug::debug() << "SettingCollectionPage::restoreSettings dbManager->DB_NAME" << dbManager->DB_NAME;

    foreach (const QString& name, dbManager->DB_NAME_LIST()) {
      //Debug::debug() << "SettingCollectionPage::restoreSettings dbManager->params.keys()" << name;
      addDatabaseParam(name, dbManager->DB_PARAM(name));
    }

    int selectedIdx = ui_choose_db->findText(dbManager->DB_NAME);
    if (selectedIdx != -1)
      ui_choose_db->setCurrentIndex(selectedIdx);

    _isLibraryChanged = false;
}

//! ---- saveSettings -----------------------------------------------------------
void PageLibrary::saveSettings()
{
    Debug::debug() << "PageLibrary::saveSettings";

    //! save settings file
    DatabaseManager *dbManager = DatabaseManager::instance();

    dbManager->multiDb = ui_enable_multiDb->isChecked();
    dbManager->DB_NAME = ui_choose_db->currentText();
    dbManager->CLEAR_PARAM();

    foreach (const QString& name, m_db_params.keys()) {
      //Debug::debug() << "SettingCollectionPage::saveSettings m_db_params[name].autoRebuil" << m_db_params[name].autoRebuild;
      //Debug::debug() << "SettingCollectionPage::saveSettings m_db_params[name].checkCover" << m_db_params[name].checkCover;
      dbManager->SET_PARAM(name, m_db_params[name]);
    }

    dbManager->saveSettings();
}


//! ---- database instance management ------------------------------------------
void PageLibrary::enableMultiDatabase()
{
    //Debug::debug() << "SettingCollectionPage::enableMultiDatabase";
    ui_choose_db->setEnabled(ui_enable_multiDb->isChecked());
    ui_db_new_button->setEnabled(ui_enable_multiDb->isChecked());
    ui_db_del_button->setEnabled(ui_enable_multiDb->isChecked());
    ui_db_rename_button->setEnabled(ui_enable_multiDb->isChecked());
}


void PageLibrary::slot_oncheckbox_clicked()
{
    QCheckBox *cb = qobject_cast<QCheckBox *>(sender());
    if(!cb) 
      return;

    const QString dbName = ui_choose_db->currentText();
    if( !m_db_params.contains(dbName))
      return;

    
    if( cb == ui_search_cover )
    {
      m_db_params[dbName].checkCover   = ui_search_cover->isChecked();      
    }
    else if ( cb == ui_auto_update )
    {
      m_db_params[dbName].autoRebuild  = ui_auto_update->isChecked();
      
    }
    else if (cb == ui_group_albums)
    {
      m_db_params[dbName].groupAlbums   = ui_group_albums->isChecked();
    }
}


void PageLibrary::loadDatabaseParam(QString dbName)
{
   if(dbName.isEmpty()) return;

   if(m_db_params.contains(dbName)) {
     //Debug::debug() << "SettingCollectionPage::loadDatabaseParam   NAME = " << dbName;
     //Debug::debug() << "SettingCollectionPage::loadDatabaseParam   m_db_params[dbName].autoRebuild = " << m_db_params[dbName].autoRebuild;
     //Debug::debug() << "SettingCollectionPage::loadDatabaseParam   m_db_params[dbName].checkCover = " << m_db_params[dbName].checkCover;

     ui_auto_update->setChecked(m_db_params[dbName].autoRebuild);
     ui_search_cover->setChecked(m_db_params[dbName].checkCover);
     ui_group_albums->setChecked(m_db_params[dbName].groupAlbums);

     ui_paths_list->clear();
     foreach (QString path, m_db_params[dbName].sourcePathList) {
       ui_paths_list->addItem(path);
     }
     _isLibraryChanged = true;
   }
}


void PageLibrary::newDatabaseParam()
{
    QString currentDbName = ui_choose_db->currentText();

    DialogInput input(0, tr("New database"), tr("Name"));
    input.setEditValue(currentDbName);
    input.setFixedSize(480,140);
    

    if(input.exec() == QDialog::Accepted) {
      QString name = input.editValue();

       if (name.isEmpty()) return;

       if (m_db_params.contains(name)) 
       {
          DialogMessage dlg(0,tr("New database"));
          dlg.setMessage(tr("The database  \"%1\" already exist, please try another name").arg(name));
          dlg.resize(445, 120);
          dlg.exec();  
          return;
       }

      DB::S_dbParam dbParam;
      dbParam.autoRebuild    = false;
      dbParam.checkCover     = true;
      dbParam.sourcePathList = QStringList();
      dbParam.groupAlbums    = false;

      addDatabaseParam(name, dbParam);

      ui_choose_db->setCurrentIndex(ui_choose_db->findText(name));
      //! TODO ? create cover folder, playlist folder
    }
}


void PageLibrary::addDatabaseParam(const QString& name, const DB::S_dbParam& dbParam)
{
    m_db_params[name].autoRebuild    = dbParam.autoRebuild;
    m_db_params[name].checkCover     = dbParam.checkCover;
    m_db_params[name].sourcePathList.clear();
    m_db_params[name].sourcePathList << dbParam.sourcePathList;
    m_db_params[name].groupAlbums    = dbParam.groupAlbums;

    //Debug::debug() << "SettingCollectionPage::addDatabaseParam  dbParam.sourcePathList" << dbParam.sourcePathList;
    //Debug::debug() << "SettingCollectionPage::addDatabaseParam  dbParam.autoRebuild" << dbParam.autoRebuild;
    //Debug::debug() << "SettingCollectionPage::addDatabaseParam  dbParam.checkCover" << dbParam.checkCover;

    if (ui_choose_db->findText(name) == -1)
      ui_choose_db->addItem(name);
}


void PageLibrary::delDatabaseParam()
{
    QString name = ui_choose_db->currentText();

    if (!m_db_params.contains(name) || name.isEmpty()) return;

    DialogQuestion dlg(0,tr("Delete database properties"));
    dlg.setQuestion(tr("Are you sure you want to delete the \"%1\" database ?").arg(name));
    dlg.resize(445, 120);

    if(dlg.exec() == QDialog::Accepted) 
    {
      
      m_db_params.remove(name);
      ui_choose_db->removeItem(ui_choose_db->currentIndex());
      //! TODO ? delete database file + playlist folder + album folder
    }
}

void PageLibrary::renameDatabaseParam()
{
    //! rename an existing database
    Debug::debug() << "SettingCollectionPage::renameDatabaseParam";

    QString oldName = ui_choose_db->currentText();

    
    DialogInput input(0, tr("Rename database"), tr("Name"));
    input.setFixedSize(480,140);
    input.setEditValue(oldName);

    if(input.exec() != QDialog::Accepted)
      return;
    
    QString newName = input.editValue();
    
    
    if (newName.isEmpty() || (newName == oldName) )
      return;

    if (m_db_params.contains(oldName))
    {
      Debug::debug() << "SettingCollectionPage::renameDatabaseParam";

      DB::S_dbParam dbParam = m_db_params.take(oldName);
      addDatabaseParam(newName, dbParam);
      ui_choose_db->removeItem(ui_choose_db->findText(oldName));
      //! TODO rename dabatase file, cover folder, playlist folder

      QCryptographicHash hash(QCryptographicHash::Sha1);
      hash.addData(oldName.toUtf8().constData());
      QFile oldDb(QString(UTIL::CONFIGDIR + "/" + hash.result().toHex() + ".db"));

      QCryptographicHash hash2(QCryptographicHash::Sha1);
      hash2.addData(newName.toUtf8().constData());

      if(oldDb.exists()) {
        oldDb.rename(QString(UTIL::CONFIGDIR + "/" + hash2.result().toHex() + ".db"));
      }
    }
}


//! ----------- on_addPathButton_clicked ---------------------------------------
void PageLibrary::on_addPathButton_clicked()
{
    FileDialog fd(0, FileDialog::AddDir, tr("Add directory to collection"));

    if(fd.exec() == QDialog::Accepted) 
    {
      QString dirName = fd.addDirectory();
    
      if (!dirName.isEmpty())
      {
        ui_paths_list->addItem(dirName);

        QString dbName = ui_choose_db->currentText();
        if (!m_db_params.contains(dbName) || dbName.isEmpty())
          return;

        m_db_params[dbName].sourcePathList.clear();
        for (int i = 0; i < ui_paths_list->count(); i++)
          m_db_params[dbName].sourcePathList << ui_paths_list->item(i)->text();

        _isLibraryChanged = true;
      }
   }
}

//! ----------- on_removePathButton_clicked ------------------------------------
void PageLibrary::on_removePathButton_clicked()
{
    foreach (QListWidgetItem *item, ui_paths_list->selectedItems()) {
      delete ui_paths_list->takeItem(ui_paths_list->row(item));
    }

    QString dbName = ui_choose_db->currentText();
    if (!m_db_params.contains(dbName) || dbName.isEmpty()) return;

    m_db_params[dbName].sourcePathList.clear();
    for (int i = 0; i < ui_paths_list->count(); i++)
      m_db_params[dbName].sourcePathList << ui_paths_list->item(i)->text();

    _isLibraryChanged = true;
}

//! ----------- on_removeAllPathsButton_clicked --------------------------------
void PageLibrary::on_removeAllPathsButton_clicked()
{
    ui_paths_list->clear();

    QString dbName = ui_choose_db->currentText();
    if (!m_db_params.contains(dbName) || dbName.isEmpty()) return;
    m_db_params[dbName].sourcePathList.clear();

    _isLibraryChanged = true;
}

void PageLibrary::setContentVisible(bool b)
{
    if(isOpen != b)
      slot_on_titlebutton_clicked();
}

void PageLibrary::slot_on_titlebutton_clicked()
{
    if(isOpen) {
      proxy_widget->hide();
      m_button->setPixmap(QPixmap(":/images/add_32x32.png"));
      m_button->update();
      isOpen = false;
    }
    else 
    {
      proxy_widget->show();
      m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
      m_button->update();
      isOpen = true;
    }
    
    emit layout_changed();
}

/*
********************************************************************************
*                                                                              *
*    Class PageShortcut                                                        *
*                                                                              *
********************************************************************************
*/
PageShortcut::PageShortcut(QWidget* parentView) : QGraphicsWidget(0)
{
    m_parent       = parentView;
    isOpen         = true;

    createGui();

}

PageShortcut::~PageShortcut()
{
}

void PageShortcut::createGui()
{
    //Debug::debug() << "###### PageShortcut createGui";

    m_title           = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->m_name   = tr("Shortcut settings");

    m_button = new ButtonItem();
    m_button->setPos(0,0);
    m_button->setColor(QApplication::palette().color(QPalette::Base));
    m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
    m_button->setParentItem(this);

    connect(m_button, SIGNAL(clicked()), this, SLOT(slot_on_titlebutton_clicked()));


    m_items["play"]        = new ShortcutGraphicItem(tr("Play/Pause"), SETTINGS()->_shortcutsKey["play"], QPixmap(":/images/media-play.png"));
    m_items["stop"]        = new ShortcutGraphicItem(tr("Stop"), SETTINGS()->_shortcutsKey["stop"], QPixmap(":/images/media-stop.png"));
    m_items["prev_track"]  = new ShortcutGraphicItem(tr("Previous track"), SETTINGS()->_shortcutsKey["prev_track"], QPixmap(":/images/media-prev.png"));
    m_items["next_track"]  = new ShortcutGraphicItem(tr("Next track"), SETTINGS()->_shortcutsKey["next_track"], QPixmap(":/images/media-next.png"));
    m_items["inc_volume"]  = new ShortcutGraphicItem(tr("Increase volume"), SETTINGS()->_shortcutsKey["inc_volume"], QPixmap(":/images/volume-icon.png"));
    m_items["dec_volume"]  = new ShortcutGraphicItem(tr("Decrease volume"), SETTINGS()->_shortcutsKey["dec_volume"], QPixmap(":/images/volume-icon.png"));
    m_items["mute_volume"] = new ShortcutGraphicItem(tr("Mute/Unmute volume"), SETTINGS()->_shortcutsKey["mute_volume"], QPixmap(":/images/volume-muted.png"));
    m_items["jump_to_track"] = new ShortcutGraphicItem(tr("Jump to track"), SETTINGS()->_shortcutsKey["jump_to_track"], QPixmap(":/images/jump_to_32x32.png"));

    connect(m_items.value("play"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("stop"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("prev_track"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("next_track"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("inc_volume"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("dec_volume"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("mute_volume"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("jump_to_track"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));

    m_items.value("play")->setParentItem(this);
    m_items.value("stop")->setParentItem(this);
    m_items.value("prev_track")->setParentItem(this);
    m_items.value("next_track")->setParentItem(this);
    m_items.value("inc_volume")->setParentItem(this);
    m_items.value("dec_volume")->setParentItem(this);
    m_items.value("mute_volume")->setParentItem(this);
    m_items.value("jump_to_track")->setParentItem(this);

    m_title->setParentItem(this);
    m_title->setPos(0,0);
}

void PageShortcut::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
    //Debug::debug() << "## PageShortcut::resizeEvent";
    doLayout();
}

void PageShortcut::doLayout()
{
    //Debug::debug() << "## PageShortcut::doLayout";
    m_button->setPos(qobject_cast<QGraphicsView*> (m_parent)->viewport()->width()-40,0);
  
    int Xpos = 50;
    int Ypos = 30;

    m_items.value("play")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("stop")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("prev_track")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("next_track")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("inc_volume")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("dec_volume")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("mute_volume")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("jump_to_track")->setPos(Xpos,Ypos);
}


QSize PageShortcut::size()
{
  if(m_items.value("play")->isVisible())
    return QSize(
      m_parent->width()-350,
      7*50
      );
  else
    return QSize( m_parent->width()-350, 50);
}


//! ----------- restoreSettings ------------------------------------------------
void PageShortcut::restoreSettings()
{
    Debug::debug() << "PageShortcut restoreSettings";
    m_items["play"]->m_key          = SETTINGS()->_shortcutsKey["play"];
    m_items["stop"]->m_key          = SETTINGS()->_shortcutsKey["stop"];
    m_items["prev_track"]->m_key    = SETTINGS()->_shortcutsKey["prev_track"];
    m_items["next_track"]->m_key    = SETTINGS()->_shortcutsKey["next_track"];
    m_items["inc_volume"]->m_key    = SETTINGS()->_shortcutsKey["inc_volume"];
    m_items["dec_volume"]->m_key    = SETTINGS()->_shortcutsKey["dec_volume"];
    m_items["mute_volume"]->m_key   = SETTINGS()->_shortcutsKey["mute_volume"];
    m_items["jump_to_track"]->m_key = SETTINGS()->_shortcutsKey["jump_to_track"];


    m_items["play"]->m_status = GlobalShortcuts::instance()->shortcuts().value("play").status;
    m_items["stop"]->m_status = GlobalShortcuts::instance()->shortcuts().value("stop").status;
    m_items["prev_track"]->m_status = GlobalShortcuts::instance()->shortcuts().value("prev_track").status;
    m_items["next_track"]->m_status = GlobalShortcuts::instance()->shortcuts().value("next_track").status;
    m_items["inc_volume"]->m_status = GlobalShortcuts::instance()->shortcuts().value("inc_volume").status;
    m_items["dec_volume"]->m_status = GlobalShortcuts::instance()->shortcuts().value("dec_volume").status;
    m_items["mute_volume"]->m_status = GlobalShortcuts::instance()->shortcuts().value("mute_volume").status;
    m_items["jump_to_track"]->m_status = GlobalShortcuts::instance()->shortcuts().value("jump_to_track").status;

    _isChanged = false;
    update();
}

//! ----------- saveSettings ------------------------------------------------
void PageShortcut::saveSettings()
{
    Debug::debug() << "PageShortcut::saveSettings";

    SETTINGS()->_shortcutsKey["play"]          = m_items["play"]->m_key;
    SETTINGS()->_shortcutsKey["stop"]          = m_items["stop"]->m_key;
    SETTINGS()->_shortcutsKey["prev_track"]    = m_items["prev_track"]->m_key;
    SETTINGS()->_shortcutsKey["next_track"]    = m_items["next_track"]->m_key;
    SETTINGS()->_shortcutsKey["inc_volume"]    = m_items["inc_volume"]->m_key;
    SETTINGS()->_shortcutsKey["dec_volume"]    = m_items["dec_volume"]->m_key;
    SETTINGS()->_shortcutsKey["mute_volume"]   = m_items["mute_volume"]->m_key;
    SETTINGS()->_shortcutsKey["jump_to_track"] = m_items["jump_to_track"]->m_key;
}

void PageShortcut::slot_on_shorcutItem_clicked()
{
    //Debug::debug() << "PageShortcut slot_on_shorcutItem_clicked";

    ShortcutGraphicItem *item = qobject_cast<ShortcutGraphicItem *>(sender());
    if(!item) return;

    QString item_name = m_items.key(item);
    QString shortcut  = item->m_key;

    ShortcutDialog* dialog = new ShortcutDialog(item_name, shortcut, m_parent);


    if(dialog->exec() == QDialog::Accepted)
    {
      item->m_key = dialog->newShortcut();
      item->update();
     _isChanged = true;
    }

    delete dialog;
}

void PageShortcut::setContentVisible(bool b)
{
    if(isOpen != b)
      slot_on_titlebutton_clicked();
}


void PageShortcut::slot_on_titlebutton_clicked()
{
    if(isOpen) {
      foreach(ShortcutGraphicItem* item, m_items)
        item->hide();
      
      m_button->setPixmap(QPixmap(":/images/add_32x32.png"));
      m_button->update();
      isOpen = false;
    }
    else 
    {
      foreach(ShortcutGraphicItem* item, m_items)
        item->show();
      
      m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
      m_button->update();
      isOpen = true;
    }
    
    emit layout_changed();
}

/*
********************************************************************************
*                                                                              *
*    Class ShortcutGraphicItem                                                 *
*                                                                              *
********************************************************************************
*/
ShortcutGraphicItem::ShortcutGraphicItem(const QString& name, const QString& key, QPixmap pix)
{
    m_name   = name;
    m_key    = key;
    m_pixmap = pix.scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_status = true;

    setAcceptsHoverEvents(true);
    setAcceptDrops(false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setCursor(Qt::PointingHandCursor);

    setGraphicsItem(this);
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

    //! option configuration
    QStyle *style =  QApplication::style();
    opt.widget = 0;
    opt.palette = QApplication::palette();
    opt.font = QApplication::font();
    opt.fontMetrics = QFontMetrics(opt.font);

    opt.showDecorationSelected = style->styleHint(QStyle::SH_ItemView_ShowDecorationSelected);
    int pm = style->pixelMetric(QStyle::PM_IconViewIconSize);

    opt.decorationSize = QSize(pm, pm);
    opt.decorationPosition = QStyleOptionViewItem::Top;
    opt.displayAlignment = Qt::AlignCenter;

    opt.locale.setNumberOptions(QLocale::OmitGroupSeparator);
    opt.state |= QStyle::State_Active;
    opt.state |= QStyle::State_Enabled;
    opt.state &= ~QStyle::State_Selected;
}

QRectF ShortcutGraphicItem::boundingRect() const
{
    return QRectF(0, 0, 450, 30);
}

// Inherited from QGraphicsLayoutItem
void ShortcutGraphicItem::setGeometry(const QRectF &geom)
{
    setPos(geom.topLeft());
}

// Inherited from QGraphicsLayoutItem
QSizeF ShortcutGraphicItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which);
Q_UNUSED(constraint);
    return boundingRect().size();
}

void ShortcutGraphicItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
   //! Draw frame for State_HasFocus item
   QStyle *style = widget ? widget->style() : QApplication::style();
   opt.rect = boundingRect().toRect();
   style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

   //! paint shortcut name
   painter->setPen(QApplication::palette().color(QPalette::Normal,QPalette::WindowText)) ;
   painter->setFont(opt.font);

   painter->drawPixmap(4, 3, m_pixmap);
   painter->drawText(QRect (32,0,300, 30), Qt::AlignLeft | Qt::AlignVCenter,m_name );
   painter->drawText(QRect (310,0,190, 30), Qt::AlignLeft | Qt::AlignVCenter,m_key );

   if(m_status == false)
     painter->drawPixmap(400, 5, QPixmap(":/images/media-broken-18x18.png") );
   else {
     painter->drawPixmap(400, 5, QPixmap(":/images/checkmark-48x48.png").scaled(QSize(18,18), Qt::KeepAspectRatio, Qt::SmoothTransformation));
   }
}

void ShortcutGraphicItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    opt.state |= QStyle::State_MouseOver;
    this->update();
}

void ShortcutGraphicItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    opt.state &= ~QStyle::State_MouseOver;
    this->update();
}

void ShortcutGraphicItem::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
Q_UNUSED(event)
    emit clicked();
}



/*
********************************************************************************
*                                                                              *
*    Class PageScrobbler                                                       *
*                                                                              *
********************************************************************************
*/
PageScrobbler::PageScrobbler(QWidget* parentView) : QGraphicsWidget(0)
{
    m_parent       = parentView;
    isOpen         = true;

    m_title           = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->m_name   = tr("Scrobbler settings");

    m_button = new ButtonItem();
    m_button->setPos(0,0);
    m_button->setColor(QApplication::palette().color(QPalette::Base));
    m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
    m_button->setParentItem(this);

    connect(m_button, SIGNAL(clicked()), this, SLOT(slot_on_titlebutton_clicked()));

    
    // main widget
    QWidget* main_widget = new QWidget();

    main_widget->setAttribute(Qt::WA_NoBackground, true);
    main_widget->setAutoFillBackground(true);

    /*-------------------------------------------------*/
    /* Scrobbler settings widget                       */
    /* ------------------------------------------------*/
    QVBoxLayout* vl0 = new QVBoxLayout(main_widget);

    //! Check box for lastFm scrobbler enable/disable
    useLastFmScrobbler = new QCheckBox(main_widget);
    useLastFmScrobbler->setText(tr("Use LastFm scrobbler"));

    //! lastfm connexion
    QHBoxLayout* hl0 = new QHBoxLayout();
    statusLabel  = new QLabel(main_widget);
    statusPixmap = new QLabel(main_widget);
    hl0->addWidget(statusPixmap);
    hl0->addWidget(statusLabel);
    hl0->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    accountGroupB = new QGroupBox(tr("Account details"), main_widget);
    accountGroupB->setFlat(true);

    QGridLayout* gl = new QGridLayout(accountGroupB);
    gl->setContentsMargins(0, 0, 0, 0);

    QLabel* label_1 = new QLabel(tr("username"), accountGroupB);
    gl->addWidget(label_1, 0, 0, 1, 1);

    QLabel* label_2 = new QLabel(tr("password"), accountGroupB);
    gl->addWidget(label_2, 1, 0, 1, 1);

    lineEdit_1 = new QLineEdit(accountGroupB);
    lineEdit_1->setMinimumWidth(QFontMetrics(QFont()).width("WWWWWWWWWWWW"));
    gl->addWidget(lineEdit_1, 0, 1, 1, 1);

    lineEdit_2 = new QLineEdit(accountGroupB);
    lineEdit_2->setEchoMode(QLineEdit::Password);
    gl->addWidget(lineEdit_2, 1, 1, 1, 1);

    //! sign in button
    signInButton = new QPushButton(main_widget);
    signInButton->setText(tr("Sign In"));

    //! connextion status
    vl0->addWidget(useLastFmScrobbler);
    vl0->addLayout(hl0);
    vl0->addWidget(accountGroupB);
    vl0->addWidget(signInButton);
    vl0->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // proxy widget
    proxy_widget = new QGraphicsProxyWidget( this );
    proxy_widget->setWidget( main_widget );
    proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    m_title->setParentItem(this);
    m_title->setPos(0,0);


    // scrobbler & slots
    connect(this->useLastFmScrobbler, SIGNAL(clicked()), this, SLOT(enableChange()));
    connect(this->signInButton, SIGNAL(clicked()), this, SLOT(signInClicked()));
    connect(LastFmService::instance(), SIGNAL(signInFinished()), this, SLOT(slotSignInDone()));
}

void PageScrobbler::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
    doLayout();
}

void PageScrobbler::doLayout()
{
    //Debug::debug() << "## PageScrobbler::doLayout";Minimum
    m_button->setPos(qobject_cast<QGraphicsView*> (m_parent)->viewport()->width()-40,0);
    proxy_widget->setPos(50,30);
}

QSize PageScrobbler::size()
{
  if(proxy_widget->isVisible())
    return QSize(
      proxy_widget->geometry().size().width(),
      proxy_widget->geometry().size().height() + 50
      );
  else
    return QSize( proxy_widget->geometry().size().width(), 50);
}

//! ----------- saveSettings ---------------------------------------------------
void PageScrobbler::saveSettings()
{
    Debug::debug() << "PageScrobbler::saveSettings";

    SETTINGS()->_useLastFmScrobbler = this->useLastFmScrobbler->isChecked();
}

//! ----------- restoreSettings ------------------------------------------------
void PageScrobbler::restoreSettings()
{
    _isEnableOld = SETTINGS()->_useLastFmScrobbler;
    this->useLastFmScrobbler->setChecked(_isEnableOld);

    enableChange();

    updateSignInStatus();

    _isChanged = false;
}


//! ----------- enableChange ---------------------------------------------------
void PageScrobbler::enableChange()
{
    //Debug::debug() << "SettingScrobblerPage::enableChange";
    bool isEnableNew = this->useLastFmScrobbler->isChecked();

    if(isEnableNew != _isEnableOld)
       _isChanged = true;
    else
       _isChanged = false;

    // update state according to enable status
    statusLabel->setEnabled(isEnableNew);
    signInButton->setEnabled(isEnableNew);
    accountGroupB->setEnabled(isEnableNew);
}


//! ----------- getAuthentificationStatus --------------------------------------
void PageScrobbler::updateSignInStatus()
{
    bool auth = LastFmService::instance()->isAuthenticated();
    if(auth) {
      lineEdit_1->setText(LastFmService::instance()->username());
      lineEdit_2->clear();
      statusPixmap->setPixmap(QPixmap(":/images/checkmark-48x48.png").scaled(QSize(24,24)));
      //statusPixmap->setPixmap(QPixmap(":/images/signal_accepted-48x48.png").scaled(QSize(24,24)));
      statusLabel->setText(QString(tr("You are log in lastFm service as <b>%1</b>").arg(LastFmService::instance()->username())));

      signInButton->setText(tr("Sign Out"));
    }
    else {
      statusPixmap->setPixmap(QPixmap(":/images/warning-48x48.png").scaled(QSize(24,24)));
      statusLabel->setText(tr("You are not log into"));
      signInButton->setText(tr("Sign In"));
    }
}


//! ----------- signInClicked --------------------------------------------------
void PageScrobbler::signInClicked()
{
     Debug::debug() << "PageScrobbler::signInClicked";

    _isChanged = true;
    bool auth = LastFmService::instance()->isAuthenticated();
    if(auth) {
       lineEdit_1->clear();
       lineEdit_2->clear();
       LastFmService::instance()->signOut();
       updateSignInStatus();
    }
    else {
      LastFmService::instance()->signIn(lineEdit_1->text(), lineEdit_2->text());
    }
}



//! ----------- slotSignInDone -------------------------------------------------
void PageScrobbler::slotSignInDone()
{
    Debug::debug() << "PageScrobbler::slotSignInDone";

    bool success = LastFmService::instance()->isAuthenticated();
    updateSignInStatus();

    if (!success) {
        DialogMessage dlg(0,tr("Authentication failed"));
        dlg.setMessage( tr("Your Last.fm credentials were incorrect"));
        dlg.resize(445, 120);
        dlg.exec();  
    }
}

void PageScrobbler::setContentVisible(bool b)
{
    if(isOpen != b)
      slot_on_titlebutton_clicked();
}

void PageScrobbler::slot_on_titlebutton_clicked()
{
    if(isOpen) {
      proxy_widget->hide();
      m_button->setPixmap(QPixmap(":/images/add_32x32.png"));
      m_button->update();
      isOpen = false;
    }
    else 
    {
      proxy_widget->show();
      m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
      m_button->update();
      isOpen = true;
    }
    
    emit layout_changed();
}


/*
********************************************************************************
*                                                                              *
*    Class PageSongInfo                                                        *
*                                                                              *
********************************************************************************
*/
PageSongInfo::PageSongInfo(QWidget* parentView) : QGraphicsWidget(0)
{
    m_parent       = parentView;
    isOpen         = true;


    m_title   = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->m_name   = tr("Song info");

    
    m_button = new ButtonItem();
    m_button->setPos(0,0);
    m_button->setColor(QApplication::palette().color(QPalette::Base));
    m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
    m_button->setParentItem(this);

    connect(m_button, SIGNAL(clicked()), this, SLOT(slot_on_titlebutton_clicked()));
    
    
    // main widget
    QWidget* main_widget = new QWidget();
    main_widget->setAttribute(Qt::WA_NoBackground, true);
    main_widget->setAutoFillBackground(true);

    
    QLabel *ui_label = new QLabel(tr("Choose the websites you want to use when searching for lyrics"), main_widget);

    ui_listwidget = new QListWidget();

    
    ui_move_up   = new QPushButton(tr("move up"), main_widget);
    ui_move_down = new QPushButton(tr("move down"), main_widget);
    
    ui_move_up->setEnabled(false);   
    ui_move_down->setEnabled(false);
    
    QVBoxLayout* vl1 = new QVBoxLayout();
    vl1->addWidget(ui_move_up);
    vl1->addWidget(ui_move_down);
    vl1->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    
    QHBoxLayout* hl0 = new QHBoxLayout();
    hl0->addWidget(ui_listwidget);    
    hl0->addLayout(vl1);
    
    QVBoxLayout* vl0 = new QVBoxLayout(main_widget);
    vl0->addWidget(ui_label);
    vl0->addLayout(hl0);
    
    connect(ui_listwidget, SIGNAL(itemChanged(QListWidgetItem*)),SLOT(slot_item_changed(QListWidgetItem*)));    

    connect(ui_listwidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
          SLOT(slot_current_item_changed(QListWidgetItem*)));


    connect(ui_move_up, SIGNAL(clicked()), this, SLOT(slot_move_up()));
    connect(ui_move_down, SIGNAL(clicked()), this, SLOT(slot_move_down()));
  
    // proxy widget
    proxy_widget = new QGraphicsProxyWidget( this );
    proxy_widget->setWidget( main_widget );
    proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    m_title->setParentItem(this);
    m_title->setPos(0,0);
}

void PageSongInfo::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
    //Debug::debug() << "## PageSongInfo::resizeEvent";
    doLayout();
}

void PageSongInfo::doLayout()
{
    m_button->setPos(qobject_cast<QGraphicsView*> (m_parent)->viewport()->width()-40,0);
    proxy_widget->setPos(50,30);
}


QSize PageSongInfo::size()
{
  if(proxy_widget->isVisible())
    return QSize(
      proxy_widget->geometry().size().width(),
      proxy_widget->geometry().size().height() + 50
      );
  else
    return QSize( proxy_widget->geometry().size().width(), 50);
}


//! ----------- saveSettings ---------------------------------------------------
void PageSongInfo::saveSettings()
{
    Debug::debug() << "PageSongInfo::saveSettings";
    QStringList search_order;
    for (int i=0 ; i < ui_listwidget->count() ; ++i) 
    {
        const QListWidgetItem* item = ui_listwidget->item(i);
        if (item->checkState() == Qt::Checked)
            search_order << item->text();
    }

    SETTINGS()->_lyrics_providers = search_order;
}

//! ----------- restoreSettings ------------------------------------------------
void PageSongInfo::restoreSettings()
{
    QStringList all_providers = ServiceLyrics::fullProvidersList();
    
    foreach(const QString& provider, SETTINGS()->_lyrics_providers) {
      
      QListWidgetItem* item = new QListWidgetItem(ui_listwidget);
      item->setText(provider);
      item->setCheckState(Qt::Checked);
      item->setForeground(palette().color(QPalette::Active, QPalette::Text));
      
      all_providers.removeOne(provider);
    }
    
    foreach(const QString& provider, all_providers) {
      QListWidgetItem* item = new QListWidgetItem(ui_listwidget);
      item->setText(provider);
      item->setCheckState(Qt::Unchecked);
      item->setForeground(palette().color(QPalette::Disabled, QPalette::Text));
    }
}

void PageSongInfo::setContentVisible(bool b)
{
    if(isOpen != b)
      slot_on_titlebutton_clicked();
}

void PageSongInfo::slot_on_titlebutton_clicked()
{
    if(isOpen) {
      proxy_widget->hide();
      m_button->setPixmap(QPixmap(":/images/add_32x32.png"));
      m_button->update();
      isOpen = false;
    }
    else 
    {
      proxy_widget->show();
      m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
      m_button->update();
      isOpen = true;
    }
    
    emit layout_changed();
}

void PageSongInfo::slot_item_changed(QListWidgetItem* item) 
{
  const bool checked = item->checkState() == Qt::Checked;
  item->setForeground(checked ? palette().color(QPalette::Active, QPalette::Text)
                              : palette().color(QPalette::Disabled, QPalette::Text));
}

void PageSongInfo::slot_current_item_changed(QListWidgetItem* item) 
{
  if (!item) {
    ui_move_up->setEnabled(false);
    ui_move_down->setEnabled(false);
  } else {
    const int row = ui_listwidget->row(item);
    ui_move_up->setEnabled(row != 0);
    ui_move_down->setEnabled(row != ui_listwidget->count() - 1);
  }
}

void PageSongInfo::slot_move_up() 
{
    Move(-1);
}

void PageSongInfo::slot_move_down() 
{
    Move(+1);
}

void PageSongInfo::Move(int d) 
{
  const int row = ui_listwidget->currentRow();
  QListWidgetItem* item = ui_listwidget->takeItem(row);
  ui_listwidget->insertItem(row + d, item);
  ui_listwidget->setCurrentRow(row + d);
}


/*
********************************************************************************
*                                                                              *
*    Class BottomWidget                                                        *
*                                                                              *
********************************************************************************
*/
BottomWidget::BottomWidget(QWidget* parentView)
{
    m_parent       = parentView;

    this->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    
    QPushButton  *ui_save_button   = new QPushButton(QIcon(":/images/save-32x32.png"), tr("Apply"), 0);
    QPushButton  *ui_cancel_button = new QPushButton(QIcon::fromTheme("dialog-cancel"), tr("Cancel"), 0);

    ui_save_button->setMinimumWidth(150);
    ui_save_button->setMaximumWidth(150);

    ui_cancel_button->setMinimumWidth(150);
    ui_cancel_button->setMaximumWidth(150);
    
    connect(ui_save_button, SIGNAL(clicked()), this, SIGNAL(save_clicked()));
    connect(ui_cancel_button, SIGNAL(clicked()), this, SIGNAL(cancel_clicked()));

    
    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addItem( new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
    hbox->addWidget(ui_save_button);
    hbox->addWidget(ui_cancel_button);
    hbox->addItem( new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );    

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0,0,0,0);
    vbox->setSpacing(2);
    vbox->setMargin(0);    
    vbox->addLayout(hbox);
}

