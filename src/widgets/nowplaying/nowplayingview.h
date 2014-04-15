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

#ifndef _NOWPLAYINGVIEW_H_
#define _NOWPLAYINGVIEW_H_

//Qt
#include <QWidget>
#include <QMenu>
#include <QPaintEvent>
#include <QContextMenuEvent>

/*
********************************************************************************
*                                                                              *
*    Class NowPlayingView                                                      *
*                                                                              *
********************************************************************************
*/
class NowPlayingView : public QWidget
{
Q_OBJECT
  public:
    NowPlayingView(QWidget *parent);

  protected:
    void paintEvent(QPaintEvent *event);
    void contextMenuEvent ( QContextMenuEvent * event );

  private:
    QMenu           *m_context_menu;
};


#endif // _NOWPLAYINGVIEW_H_
