
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wall -Wno-sign-compare")

option(BUILD_WERROR "Build with -Werror" ON)

if(BUILD_WERROR)
  if (LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  endif (LINUX)
endif(BUILD_WERROR)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_BINARY_DIR})
include_directories(${QTSINGLEAPPLICATION_INCLUDE_DIRS})
include_directories(${QXT_INCLUDE_DIRS})

#cmake_policy(SET CMP0011 NEW)


# ------------------------------------------------------------------------------
#  SOURCES
# ------------------------------------------------------------------------------
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/core
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/database
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/history
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/mediaitem
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/mediasearch
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/player
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/player/vlc
                     ${CMAKE_CURRENT_SOURCE_DIR}/covers
                     ${CMAKE_CURRENT_SOURCE_DIR}/infosystem
                     ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services
                     ${CMAKE_CURRENT_SOURCE_DIR}/dbus
                     ${CMAKE_CURRENT_SOURCE_DIR}/models
                     ${CMAKE_CURRENT_SOURCE_DIR}/models/local
                     ${CMAKE_CURRENT_SOURCE_DIR}/models/online
                     ${CMAKE_CURRENT_SOURCE_DIR}/models/stream   
                     ${CMAKE_CURRENT_SOURCE_DIR}/models/filesystem                     
                     ${CMAKE_CURRENT_SOURCE_DIR}/online
                     ${CMAKE_CURRENT_SOURCE_DIR}/playqueue
                     ${CMAKE_CURRENT_SOURCE_DIR}/settings
                     ${CMAKE_CURRENT_SOURCE_DIR}/smartplaylist
                     ${CMAKE_CURRENT_SOURCE_DIR}/views
                     ${CMAKE_CURRENT_SOURCE_DIR}/views/about
                     ${CMAKE_CURRENT_SOURCE_DIR}/views/context
                     ${CMAKE_CURRENT_SOURCE_DIR}/views/filesystem
                     ${CMAKE_CURRENT_SOURCE_DIR}/views/local
                     ${CMAKE_CURRENT_SOURCE_DIR}/views/settings
                     ${CMAKE_CURRENT_SOURCE_DIR}/views/stream
                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets
                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs
                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/equalizer
                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/nowplaying
                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main
                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/popupcompleter
)

SET(YAROCK_SOURCES 
           ${CMAKE_CURRENT_SOURCE_DIR}/global_actions.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/commandlineoptions.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/debug.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/networkaccess.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/global_shortcuts.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/settings.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/threadmanager.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/utilities.cpp
)

set (YAROCK_SOURCES 
           ${YAROCK_SOURCES}
           ${CMAKE_CURRENT_SOURCE_DIR}/core/database/database.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/database/databasebuilder.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/database/databasemanager.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/core/history/histomanager.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/history/histomanager.cpp

           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediaitem/mediaitem.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediaitem/mediaitem_replaygain.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediaitem/mediaitem_statistic.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediaitem/mediamimedata.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediaitem/playlist_parser.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediasearch/media_search.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediasearch/media_search_engine.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediasearch/media_search_dialog.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediasearch/search_query.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediasearch/search_query_widget.cpp

           ${CMAKE_CURRENT_SOURCE_DIR}/core/player/engine.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/player/engine_base.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/player/engine_phonon.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/player/engine_vlc.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/player/vlc/vlc_lib.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/core/player/vlc/vlc_media.cpp
)

set (YAROCK_SOURCES 
           ${YAROCK_SOURCES}           
           ${CMAKE_CURRENT_SOURCE_DIR}/covers/covercache.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/covers/covertask.cpp             
)           

set (YAROCK_SOURCES 
           ${YAROCK_SOURCES}           
           ${CMAKE_CURRENT_SOURCE_DIR}/dbus/dbusnotification.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/dbus/mpris_manager.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/dbus/mpris2.cpp        
)    

