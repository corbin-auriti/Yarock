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

#include "playlistwidget.h"
#include "playqueue_proxymodel.h"
#include "widgets/statuswidget.h"
#include "widgets/main/centraltoolbar.h"
#include "settings.h"

#include "global_actions.h"
#include "debug.h"


/*
********************************************************************************
*                                                                              *
*    Class PlaylistWidget                                                      *
*                                                                              *
********************************************************************************
*/
PlaylistWidget::PlaylistWidget(QWidget *parent,PlaylistView *view, PlayqueueModel* model) : QWidget(parent)
{
    //! init
    m_menu     = 0;
    m_view     = view;
    m_model    = model;

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setSpacing(0);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->addWidget(m_view);

    //! connect
    connect(m_model, SIGNAL(updated()), this, SLOT(slot_update_playqueue_status_info()));
    connect(m_model, SIGNAL(updated()), this, SLOT(slot_update_playqueue_actions()));
    connect(m_view, SIGNAL(selectionChanged()), this, SLOT(slot_update_playqueue_actions()));
    connect(CentralToolBar::instance(), SIGNAL(playqueueFilterActivated(const QString&)),this, SLOT(updateFilter(const QString&)));

    //! restore last mode for display
    m_view->setMode(SETTINGS()->_playqueueMode);

    //! restore state
    slot_update_playqueue_actions();
    
    
    connect(ACTIONS()->value(PLAYQUEUE_REMOVE_DUPLICATE), SIGNAL(triggered()), SLOT(slot_removeduplicate_changed()));
}


/*******************************************************************************
    updateFilter
*******************************************************************************/
void PlaylistWidget::updateFilter(const QString& filter)
{
    Debug::debug() << "PlaylistWidget --> updateFilter  " << filter;

    m_model->proxy()->setFilterRegExp(filter);
}

/*******************************************************************************
    slot_update_playqueue_status_info
*******************************************************************************/
void PlaylistWidget::slot_update_playqueue_status_info()
{
    //Debug::debug() << "--- PlaylistWidget-->slot_update_plaqueue_status_info";

    const int rowCount = m_model->rowCount(QModelIndex());
    QString duree      = m_model->queueDuration();

    QString playlistInfo;

    if (rowCount < 1)
      playlistInfo = tr("Empty Playlist");
    else
      playlistInfo = QString(tr("Playlist : <b>%1</b> tracks - %2")).arg(QString::number(rowCount), duree);

    StatusWidget::instance()->startShortMessage(playlistInfo, STATUS::TYPE_PLAYQUEUE, 2500);
}


/*******************************************************************************
    slot_update_playqueue_actions
*******************************************************************************/
void PlaylistWidget::slot_update_playqueue_actions()
{
    //Debug::debug() << "--- PlaylistWidget-->sot_update_plaqueue_actions";
    const bool isPlaylistEmpty = m_model->rowCount(QModelIndex()) < 1;

    ACTIONS()->value(PLAYQUEUE_CLEAR)->setEnabled(!isPlaylistEmpty);
    ACTIONS()->value(PLAYQUEUE_SAVE)->setEnabled(!isPlaylistEmpty);
    ACTIONS()->value(PLAYQUEUE_AUTOSAVE)->setEnabled(!isPlaylistEmpty);
    ACTIONS()->value(PLAYQUEUE_REMOVE_ITEM)->setEnabled((!isPlaylistEmpty) && m_view->isTrackSelected());
}

/*******************************************************************************
    contextMenuEvent
*******************************************************************************/
void PlaylistWidget::contextMenuEvent(QContextMenuEvent* e)
{
    //Debug::debug() << "--- PlaylistWidget-->contextMenuEvent";
    QMap<ENUM_ACTION, QAction*> *actions = ACTIONS();

    /* handle stop after track action */
    QModelIndex idx           = m_view->indexAt(e->pos());
    QModelIndex source_idx    = m_model->proxy()->mapToSource(idx);
    const int right_click_row = source_idx.row();

    if( right_click_row != -1) {
      actions->value(PLAYQUEUE_STOP_AFTER)->setEnabled(true);

      MEDIA::TrackPtr right_click_track = m_model->trackAt(right_click_row);

      QVariant v;
      v.setValue(static_cast<MEDIA::TrackPtr>(right_click_track));
      (actions->value(PLAYQUEUE_STOP_AFTER))->setData(v);

      actions->value(PLAYQUEUE_STOP_AFTER)->setChecked( right_click_track->isStopAfter );
    }
    else {
      actions->value(PLAYQUEUE_STOP_AFTER)->setEnabled(false);
      actions->value(PLAYQUEUE_STOP_AFTER)->setChecked(false);
    }

    /* build menu */
    if (!m_menu) {
      m_menu = new QMenu(this);

      QMenu *m = m_menu->addMenu(tr("Choose playlist mode"));
      m_menu->addSeparator();
      m_menu->addAction(actions->value(PLAYQUEUE_ADD_FILE));
      m_menu->addAction(actions->value(PLAYQUEUE_ADD_DIR));
      m_menu->addAction(actions->value(PLAYQUEUE_ADD_URL));
      m_menu->addAction(actions->value(PLAYQUEUE_CLEAR));
      m_menu->addAction(actions->value(PLAYQUEUE_SAVE));
      m_menu->addAction(actions->value(PLAYQUEUE_AUTOSAVE));
      m_menu->addAction(actions->value(PLAYQUEUE_REMOVE_ITEM));
      m_menu->addAction(actions->value(PLAYQUEUE_STOP_AFTER));
      m_menu->addAction(actions->value(PLAYQUEUE_REMOVE_DUPLICATE));
      m_menu->addAction(actions->value(PLAYQUEUE_JUMP_TO_TRACK));

      QActionGroup* group = new QActionGroup(this);
        /* WARNING actions are already connected with Mainwindows */
        group->addAction(actions->value(PLAYQUEUE_MODE_TITLE));
        group->addAction(actions->value(PLAYQUEUE_MODE_ALBUM));
        group->addAction(actions->value(PLAYQUEUE_MODE_ARTIST));
        group->addAction(actions->value(PLAYQUEUE_MODE_EXTENDED));
      m->addActions(group->actions());

      m_menu->addSeparator();
      m_menu->addAction(actions->value(LASTFM_LOVE));
      m_menu->addSeparator();
      m_menu->addAction(actions->value(APP_SHOW_PLAYQUEUE));
    }

    /* build menu */
    m_menu->popup(e->globalPos());
    e->accept();
}

void PlaylistWidget::slot_removeduplicate_changed()
{
    if( ACTIONS()->value(PLAYQUEUE_REMOVE_DUPLICATE)->isChecked() )
      m_model->removeDuplicate();  
}


/*******************************************************************************
    slot_playqueue_mode_change
*******************************************************************************/
// public slots (ré-utiliser par le menu du centraltoolbar)
void PlaylistWidget::slot_playqueue_mode_change(int mode)
{
    // il faut forcer le repaint de la playlistview
    m_view->setMode(mode);
    m_model->signalUpdate();
}
