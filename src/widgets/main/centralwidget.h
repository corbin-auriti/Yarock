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

#ifndef _CENTRAL_WIDGET_H_
#define _CENTRAL_WIDGET_H_

#include "views/browser_view.h"

#include "widgets/main/centraltoolbar.h"
#include "widgets/main/menuwidget.h"
#include "widgets/nowplaying/nowplayingview.h"
#include "playqueue/playlistwidget.h"


#include "widgets/customsplitter.h"
#include "views.h"

#include <QWidget>
#include <QFrame>
#include <QByteArray>
#include <QResizeEvent>


/*
********************************************************************************
*                                                                              *
*    Class CentralWidget                                                       *
*                                                                              *
********************************************************************************
*/
class CentralWidget : public QWidget
{
Q_OBJECT
  public:
    CentralWidget(QWidget *parent,
               CentralToolBar     *centralToolBar,
               NowPlayingView     *nowPlayingView,
               PlaylistWidget     *playlistWidget,
               BrowserView        *browser
               );

    QByteArray splitterState() {return m_viewsSplitter->saveState();}
    bool eventFilter(QObject *obj, QEvent *ev);

    void restoreState();

  private :
    void createGui();

  private slots:
    void slot_show_playlist();
    void slot_show_menu();
    void slot_show_nowplaying();

  private :
    QWidget             *m_parent;

    CentralToolBar      *m_centraltoolbar;

    MenuWidget          *m_menu_widget;
    NowPlayingView      *m_nowplayingview;
    PlaylistWidget      *m_playlistwidget;
    BrowserView         *m_browserview;

    CustomSplitter      *m_viewsSplitter;
};


#endif // _CENTRAL_WIDGET_H_
