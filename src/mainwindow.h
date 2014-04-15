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
#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

// qt
#include <QMainWindow>
#include <QSystemTrayIcon>

class CommandlineOptions;

// player engine
class EngineBase;

// widgets
class PlaylistView;
class PlaylistWidget;
class NowPlayingView;
class BrowserView;
class CentralWidget;
class MenuModel;
class MinimalWidget;
class StatusWidget;
class CentralToolBar;

// Model
class PlayqueueModel;
class LocalTrackModel;
class LocalPlaylistModel;
class StreamModel;
class HistoModel;

// Views
class LocalScene;
class StreamScene;
class SettingsScene;

// Core
class ThreadManager;
class HistoManager;
class DatabaseManager;
class VirtualPlayqueue;
class GlobalShortcuts;

// Dbus & Mpris
class DbusNotification;
class MprisManager;
/*
********************************************************************************
*                                                                              *
*    Class MainWindow                                                          *
*                                                                              *
********************************************************************************
*/
class MainWindow : public QMainWindow
{
Q_OBJECT
    static MainWindow         *INSTANCE;

  public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static MainWindow* instance() { return INSTANCE; }

    void commandlineOptionsHandle(const CommandlineOptions& options);
    
  private:
    /* Mainwindow        */
    void createGui();
    void createActions();
    void connectSlots();

    /* Settings          */
    void reloadSystraySettings();
    void savePlayingTrack();
    void restorePlayingTrack();

    /* Database Method   */
    void removeDatabase();
    void createDatabase();
    void rebuildDatabase();

  private slots:
    /* Mainwindow        */
    void slot_systray_clicked(QSystemTrayIcon::ActivationReason);
    void slot_widget_mode_switch();
    void slot_commandline_received(const QByteArray& serialized_options);
    void slot_on_settings_saved();
    void slot_on_aboutYarock();
    void slot_on_yarock_quit();

    /* Playqueue        */
    void slot_playqueue_add();
    void slot_playqueue_clear();
    void slot_playqueue_save();
    void slot_playqueue_save_auto();
    void slot_playqueue_sort();

    /*  Player           */
    void slot_player_enqueue_next();
    void slot_player_on_state_change();
    void slot_play_from_playqueue();
    void slot_play_from_collection();
    void slot_player_on_track_change ();
    void slot_stop_after_media_triggered();

    void playOrPause();
    void stopPlayer();
    void playNext();
    void playPrev();

    /* Database slots    */
    void slot_database_start();
    void slot_database_dialog();

    /* Last FM           */
    void slot_LastFmLove();

    /* Playlist editor   */
    void slot_open_playlist_editor();
    void slot_delete_playlist_editor();

    /* Jump to item slot */
    void set_enable_jump_to(bool b);

    /* Equalizer         */
    void slot_eq_openDialog();
    void slot_eq_enableChange(bool eqActivated);
    void slot_eq_paramChange(int, QList<int>);

  protected:
    void closeEvent(QCloseEvent *event);

  private:
    bool                 is_first_start;

    enum E_PLAYING_FROM   {FromCollection, FromPlayQueue};

    E_PLAYING_FROM        _playRequestFrom;

    EngineBase            *_player;

    StatusWidget          *_statusWidget;

    CentralToolBar        *_centralToolBar;
    CentralWidget         *_centralWidget;
    MenuModel             *_menuModel;

    PlaylistView          *_playlistView;
    
    PlayqueueModel        *m_playqueue;

    PlaylistWidget        *_playlistwidget;
    NowPlayingView        *_nowplayingview;

    LocalTrackModel       *m_localTrackModel;
    LocalPlaylistModel    *m_localPlaylistModel;
    HistoModel            *m_histoModel;
    LocalScene            *_localScene;
    BrowserView           *m_browserView;

    VirtualPlayqueue      *m_virtual_queue;

    StreamModel           *m_streamModel;
    StreamScene           *_streamScene;
    SettingsScene         *m_settingsScene;

    ThreadManager         *m_thread_manager;

    DatabaseManager       *m_dbManager;

    QIcon                  *m_appIcon;
    QSystemTrayIcon        *m_trayIcon;
    bool                    m_canClose;

    MinimalWidget          *m_minimalwidget;
    HistoManager           *m_histoManager;

    GlobalShortcuts        *m_globalShortcuts;
    DbusNotification       *m_dbus_notifier;
    MprisManager           *m_mpris_manager;

signals:
    void playlistFinished();
};

#endif // _MAINWINDOW_H_
