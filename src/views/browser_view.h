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
#ifndef _BROWSER_VIEW_H_
#define _BROWSER_VIEW_H_

#include "scene_base.h"
#include "views.h"
#include "threadmanager.h"
#include "mediaitem.h"

#include <QGraphicsView>
#include <QHash>
#include <QMap>
#include <QString>
#include <QModelIndex>
#include <QMenu>

class LocalScene;
class StreamScene;
class ContextScene;
class SettingsScene;
class FileScene;

class BottomContainer;


/*
********************************************************************************
*                                                                              *
*    BrowserView                                                               *
*                                                                              *
********************************************************************************
*/
class BrowserView : public QGraphicsView
{
Q_OBJECT
public:
    BrowserView(QWidget *parent);
    
    void setLocalScene(LocalScene *scene);
    void setStreamScene(StreamScene *scene);
    void setContextScene(ContextScene *scene);
    void setSettingsScene(SettingsScene *scene);
    bool eventFilter(QObject *obj, QEvent *ev);

    void active_view(VIEW::Id, QString, QVariant);

public slots:
    void restore_view();

protected:
    void resizeEvent ( QResizeEvent * event );
    void contextMenuEvent ( QContextMenuEvent * event );
    void keyPressEvent ( QKeyEvent * event );
    
private slots:
    void slot_on_search_changed(const QString& );
    void slot_on_model_populated(E_MODEL_TYPE);
    void slot_jump_to_media();
    void slot_check_slider(int);

    void slot_on_menu_index_changed(QModelIndex idx);
    void slot_on_load_new_data(const QString&);    
    void slot_on_history_next_activated();
    void slot_on_history_prev_activated();

private:
    void do_statuswidget_update();
    void show_bottom_panel(QWidget* w);
    void jump_to_media(MEDIA::MediaPtr media);

private :
    struct BrowserParam
    {
      BrowserParam() {
        scroll = 0;
      }
      BrowserParam(VIEW::Id m,const QString& f, const QVariant& d)
      {
        mode   = m;
        filter = f;
        data   = d;
        scroll = 0;
      }

      VIEW::Id  mode;
      QString         filter;
      QVariant        data;
      int             scroll;
    };
    void switch_view(BrowserParam& param);
    void add_history_entry(BrowserParam& param);
    
    /* prev/next navigation */
    QList<BrowserParam>   m_browser_params;
    int                   m_browser_params_idx;
    
    /* scene */
    QMap<VIEW::Id,SceneBase*>   m_scenes;

    /* scroll position */
    QScrollBar           *m_scrollbar;
    QMap<VIEW::Id, int>  scrolls;
   
    bool is_started;
    
    /* bottom widget container */
    BottomContainer    *m_bottomWidget;
    
    /* context menu */
    QMenu      *m_menu;
};

#endif // _BROWSER_VIEW_H_