set (YAROCK_SOURCES 
           ${YAROCK_SOURCES}           
           ${CMAKE_CURRENT_SOURCE_DIR}/models/local/local_track_model.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/models/local/local_track_populator.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/models/local/histo_model.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/models/local/local_playlist_model.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/models/local/local_playlist_populator.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/models/stream/service_base.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/models/stream/service_shoutcast.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/models/stream/service_tunein.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/models/stream/service_xspf.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/models/stream/service_dirble.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/models/stream/stream_model.cpp 
           ${CMAKE_CURRENT_SOURCE_DIR}/models/filesystem/file_model.cpp
)

set (YAROCK_SOURCES 
           ${YAROCK_SOURCES}           
           ${CMAKE_CURRENT_SOURCE_DIR}/online/lastfm.cpp
)
   
set (YAROCK_SOURCES 
           ${YAROCK_SOURCES}           
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playqueue_base.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playlistdbwriter.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playlisteditor.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playqueue_model.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playqueue_proxymodel.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playlistpopulator.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playlistview.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playlistwidget.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playlistwriter.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/task_manager.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/virtual_playqueue.cpp                
)

   
set (YAROCK_SOURCES 
           ${YAROCK_SOURCES}           
           ${CMAKE_CURRENT_SOURCE_DIR}/smartplaylist/smartplaylist.cpp        
)
      
set (YAROCK_SOURCES 
           ${YAROCK_SOURCES}                            
           ${CMAKE_CURRENT_SOURCE_DIR}/views/bottom_container.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/views/browser_view.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/views/item_button.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/item_common.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/item_menu.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/views/scene_base.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/views/about/about_scene.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/qpixmapfilter.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/local_item.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/local_scene.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/local_scene_dashboard.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/local_scene_playing.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/local_scene_playqueue.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/local_scene_rate.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/context/context_item.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/context/context_widget.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/context/context_scene.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/context/lyricseditor.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/stream/stream_item.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/stream/stream_scene.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/stream/stream_loader.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/views/filesystem/file_scene.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/filesystem/file_graphicitem.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/settings/settings_scene.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/views/settings/settings_widget.cpp
)
                
set (YAROCK_SOURCES 
           ${YAROCK_SOURCES}           
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs/dialog_base.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs/addstreamdialog.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs/dboperationdialog.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs/mediaitem_edit_dialog.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs/shortcutdialog.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs/filedialog.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/centraltoolbar.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/centralwidget.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/maintoolbutton.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/menumodel.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/menuview.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/menuwidget.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/menubar.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/nowplaying/nowplayingview.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/popupcompleter/popup_item.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/popupcompleter/popup_model.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/popupcompleter/popup_view.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/popupcompleter/search_popup.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/audiocontrols.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/customsplitter.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/equalizer/equalizer_dialog.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/equalizer/equalizer_preset.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/equalizer/equalizer_slider.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/exlineedit.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/minimalwidget.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/playertoolbar.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/ratingwidget.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/seekslider.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/spacer.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/splittercollapser.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/statuswidget.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/sort_widget.cpp            
)

set (YAROCK_SOURCES 
           ${YAROCK_SOURCES}           
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/info_service.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/info_system.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/info_system_worker.cpp           
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/service_discogs.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/service_echonest.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/service_lastfm.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/service_musicbrainz.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/service_lyrics.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/ultimatelyricsprovider.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/ultimatelyricsreader.cpp
)


