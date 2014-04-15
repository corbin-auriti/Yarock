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

#ifndef _PLAYLIST_EDITOR_H_
#define _PLAYLIST_EDITOR_H_

#include "core/mediaitem/mediaitem.h"
#include "widgets/exlineedit.h"

#include <QtGui/QDialog>
#include <QtGui/QLineEdit>
#include <QObject>
#include <QThreadPool>
#include <QList>
#include <QUrl>
#include <QAction>

class PlayqueueModel;
class PlaylistView;
class TaskManager;


/*
********************************************************************************
*                                                                              *
*    Class PlaylistEditor                                                      *
*                                                                              *
********************************************************************************
*/
class PlaylistEditor : public QDialog
{
Q_OBJECT

public:
  PlaylistEditor(QWidget* parent = 0);
  ~PlaylistEditor();

  PlayqueueModel* model() {return m_model;}

  void setMediaList(QList<MEDIA::TrackPtr> list);

  void setPlaylist(MEDIA::PlaylistPtr playlist);

private:
  void create_ui();

  // Playlist Populator Thread
  void playlistAddFiles(const QStringList &files);
  void playlistAddFile(const QString &file);
  void playlistAddUrls(QList<QUrl> listUrl, int playlist_row=-1);

  // Playlist Writer Thread
  void playlistSaveToFile(const QString &filename);
  void playlistSaveToDb(const QString &name);

protected:
  void closeEvent ( QCloseEvent *);
  void hideEvent ( QHideEvent * );

private slots:
  void slot_clear_playlist();
  void slot_add_file();
  void slot_add_dir();
  void slot_add_url();
  void slot_save_to_file();
  void slot_save_to_db();
  void slot_on_buttonBox_rejected();
  void slot_line_edit_change();

private:
  TaskManager           *m_task_manager;

  PlayqueueModel        *m_model;
  PlaylistView          *m_view;
  MEDIA::PlaylistPtr     m_playlist;

  ExLineEdit            *ui_line_edit; /* playlist name */
  QAction               *ui_action_save_file;
  QAction               *ui_action_save_auto;

signals:
  void playlistSaved();
  void closed();
};

#endif // _PLAYLIST_EDITOR_H_
