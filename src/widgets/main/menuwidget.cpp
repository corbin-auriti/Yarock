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

//! local
#include "menuwidget.h"
#include "menumodel.h"
#include "global_actions.h"
#include "settings.h"

#include "debug.h"

//! Qt
#include <QtGui>
#include <QApplication>
#include <QPainter>
#include <QMenu>

/*
********************************************************************************
*                                                                              *
*    Class MenuWidget                                                          *
*                                                                              *
********************************************************************************
*/
MenuWidget::MenuWidget(QWidget * parent) : QWidget(parent)
{
    m_parent = parent;
    
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    this->setPalette(palette);

    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        
    /*  navigator bar  */
    m_menuBar = new MenuBar(this);
    m_menuBar->setModel( MenuModel::instance() );

    /*  navigator tree  */    
    m_menu_view = new MenuView(MenuModel::instance(), this);
    
    /*  stacked widget */    
    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(m_menu_view);
    stackedWidget->addWidget(m_menuBar);
     
    ui_control_button = new QPushButton(this);
    ui_control_button->setFlat(true);
    ui_control_button->setStyleSheet ("text-align: left"); 
    ui_control_button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QHBoxLayout* hl = new QHBoxLayout();
    hl->setSpacing(0);
    hl->setContentsMargins(0, 0, 0, 0);    
    hl->addWidget(ui_control_button);
    hl->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    /* final layout */
    QVBoxLayout* verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    verticalLayout->addWidget(stackedWidget);
    verticalLayout->addLayout(hl);
    
    
    this->setLayout(verticalLayout);
    
    /* action */
    connect(ui_control_button, SIGNAL(clicked()), this, SLOT(slot_change_menu_style()));
}


MenuView* MenuWidget::menuView()
{
    return m_menu_view;
}

/*******************************************************************************
    restoreState
*******************************************************************************/
void MenuWidget::restoreState()
{
    /* only called at startup through CentralWidget::restoreState() */
    slot_change_menu_style(true);
}

/*******************************************************************************
    slot_change_menu_style
*******************************************************************************/
void MenuWidget::slot_change_menu_style ( bool isStartup /*=false*/ )
{
    /* switch settings if not startup */
    if(!isStartup) {
      SETTINGS()->_is_menu_bar = !SETTINGS()->_is_menu_bar;
    }

    QList<int> widths = m_splitter->sizes();

    int width_menu    = widths.at(0);
    int width_browser = widths.at(1);
    
    if(SETTINGS()->_is_menu_bar)
    {
      stackedWidget->setCurrentIndex(1);

      ui_control_button->setIcon(QIcon(":/images/add_32x32.png"));
      ui_control_button->setToolTip( tr("show extended menu") );
      
      /* resize splitter */
      if(!isStartup) {
        width_browser = width_browser + (width_menu - m_menuBar->width());

        QList<int> list;
        list << m_menuBar->width() << width_browser << widths.at(2);
        m_splitter->setSizes (list);
      }
      
      /* enable splitter handle */
      QSplitterHandle *hndl = m_splitter->handle(1);
      hndl->setEnabled(false);
    }
    else 
    {
      stackedWidget->setCurrentIndex(0);

      ui_control_button->setIcon(QIcon(":/images/remove_32x32.png"));
      ui_control_button->setToolTip( tr("show compact menu") );

      /* resize splitter */
      if(!isStartup) {
        width_browser += width_menu - 180;

        QList<int> list;
        list << 180 << width_browser << widths.at(2);
        m_splitter->setSizes (list);
      }

      
      /* disable splitter handle */
      QSplitterHandle *hndl = m_splitter->handle(1);
      hndl->setEnabled(true);
    }
}