# ------------------------------------------------------------------------------
#  YAROCK_HEADERS
# ------------------------------------------------------------------------------
SET(YAROCK_HEADERS 
           ${CMAKE_CURRENT_SOURCE_DIR}/commandlineoptions.h
           ${CMAKE_CURRENT_SOURCE_DIR}/debug.h
           ${CMAKE_CURRENT_SOURCE_DIR}/global_actions.h
           ${CMAKE_CURRENT_SOURCE_DIR}/global_shortcuts.h
           ${CMAKE_CURRENT_SOURCE_DIR}/main.h
           ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h
           ${CMAKE_CURRENT_SOURCE_DIR}/networkaccess.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/settings.h
           ${CMAKE_CURRENT_SOURCE_DIR}/threadmanager.h
           ${CMAKE_CURRENT_SOURCE_DIR}/utilities.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/database/database.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/database/databasebuilder.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/database/databasemanager.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/core/history/histomanager.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/history/histomanager.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediaitem/mediaitem.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediaitem/mediaitem_replaygain.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediaitem/mediaitem_statistic.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediaitem/mediamimedata.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediaitem/playlist_parser.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediasearch/media_search.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediasearch/media_search_engine.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediasearch/media_search_dialog.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediasearch/search_query.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/mediasearch/search_query_widget.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/player/engine.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/player/engine_base.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/player/engine_phonon.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/player/engine_vlc.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/player/vlc/vlc_lib.h
           ${CMAKE_CURRENT_SOURCE_DIR}/core/player/vlc/vlc_media.h
           ${CMAKE_CURRENT_SOURCE_DIR}/covers/covercache.h
           ${CMAKE_CURRENT_SOURCE_DIR}/covers/covertask.h             
           ${CMAKE_CURRENT_SOURCE_DIR}/dbus/dbusnotification.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/dbus/mpris2.h   
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/info_service.h
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/info_system.h
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/info_system_worker.h
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/service_discogs.h
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/service_echonest.h
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/service_lastfm.h
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/service_musicbrainz.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/service_lyrics.h  
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/ultimatelyricsprovider.h
           ${CMAKE_CURRENT_SOURCE_DIR}/infosystem/services/ultimatelyricsreader.h
           ${CMAKE_CURRENT_SOURCE_DIR}/models/local/local_track_model.h
           ${CMAKE_CURRENT_SOURCE_DIR}/models/local/local_track_populator.h
           ${CMAKE_CURRENT_SOURCE_DIR}/models/local/histo_model.h
           ${CMAKE_CURRENT_SOURCE_DIR}/models/local/local_playlist_model.h
           ${CMAKE_CURRENT_SOURCE_DIR}/models/local/local_playlist_populator.h
           ${CMAKE_CURRENT_SOURCE_DIR}/models/stream/service_base.h
           ${CMAKE_CURRENT_SOURCE_DIR}/models/stream/service_shoutcast.h
           ${CMAKE_CURRENT_SOURCE_DIR}/models/stream/service_tunein.h
           ${CMAKE_CURRENT_SOURCE_DIR}/models/stream/service_xspf.h
           ${CMAKE_CURRENT_SOURCE_DIR}/models/stream/service_dirble.h
           ${CMAKE_CURRENT_SOURCE_DIR}/models/stream/stream_model.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/models/filesystem/file_model.h
           ${CMAKE_CURRENT_SOURCE_DIR}/online/lastfm.h
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playqueue_base.h
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playqueue_model.h
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playlistdbwriter.h
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playlisteditor.h
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playlistpopulator.h
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playqueue_proxymodel.h
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playlistview.h
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playlistwidget.h
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/playlistwriter.h
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/task_manager.h  
           ${CMAKE_CURRENT_SOURCE_DIR}/playqueue/virtual_playqueue.h  
           ${CMAKE_CURRENT_SOURCE_DIR}/smartplaylist/smartplaylist.h        
           ${CMAKE_CURRENT_SOURCE_DIR}/views/bottom_container.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/item_common.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/views/item_button.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/views/item_menu.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/browser_view.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/views/scene_base.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/views/about/about_scene.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/qpixmapfilter.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/local_item.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/local_scene.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/local_scene_dashboard.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/local_scene_playing.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/local_scene_playqueue.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/local/local_scene_rate.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/context/context_item.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/context/context_widget.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/context/context_scene.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/context/lyricseditor.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/stream/stream_item.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/stream/stream_scene.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/stream/stream_loader.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/filesystem/file_scene.h
           ${CMAKE_CURRENT_SOURCE_DIR}/views/filesystem/file_graphicitem.h  
           ${CMAKE_CURRENT_SOURCE_DIR}/views/settings/settings_scene.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/views/settings/settings_widget.h             
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs/dialog_base.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs/addstreamdialog.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs/dboperationdialog.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs/mediaitem_edit_dialog.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs/shortcutdialog.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs/filedialog.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/centraltoolbar.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/centralwidget.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/maintoolbutton.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/menumodel.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/menuview.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/menuwidget.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main/menubar.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/nowplaying/nowplayingview.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/popupcompleter/popup_item.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/popupcompleter/popup_model.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/popupcompleter/popup_view.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/popupcompleter/search_popup.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/audiocontrols.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/customsplitter.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/equalizer/equalizer_dialog.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/equalizer/equalizer_preset.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/equalizer/equalizer_slider.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/exlineedit.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/minimalwidget.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/playertoolbar.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/ratingwidget.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/seekslider.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/spacer.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/splittercollapser.h
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/statuswidget.h           
           ${CMAKE_CURRENT_SOURCE_DIR}/widgets/sort_widget.h  
)

