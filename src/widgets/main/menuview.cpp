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

#include "menuview.h"
#include "menumodel.h"
#include "core/database/databasemanager.h"
#include "global_actions.h"
#include "debug.h"

/*
********************************************************************************
*                                                                              *
*    Class MenuView                                                            *
*                                                                              *
********************************************************************************
*/
MenuView::MenuView(MenuModel *model, QWidget *parent) : QTreeView(parent)
{
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    this->setPalette(palette);

    //! config QTreeView
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    this->setFrameShape(QFrame::NoFrame);
    this->setFrameShadow(QFrame::Plain);
    this->setContentsMargins(0,0,0,0);

    this->header()->hide();
    this->setWordWrap(true);
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setRootIsDecorated(false);
    this->setUniformRowHeights(false);

    //! scrolbar setup
    this->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    this->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    //! model & delegate
    m_model = model;
    this->setItemDelegate(new MenuViewDelegate(this));
    this->setModel(model);

    //! default expand/collapse
    this->expandAll();

    //! selection model
    m_selection_model = new QItemSelectionModel(model);
    this->setSelectionModel(m_selection_model);

    //! signals
    connect(this,SIGNAL(clicked(QModelIndex)),this, SLOT(slot_on_item_activated(QModelIndex)));
}


void MenuView::slot_on_item_activated(QModelIndex index)
{
    //Debug::debug() << "-- MenuView --> slot_on_item_activated" << index;
    const bool children =  index.model()->hasChildren(index);

    if(!children) 
    {
       m_model->slot_on_menu_view_activated( index );      
    }
}

/*******************************************************************************
    updateSelection
*******************************************************************************/
void MenuView::updateSelection(QModelIndex& index)
{
    //Debug::debug() << "-- MenuView --> updateSelection" << index;
    m_selection_model->select(index, QItemSelectionModel::ClearAndSelect);
}


/*******************************************************************************
    drawBranches
*******************************************************************************/
void MenuView::drawBranches ( QPainter * painter, const QRect & rect, const QModelIndex & index ) const
{
    QRect primitive(rect.right() + 4, rect.top(), 5, rect.height());

    QStyleOption opt;
    opt.initFrom(this);
    opt.rect = primitive;

    const bool children = static_cast<QStandardItemModel*>( this->model() )->hasChildren(index);

    if(children) {
      const bool expanded = isExpanded (  index );
      if(!expanded)
        style()->drawPrimitive(QStyle::PE_IndicatorArrowRight, &opt, painter, this);
      else
        style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, painter, this);
    }
}

/*
********************************************************************************
*                                                                              *
*    Class MenuViewDelegate                                                    *
*                                                                              *
********************************************************************************
*/
MenuViewDelegate::MenuViewDelegate(QObject *parent):QItemDelegate(parent)
{
}

void MenuViewDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    if(!index.isValid() ) return;

    const bool children =  index.model()->hasChildren(index);

    //! get item data
    const QString text = index.data(Qt::DisplayRole).toString();
    const QIcon   icon = qvariant_cast<QIcon> ( index.data(Qt::DecorationRole) );

    //! font
    QFont font_normal = QApplication::font();
    font_normal.setStyleStrategy(QFont::PreferAntialias);
    font_normal.setBold(false);

    QFont font_title  = QApplication::font();
    font_title.setPointSize(font_normal.pointSize()*1.1);
    font_title.setStyleStrategy(QFont::PreferAntialias);
    font_title.setBold(true);

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (!index.parent().isValid() && children)
    {
      //! paint text
      painter->setFont(font_title);
      painter->setPen( QApplication::palette().color ( QPalette::Disabled, QPalette::WindowText) );
      painter->drawText(option.rect.adjusted(15 ,0,0,0),Qt::AlignVCenter, text);
    }
    else if(!index.parent().isValid() && !children)
    {
      //! draw backgroud
      QStyleOptionViewItemV4 opt(option);
      opt.rect.adjust(-20,0,0,0);
      QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
      style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

      //! paint icon
      QRect decorationRect = QRect(option.rect.topLeft(),QSize(18,18));
      icon.paint(painter, decorationRect.adjusted(18,4,18,4), option.decorationAlignment);

      //! paint text
      painter->setFont(font_normal);
      if(option.state & QStyle::State_Selected)
         painter->setPen( QApplication::palette().color ( QPalette::Normal, QPalette::HighlightedText) );
      else
        painter->setPen( QApplication::palette().color ( QPalette::Normal, QPalette::WindowText) );


      painter->drawText(option.rect.adjusted(38,0,0,0),Qt::AlignVCenter, text);
    }
    else //! parent valid --> it's a child
    {
      //! draw backgroud
      QStyleOptionViewItemV4 opt(option);
      opt.rect.adjust(-20,0,0,0);
      QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
      style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

      //! paint icon
      QRect decorationRect = QRect(option.rect.topLeft(),QSize(18,18));
      icon.paint(painter, decorationRect.adjusted(-2,4,-2,4), option.decorationAlignment);

      //! paint text
      painter->setFont(font_normal);
      if(option.state & QStyle::State_Selected)
         painter->setPen( QApplication::palette().color ( QPalette::Normal, QPalette::HighlightedText) );
      else
        painter->setPen( QApplication::palette().color ( QPalette::Normal, QPalette::WindowText) );


      painter->drawText(option.rect.adjusted(20 ,0,0,0),Qt::AlignVCenter, text);
    }
}

/*******************************************************************************
    sizeHint
*******************************************************************************/
QSize MenuViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(!index.isValid())
      return QSize();
    else
      return QSize(option.rect.width(), 26);
}
