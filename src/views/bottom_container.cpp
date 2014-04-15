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

#include "bottom_container.h"
#include "settings.h"

#include <QPainter>
/*
********************************************************************************
*                                                                              *
*    Class BottomContainer                                                     *
*                                                                              *
********************************************************************************
*/
BottomContainer::BottomContainer(QWidget *parent) : QWidget(parent)
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum );
    
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    
    m_layout->addSpacing(2);    
    m_layout->addSpacing(2);
    
    m_contentWidget = 0;
}


void BottomContainer::setContent(QWidget* w)
{
    if(m_contentWidget) {
      m_contentWidget->hide();
      m_layout->removeWidget(m_contentWidget);
      m_contentWidget = 0;
    }
  
    if(w) {
      w->setParent(this);
      m_layout->insertWidget(1,w);
      this->show();
    }
}


/*******************************************************************************
   paintEvent
*******************************************************************************/
void BottomContainer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QColor m_brush_color = SETTINGS()->_baseColor;
    m_brush_color.setAlphaF(0.6);
    
    painter.setPen(QPen( m_brush_color, 0.1, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(QBrush( m_brush_color ,Qt::SolidPattern));

    painter.drawRect(this->rect().adjusted(0,0,-10,0));

    QWidget::paintEvent(event);
}