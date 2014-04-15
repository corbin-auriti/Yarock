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

#ifndef _PLAYLISTWIDGET_H_
#define _PLAYLISTWIDGET_H_

#include "playlistview.h"
#include "playqueue_model.h"

#include <QtGui/QWidget>
#include <QSignalMapper>
#include <QActionGroup>
#include <QAction>

/*
********************************************************************************
*                                                                              *
*    Class PlaylistWidget                                                      *
*                                                                              *
********************************************************************************
*/
class PlaylistWidget : public QWidget
{
Q_OBJECT
  public:
    PlaylistWidget(QWidget *parent, PlaylistView *view, PlayqueueModel* model);

    int mode() {return m_view->mode();}

  private:
    void contextMenuEvent ( QContextMenuEvent * event );

  public slots:
    void slot_playqueue_mode_change(int mode);
    void updateFilter(const QString& filter);

  private slots:
    void slot_update_playqueue_actions();
    void slot_update_playqueue_status_info();
    void slot_removeduplicate_changed();

  private :
    PlaylistView       *m_view;
    PlayqueueModel     *m_model;
    QMenu              *m_menu;
};


#endif // _PLAYLISTWIDGET_H_
