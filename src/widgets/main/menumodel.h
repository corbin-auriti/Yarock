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

#ifndef _MENU_MODEL_H_
#define _MENU_MODEL_H_

#include "views.h"

#include <QStandardItemModel>
#include <QModelIndex>
#include <QAction>


enum MenuItemRole {
    GlobalActionRole    = Qt::UserRole + 1,
    MenuActionRole      = Qt::UserRole + 2,
    ViewModeRole        = Qt::UserRole + 3,
    FileRole            = Qt::UserRole + 4
};


class QStandardItem;
/*
********************************************************************************
*                                                                              *
*    Class MenuModel                                                           *
*                                                                              *
********************************************************************************
*/
class MenuModel : public QStandardItemModel
{
Q_OBJECT
  static MenuModel *INSTANCE;

  public:
    MenuModel(QObject *parent = 0);
    ~MenuModel();

    static MenuModel* instance() { return INSTANCE; }

  public slots:
    void slot_on_menu_view_activated(QModelIndex);
    
  private:
    void configureMenuAction();
    void populateMenu();
    void populateComputerItem();

  private slots:
    void slot_on_menu_triggered();

  signals :
    void modelItemActivated(QModelIndex);

  private:
    QStandardItem       *m_radioItem;
    QStandardItem       *m_computerItem;
};

Q_DECLARE_METATYPE(QAction*);
Q_DECLARE_METATYPE(QModelIndex);


#endif // _MENU_MODEL_H_
