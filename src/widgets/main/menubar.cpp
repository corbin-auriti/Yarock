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
#include "menubar.h"
#include "menumodel.h"

#include "settings.h"
#include "global_actions.h"

#include "debug.h"

//! Qt
#include <QVBoxLayout>
#include <QApplication>
#include <QPainter>
#include <QMenu>

/*
********************************************************************************
*                                                                              *
*    Class MenuBar                                                             *
*                                                                              *
********************************************************************************
*/
MenuBar::MenuBar(QWidget * parent) : QWidget(parent)
{
    this->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::MinimumExpanding );
    this->setFocusPolicy(Qt::NoFocus);

    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    this->setPalette(palette);
    
    QVBoxLayout* verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(10);
    verticalLayout->setContentsMargins(5, 5, 5, 5);
        
    this->setLayout(verticalLayout);
    
    /* set fixed width to 50 (sizepolicy is Fixed */
    this->setMinimumWidth(50);
    this->setMaximumWidth(50);
}

//! ----- MenuBar::setModel -----------------------------------------------
void MenuBar::setModel(MenuModel * m)
{
    m_model = m;
    connect(m_model, SIGNAL(modelUpdated()),this, SLOT(updateEntry()));

    updateEntry();
}


//! ----- MenuBar::updateEntry --------------------------------------------
void MenuBar::updateEntry()
{
    // clear all widget 
    QLayoutItem *child;
    while ((child = this->layout()->takeAt(0)) != 0)
      delete child;

    this->layout()->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));


    /* clear toolbar and delete previous button */
    qDeleteAll(m_listButton);
    m_listButton.clear();

    for (int i=0; i < m_model->rowCount(QModelIndex()); i++)
    {
      const QModelIndex &idx = m_model->index(i, 0, QModelIndex());

      if( !idx.isValid() ) continue;

      QString text   = idx.data(Qt::DisplayRole).toString();
      QIcon icon   = qvariant_cast<QIcon>(idx.data(Qt::DecorationRole));

      MenuBarButton *button = new MenuBarButton(icon,text,this);

      this->layout()->addWidget(button);
      m_listButton.append(button);

      //! build menu
      for (int j=0; j < m_model->rowCount(idx); j++)
      {
          const QModelIndex childIdx = m_model->index(j, 0, idx);
          QAction *a = childIdx.data(MenuActionRole).value<QAction*>();
          button->menu()->addAction(a);
      }
    }

    //! action hide menu on click
    foreach(MenuBarButton* button, m_listButton) {
      connect(button, SIGNAL(menu_activated()), this, SLOT(slot_hideMenu()));
    }

    this->layout()->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    
    update();
    updateGeometry();
}


void MenuBar::slot_hideMenu ( )
{
    MenuBarButton *b = qobject_cast<MenuBarButton *>(sender());
    foreach(MenuBarButton* button, m_listButton) {
     if(button != b)
       button->menu()->hide();
    }
}


/*
********************************************************************************
*                                                                              *
*    Class MenuBarButton                                                       *
*                                                                              *
********************************************************************************
*/
MenuBarButton::MenuBarButton( const QIcon &icon, const QString &text, QWidget *parent )
    : QPushButton( icon, "", parent )
{
Q_UNUSED(text)
  
    this->setFocusPolicy(Qt::TabFocus);
    this->setIconSize( QSize(26,26) );
    this->setFlat ( true );

    m_menu  = new QMenu(this);
}

void MenuBarButton::paintEvent( QPaintEvent* event )
{
    QPushButton::paintEvent(event);
}

QSize MenuBarButton::sizeHint() const
{
    QSize size = QPushButton::sizeHint();

    int width = 8;
    if( !icon().isNull() ) 
    {
        width += iconSize().width();
    }

    if( !text().isEmpty() )  
    {
        QFontMetrics fm( this->font() );
        width += fm.width( text() );
        width += 10; // paddding
    }
    size.setWidth( width );
    return size;
}


void MenuBarButton::enterEvent(QEvent *e)
{
    slot_button_clicked();

    QPushButton::enterEvent(e);
}

void MenuBarButton::leaveEvent(QEvent *e)
{
    //m_menu->hide();
    QPushButton::leaveEvent(e);
}

void MenuBarButton::slot_button_clicked()
{
    //Debug::debug() << "--- MenuBarButton -> slot_button_clicked ";
    if(!m_menu->isVisible()) {
      QPoint location = this->mapToGlobal(QPoint(this->width()+4,5));
      m_menu->popup(location);
      m_menu->show();
      emit menu_activated();
    }
}

void MenuBarButton::mousePressEvent ( QMouseEvent * e )
{
    e->ignore();
}
