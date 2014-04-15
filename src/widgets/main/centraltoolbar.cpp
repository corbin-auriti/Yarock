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
#include "centraltoolbar.h"

#include "maintoolbutton.h"
#include "menumodel.h"
#include "sort_widget.h"

#include "widgets/exlineedit.h"
#include "widgets/spacer.h"
#include "widgets/popupcompleter/search_popup.h"

#include "settings.h"
#include "global_actions.h"
#include "debug.h"

#include <QtGui>
#include <QShortcut>

CentralToolBar* CentralToolBar::INSTANCE = 0;
/*
********************************************************************************
*                                                                              *
*    Class CentralToolBar                                                      *
*                                                                              *
********************************************************************************
*/
CentralToolBar::CentralToolBar(QWidget *parent) : QToolBar( "CentralToolBar", parent )
{
    INSTANCE   = this;

    this->setObjectName(QString::fromUtf8("CentralToolBar"));
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum );
    this->setMaximumHeight(28);
    this->setFloatable(false);
    this->setMovable(false);
    this->setContentsMargins( 0, 0, 0, 0);
    this->setToolButtonStyle( Qt::ToolButtonIconOnly );
    this->setIconSize( QSize( 20, 20) );

    /* Tools Button and Menu */
    MainToolButton* main_tb = new MainToolButton(this);

    /*  prev/next History actions  */
    ACTIONS()->insert(BROWSER_PREV, new QAction(QIcon::fromTheme("go-previous", QIcon(":/images/go-previous.png")),tr("Go back"),this));
    ACTIONS()->insert(BROWSER_NEXT, new QAction(QIcon::fromTheme("go-next", QIcon(":/images/go-next.png")),tr("Go forward"),this));
    ACTIONS()->insert(BROWSER_UP,   new QAction(QIcon::fromTheme("go-up", QIcon(":/images/go-up.png")),tr("Go up"),this));
    ACTIONS()->value(BROWSER_PREV)->setEnabled(false);
    ACTIONS()->value(BROWSER_NEXT)->setEnabled(false);
    
    /* explorer search field  */
    ui_explorer_filter = new ExLineEdit(this);
    ui_explorer_filter->setInactiveText(tr("Search"));
    ui_explorer_filter->setMinimumWidth(250);
    ui_explorer_filter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    new QShortcut(QKeySequence("Ctrl+F"), ui_explorer_filter, SLOT(slotFocus()));

    /* explorer search button menu */
    IconButton* icon_1 = new IconButton(this);
    icon_1->setIcon(QIcon(":/images/search_48x48.png"));
  
    QMenu* explorer_menu = new QMenu();
      explorer_menu->addAction(ACTIONS()->value(APP_ENABLE_SEARCH_POPUP));
    icon_1->setMenu(explorer_menu);
    ui_explorer_filter->addLeftIcon(icon_1);
    
    /* explorer popup completer */
    ui_popup_completer = 0;
    slot_explorer_popup_setting_change();
    
    /* playqueue search field  */
    ui_playqueue_filter = new ExLineEdit(this);
    ui_playqueue_filter->setMinimumWidth(0);
    ui_playqueue_filter->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    ui_playqueue_filter->setInactiveText(tr("Playqueue filter"));
    
    /* playqueue sort button */
    QMenu* sort_menu = new QMenu();
    QWidgetAction * sort_action_widget = new QWidgetAction( this );
    
    SortWidget* sort_widget = new SortWidget(sort_menu);
    sort_action_widget->setDefaultWidget( sort_widget );
    sort_menu->addAction( sort_action_widget );
    
    IconButton* icon_2 = new IconButton(this);
    icon_2->setIcon(QIcon(":/images/view-option-48x48.png"));
    icon_2->setMenu(sort_menu);
    ui_playqueue_filter->addLeftIcon(icon_2);

   
    /* ToolBar setup  */
    this->addWidget( new FixedSpacer( this, QSize(5, 0)) );
    this->addWidget( main_tb );
    this->addAction( ACTIONS()->value(BROWSER_PREV) );
    this->addAction( ACTIONS()->value(BROWSER_NEXT) );
    this->addAction( ACTIONS()->value(BROWSER_UP) );
    this->addWidget( new FixedSpacer( this, QSize(5, 0)) );
    this->addWidget( ui_explorer_filter );
    this->addWidget( new FixedSpacer( this, QSize(5, 0)) );
    
    m_filterWidgetAction = this->addWidget( ui_playqueue_filter );

    /* connect signals */  
    connect(main_tb, SIGNAL(dbNameChanged()), this, SIGNAL(dbNameChanged()));
    connect(ui_explorer_filter, SIGNAL(textfield_entered()), this, SLOT(slot_send_explorer_filter()));
    connect(ui_playqueue_filter, SIGNAL(textfield_entered()), this, SLOT(slot_send_playqueue_filter()));
    connect(ACTIONS()->value(APP_ENABLE_SEARCH_POPUP), SIGNAL(triggered()), this, SLOT(slot_explorer_popup_setting_change()));
}


/*******************************************************************************
    CentralToolBar::showPlayqueueFilter
*******************************************************************************/
void CentralToolBar::showPlayqueueFilter(bool show)
{
    m_filterWidgetAction->setVisible(show);
}

void CentralToolBar::resizePlayqueueToolWidget(int size)
{
    int new_size = size + 8;
    if(new_size > 20) {
      m_filterWidgetAction->setVisible(true);
      ui_playqueue_filter->setMinimumWidth(new_size);
    }
    else
      m_filterWidgetAction->setVisible(false);
}

/*******************************************************************************
    CentralToolBar explorer filter
*******************************************************************************/
QString CentralToolBar::explorerFilter()
{
    return ui_explorer_filter->text();
}
       
    
void CentralToolBar::setExplorerFilterText(const QString& t)
{
    ui_explorer_filter->setText(t);
    ui_explorer_filter->clearFocus();

}

/*******************************************************************************
    CentralToolBar::showHideGoUp
*******************************************************************************/
void CentralToolBar::showHideGoUp(VIEW::Id m)
{
    ACTIONS()->value(BROWSER_UP)->setVisible(m == VIEW::ViewFileSystem);
}

    
/*******************************************************************************
    CentralToolBar::slot_send_explorer_filter
*******************************************************************************/
void CentralToolBar::slot_send_explorer_filter()
{
    //Debug::debug() << "CentralToolBar::slot_send_explorer_filter";

    // send search
    emit explorerFilterActivated(ui_explorer_filter->text());
}

/*******************************************************************************
    CentralToolBar::slot_send_playqueue_filter
*******************************************************************************/
void CentralToolBar::slot_send_playqueue_filter()
{
    // send search
    emit playqueueFilterActivated(ui_playqueue_filter->text());
}

/*******************************************************************************
    CentralToolBar::slot_explorer_popup_setting_change
*******************************************************************************/
void CentralToolBar::slot_explorer_popup_setting_change()
{
    //Debug::debug() << "CentralToolBar::slot_explorer_popup_setting_change ";

    SETTINGS()->_enableSearchPopup = ACTIONS()->value(APP_ENABLE_SEARCH_POPUP)->isChecked();

    if( SETTINGS()->_enableSearchPopup)
    {
      if(!ui_popup_completer)
        ui_popup_completer = new SearchPopup(ui_explorer_filter);
    }
    else
    {
      if(ui_popup_completer) {
        delete ui_popup_completer;
        ui_popup_completer = 0;
      }
    }
}
