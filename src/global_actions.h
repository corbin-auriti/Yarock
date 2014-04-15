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
#ifndef _GLOBAL_ACTIONS_H_
#define _GLOBAL_ACTIONS_H_

#include "views.h"

//! Qt
#include <QString>
#include <QAction>
#include <QMap>

/*
********************************************************************************
*                                                                              *
*    ACTION enumerate                                                          *
*                                                                              *
********************************************************************************
*/
    enum ENUM_ACTION {
         APP_QUIT,
         APP_SHOW_YAROCK_ABOUT,
         APP_SHOW_PLAYQUEUE,
         APP_SHOW_MENU,
         APP_SHOW_NOW_PLAYING,
         APP_MODE_COMPACT,
         APP_MODE_NORMAL,

         APP_ENABLE_SEARCH_POPUP,

         BROWSER_PREV,
         BROWSER_NEXT,
         BROWSER_UP, 
         BROWSER_ITEM_RATING_CLICK,
         BROWSER_LOCAL_ITEM_MOUSE_MOVE,
         BROWSER_STREAM_ITEM_MOUSE_MOVE,
         BROWSER_DIR_ITEM_MOUSE_MOVE,
         BROWSER_JUMP_TO_ARTIST,
         BROWSER_JUMP_TO_ALBUM,
         BROWSER_JUMP_TO_TRACK,
         BROWSER_JUMP_TO_MEDIA,

         ENGINE_PLAY,
         ENGINE_STOP,
         ENGINE_PLAY_PREV,
         ENGINE_PLAY_NEXT,
         ENGINE_VOL_MUTE,
         ENGINE_AUDIO_EQ,

         PLAYQUEUE_ADD_FILE,
         PLAYQUEUE_ADD_DIR,
         PLAYQUEUE_ADD_URL,
         PLAYQUEUE_CLEAR,
         PLAYQUEUE_SAVE,
         PLAYQUEUE_AUTOSAVE,
         PLAYQUEUE_REMOVE_ITEM,
         PLAYQUEUE_STOP_AFTER,
         PLAYQUEUE_JUMP_TO_TRACK,
         PLAYQUEUE_SORT,
         PLAYQUEUE_REMOVE_DUPLICATE,

         PLAYQUEUE_MODE_TITLE,
         PLAYQUEUE_MODE_ALBUM,
         PLAYQUEUE_MODE_ARTIST,
         PLAYQUEUE_MODE_EXTENDED,

         DIALOG_PLAYLIST_EDITOR,
         DIALOG_DB_OPERATION,
         TASK_COVER_SEARCH,

         LASTFM_LOVE,
         LASTFM_LOVE_NOW_PLAYING
    };


/*
********************************************************************************
*                                                                              *
*    Class GlobalActions                                                       *
*                                                                              *
********************************************************************************
*/
#define ACTIONS() (GlobalActions::instance()->actions())

class GlobalActions
{
    static GlobalActions* INSTANCE;

public :
    GlobalActions();

    static GlobalActions* instance() { return INSTANCE; }

    QMap<ENUM_ACTION, QAction*>* actions() { return m_actions; }

private:
    QMap<ENUM_ACTION, QAction*>* m_actions;
};


#endif // _GLOBAL_ACTIONS_H_
