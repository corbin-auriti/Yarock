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

#ifndef _MENU_BAR_H_
#define _MENU_BAR_H_

#include <QWidget>
#include <QPushButton>
#include <QIcon>
#include <QString>
#include <QEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QTimer>

/*
********************************************************************************
*                                                                              *
*    Class MenuBar                                                             *
*                                                                              *
********************************************************************************
*/
class MenuModel;
class MenuBarButton;

class MenuBar : public QWidget
{
Q_OBJECT
  public:
    MenuBar(QWidget *parent = 0);
    virtual void setModel(MenuModel * m);

  public slots:
    void slot_hideMenu();

  private:
    MenuModel               *m_model;
    QList<MenuBarButton*>    m_listButton;
    
  private slots:
    void updateEntry();
};


/*
********************************************************************************
*                                                                              *
*    Class MenuBarButton                                                       *
*                                                                              *
********************************************************************************
*/
class MenuBarButton : public QPushButton
{
Q_OBJECT
  public:
    MenuBarButton( const QIcon & icon, const QString & text, QWidget *parent );
    QSize sizeHint() const;

    QMenu* menu() {return m_menu;}

  protected:
    void paintEvent(QPaintEvent* event);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mousePressEvent ( QMouseEvent * e );

  private slots:
    void slot_button_clicked();

  private:
    QMenu    *m_menu;

  signals :
    void menu_activated();
};


#endif // _MENU_BAR_H_