# ------------------------------------------------------------------------------
#  RESSOURCE
# ------------------------------------------------------------------------------
# MPRIS 2.0 DBUS interfaces
qt4_add_dbus_adaptor(YAROCK_SOURCES
      dbus/org.mpris.MediaPlayer2.Player.xml
      dbus/mpris2.h Mpris2 mpris2Player Mpris2Player)
qt4_add_dbus_adaptor(YAROCK_SOURCES
      dbus/org.mpris.MediaPlayer2.xml
      dbus/mpris2.h Mpris2 mpris2Root Mpris2Root)
      
      
# ------------------------------------------------------------------------------
#  RESSOURCE
# ------------------------------------------------------------------------------
SET(YAROCK_RESOURCES ../yarock.qrc)
QT4_ADD_RESOURCES(YAROCK_RESOURCES_RCC ${YAROCK_RESOURCES})


# ------------------------------------------------------------------------------
#  TRANSLATION
# ------------------------------------------------------------------------------
option (UPDATE_TRANSLATIONS "Update source translation ../translation/*.ts files")
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${YAROCK_SOURCES})

if (UPDATE_TRANSLATIONS)
  QT4_CREATE_TRANSLATION(YAROCK_QM_FILES ${FILES_TO_TRANSLATE} ${TS_FILES})
else (UPDATE_TRANSLATIONS)
  QT4_ADD_TRANSLATION(YAROCK_QM_FILES ${TS_FILES})
endif (UPDATE_TRANSLATIONS)

# # ------------------------------------------------------------------------------
# #  BUILDING
# # ------------------------------------------------------------------------------
add_definitions(${QT_DEFINITIONS})
   

ADD_EXECUTABLE(yarock ${YAROCK_SOURCES} ${YAROCK_RESOURCES_RCC} ${YAROCK_QM_FILES})
set_target_properties(yarock PROPERTIES AUTOMOC TRUE)

#to include translation file in binary
set(EXECUTABLE_OUTPUT_PATH ..)

TARGET_LINK_LIBRARIES(yarock
  X11
  ${QT_LIBRARIES}
  ${TAGLIB_LIBRARIES}
  ${QJSON_LIBRARIES}
  ${QTSINGLECOREAPPLICATION_LIBRARIES} ${QTSINGLEAPPLICATION_LIBRARIES}
  ${QXT_LIBRARIES}  
)

if (ENABLE_VLC)
  TARGET_LINK_LIBRARIES(yarock ${LIBVLC_LIBRARY})
endif (ENABLE_VLC)  

if (ENABLE_PHONON)
  TARGET_LINK_LIBRARIES(yarock ${PHONON_LIBS})
endif (ENABLE_PHONON)

# ------------------------------------------------------------------------------
#  INSTALL
# ------------------------------------------------------------------------------
# -- install binary file
install( TARGETS yarock DESTINATION /usr/bin/ )

# -- install translations files
install(FILES ${YAROCK_QM_FILES} DESTINATION /usr/share/locale/yarock/)
 