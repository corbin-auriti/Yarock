# ------------------------------------------------------------------------------
#  YAROCK CMAKE FILE
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.6)
set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

PROJECT( yarock )
set(YAROCK_MAJOR_VERSION 0)
set(YAROCK_MINOR_VERSION 9)
set(YAROCK_PATCH_VERSION 66)
set(YAROCK_VERSION ${YAROCK_MAJOR_VERSION}.${YAROCK_MINOR_VERSION}.${YAROCK_PATCH_VERSION})

# ------------------------------------------------------------------------------
#  build def
# ------------------------------------------------------------------------------
#ADD_DEFINITIONS( -DTEST_FLAG )
ADD_DEFINITIONS(-Wall -Wextra )

# ------------------------------------------------------------------------------
#  OPTION
# ------------------------------------------------------------------------------
OPTION(ENABLE_PHONON "Enable Phonon library" ON)
OPTION(ENABLE_VLC "Enable VLC library" OFF)

if (ENABLE_VLC)
    add_definitions( -DENABLE_VLC)
endif (ENABLE_VLC)

if (ENABLE_PHONON)
    add_definitions( -DENABLE_PHONON)
endif (ENABLE_PHONON)


# ------------------------------------------------------------------------------
#  EXTERNAL PACKAGE
# ------------------------------------------------------------------------------
FIND_PACKAGE( Qt4 4.7.0 REQUIRED QtCore QtGui QtXml QtNetwork QtSql QtDbus)

if (ENABLE_VLC)
  FIND_PACKAGE( LIBVLC REQUIRED)
endif (ENABLE_VLC)

if (ENABLE_PHONON)
  FIND_PACKAGE( Phonon REQUIRED )
endif (ENABLE_PHONON)
 
FIND_PACKAGE( Taglib 1.6.0 REQUIRED )
FIND_PACKAGE( QJSON REQUIRED )

if(TAGLIB_FOUND)
include(CheckTagLibFileName)
check_taglib_filename(COMPLEX_TAGLIB_FILENAME)
endif (TAGLIB_FOUND)

# ------------------------------------------------------------------------------
#  INCLUDES
# ------------------------------------------------------------------------------
INCLUDE(${QT_USE_FILE})

INCLUDE_DIRECTORIES( ${QT_INCLUDES} )

if (ENABLE_VLC)
  INCLUDE_DIRECTORIES( ${LIBVLC_INCLUDE_DIR} )
endif (ENABLE_VLC)

if (ENABLE_PHONON)
  INCLUDE_DIRECTORIES( ${PHONON_INCLUDES} )
endif (ENABLE_PHONON)

INCLUDE_DIRECTORIES( ${TAGLIB_INCLUDES} )
INCLUDE_DIRECTORIES( ${QJSON_INCLUDE_DIR} )
 
# ------------------------------------------------------------------------------
#  SUBDIRECTORIES
# ------------------------------------------------------------------------------

option(USE_SYSTEM_QTSINGLEAPPLICATION "Use system QtSingleApplication library." OFF)
if(USE_SYSTEM_QTSINGLEAPPLICATION)
  find_path(QTSINGLEAPPLICATION_INCLUDE_DIRS qtsingleapplication.h PATH_SUFFIXES QtSolutions)
  find_library(QTSINGLEAPPLICATION_LIBRARIES QtSolutions_SingleApplication-2.6)
  find_library(QTSINGLECOREAPPLICATION_LIBRARIES QtSolutions_SingleCoreApplication-2.6)
else(USE_SYSTEM_QTSINGLEAPPLICATION)
  add_subdirectory(src3party/qtsingleapplication)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src3party/qtsingleapplication)
  set(QTSINGLEAPPLICATION_LIBRARIES qtsingleapplication)
endif(USE_SYSTEM_QTSINGLEAPPLICATION)

option(USE_SYSTEM_QXT "Use system Qxt library" OFF)
if (USE_SYSTEM_QXT)
  find_path(QXTCORE_INCLUDE_DIRS qxtglobal.h PATH_SUFFIXES QxtCore)
  find_path(QXTGUI_INCLUDE_DIRS qxtglobalshortcut.h PATH_SUFFIXES QxtGui)
  set(QXT_INCLUDE_DIRS ${QXTCORE_INCLUDE_DIRS} ${QXTGUI_INCLUDE_DIRS})
  # We only need its header. We don't need to link to QxtCore.
  find_library(QXT_LIBRARIES QxtGui)
else (USE_SYSTEM_QXT)
  add_subdirectory(src3party/qxt)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src3party/qxt)
  set(QXT_LIBRARIES qxt) 
endif (USE_SYSTEM_QXT)


# ------------------------------------------------------------------------------
#  TRANSLATIONS
# ------------------------------------------------------------------------------
# Handle Translations, take all * files from trans directory.
file( GLOB TRANS_FILES ${CMAKE_SOURCE_DIR}/translation/yarock_*.ts)
set(TS_FILES ${TRANS_FILES})

SET(UPDATE_TRANSLATIONS OFF) # no ts update
#SET(UPDATE_TRANSLATIONS ON) # ts update

# ------------------------------------------------------------------------------
#  SOURCES
# ------------------------------------------------------------------------------
add_subdirectory(src)

# ------------------------------------------------------------------------------
#  INSTALLATION
# ------------------------------------------------------------------------------
# -- install destop file
install(FILES data/yarock.desktop  DESTINATION /usr/share/applications/)

# -- install icons files
install(FILES icon/16x16/yarock.png DESTINATION share/icons/hicolor/16x16/apps/ RENAME application-x-yarock.png)
install(FILES icon/32x32/yarock.png DESTINATION share/icons/hicolor/32x32/apps/ RENAME application-x-yarock.png)
install(FILES icon/48x48/yarock.png DESTINATION share/icons/hicolor/48x48/apps/ RENAME application-x-yarock.png)
install(FILES icon/64x64/yarock.png DESTINATION share/icons/hicolor/64x64/apps/ RENAME application-x-yarock.png)
install(FILES icon/128x128/yarock.png DESTINATION share/icons/hicolor/128x128/apps/ RENAME application-x-yarock.png)


# ------------------------------------------------------------------------------
#  UN-INSTALL SUPPORT
# ------------------------------------------------------------------------------

 