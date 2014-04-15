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


#include "databasemanager.h"
#include "database.h"
#include "utilities.h"
#include "debug.h"

#include <QtCore>

#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QtSql/QSqlDatabase>


#define CST_DB_REVISION     17;

DatabaseManager* DatabaseManager::INSTANCE = 0;
/*
********************************************************************************
*                                                                              *
*    Class DatabaseManager                                                     *
*                                                                              *
********************************************************************************
*/

DatabaseManager::DatabaseManager()
{
    INSTANCE = this;

    s = new QSettings(UTIL::CONFIGFILE,QSettings::IniFormat,this);
    DB_NAME       = "";
    multiDb       = false;

    restoreSettings();
}


//! --------- DatabaseManager::restoreSettings ---------------------------------
void DatabaseManager::restoreSettings()
{
    m_params.clear();

    s->beginGroup("Databases");
    if(s->contains("multiDb"))
      multiDb = s->value("multiDb").toBool();

    DB_NAME = s->value("dbCurrent").toString();

    int count = s->beginReadArray("dbEntry");
    //Debug::debug() << "- DatabaseManager count " << count;

    for (int i=0 ; i<count ; ++i) {
      s->setArrayIndex(i);

      QString name =  s->value("name").toString();
      m_params[name].autoRebuild     = s->value("autorebuild", false).toBool();
      m_params[name].checkCover      = s->value("checkcover", true).toBool();
      m_params[name].sourcePathList  << s->value("sourcepath").toStringList();
      m_params[name].groupAlbums     =  s->value("groupAlbums").toBool();
      
      Debug::debug() << "- DatabaseManager name " << name;
      Debug::debug() << "- DatabaseManager sourcePathList " << m_params[name].sourcePathList;
      Debug::debug() << "- DatabaseManager autorebuild " << m_params[name].autoRebuild;
      Debug::debug() << "- DatabaseManager checkcover "  << m_params[name].checkCover;
      Debug::debug() << "- DatabaseManager groupAlbums " << m_params[name].groupAlbums;

    }
    s->endArray();
    s->endGroup();

    if(DB_NAME.isEmpty() || !m_params.contains(DB_NAME)) {
      if(m_params.size() > 0) {
        QStringList list = m_params.keys();
        DB_NAME = list.first();
      }
      else {
        Debug::debug() << "- DatabaseManager : DB_NAME empty set collection";
        DB_NAME = "collection";
        m_params[DB_NAME].autoRebuild    = false;
        m_params[DB_NAME].checkCover     = true;
        m_params[DB_NAME].sourcePathList.clear();
        m_params[DB_NAME].groupAlbums    = false;
      }
    }
}


//! --------- DatabaseManager::saveSettings ------------------------------------
void DatabaseManager::saveSettings()
{
    Debug::debug() << "- DatabaseManager saveSettings";

    s->beginGroup("Databases");
    s->setValue("multiDb", multiDb);
    s->setValue("dbCurrent", DB_NAME);
    s->beginWriteArray("dbEntry", m_params.count());
    int i=0;
    foreach (const QString& name, m_params.keys()) {
      s->setArrayIndex(i++);
      s->setValue("name", name);
      s->setValue("autorebuild", m_params[name].autoRebuild );
      s->setValue("checkcover",  m_params[name].checkCover);
      s->setValue("sourcepath",  m_params[name].sourcePathList);
      s->setValue("groupAlbums", m_params[name].groupAlbums);
      //Debug::debug() << "- DatabaseManager -> saveSettings" << m_params[name].sourcePathList;
    }
    s->endArray();
    s->endGroup();
    s->sync();

    emit settingsChanged();
}


void DatabaseManager::SET_PARAM (const QString& name, const DB::S_dbParam &param)
{
      m_params[name].autoRebuild     = param.autoRebuild;
      m_params[name].checkCover      = param.checkCover;
      m_params[name].sourcePathList  = QStringList() << param.sourcePathList;
      m_params[name].groupAlbums     = param.groupAlbums;
}


//! --------- DatabaseManager::DB_FILE -----------------------------------------
QString DatabaseManager::DB_FILE()
{
    return QString(UTIL::CONFIGDIR + "/" + DB_ID() + ".db");
}

//! --------- DatabaseManager::DB_ID -------------------------------------------
QString DatabaseManager::DB_ID()
{
    if(DB_NAME.isEmpty())
      return QString();

    return QString(QCryptographicHash::hash(DB_NAME.toUtf8().constData(), QCryptographicHash::Sha1).toHex());
}


//! --------- Revision Management -----------------------------------------------
int DatabaseManager::DB_REVISION()
{
    return CST_DB_REVISION;
}



bool DatabaseManager::isVersionOK()
{
    bool versionOK;

    Database db;
    if(!db.connect())
      return false;

    QSqlQuery query("SELECT value FROM db_attribute WHERE name='version' LIMIT 1;", *db.sqlDb());
    query.next();

    if (query.isValid()) {
        int currentRev =  query.value(0).toInt();
        Debug::debug() << "- DatabaseManager -> database revision found : " << currentRev;
        if (currentRev != DB_REVISION())
          versionOK = false;
        else
          versionOK = true;
    }
    else {
        Debug::debug() << "- DatabaseManager -> database reading revision failed ";
        versionOK = false;
    }

    return versionOK;
}
