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

#ifndef _MENU_VIEW_H_
#define _MENU_VIEW_H_

#include "menumodel.h"
#include <QtGui>

class MenuModel;



/*
********************************************************************************
*                                                                              *
*    Class MenuView                                                            *
*                                                                              *
********************************************************************************
*/
class MenuView : public QTreeView
{
  Q_OBJECT
  public:
    MenuView (MenuModel *model, QWidget *parent);
    void updateSelection(QModelIndex& index);

  protected:
    void drawBranches ( QPainter * painter, const QRect & rect, const QModelIndex & index ) const;

  private slots:
    void slot_on_item_activated(QModelIndex);
  
  private :
    QItemSelectionModel    *m_selection_model;
    MenuModel              *m_model;
};

/*
********************************************************************************
*                                                                              *
*    Class MenuViewDelegate                                                    *
*                                                                              *
********************************************************************************
*/
class MenuViewDelegate : public QItemDelegate
{
  public:
    MenuViewDelegate(QObject *parent);

  protected:
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint(const QStyleOptionViewItem &option,const QModelIndex &index) const;
};


#endif // _MENU_VIEW_H_
