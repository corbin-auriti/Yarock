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

#ifndef _DATABASE_MANAGER_H_
#define _DATABASE_MANAGER_H_

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>

namespace DB {
  struct S_dbParam {
      S_dbParam() {
         autoRebuild = false;
         checkCover  = true;
         sourcePathList = QStringList();
         groupAlbums = false;
      }

      bool           autoRebuild;
      bool           checkCover;
      QStringList    sourcePathList;
      bool           groupAlbums;
  };
}

/*
********************************************************************************
*                                                                              *
*    Class DatabaseManager                                                     *
*                                                                              *
********************************************************************************
*/
class DatabaseManager : public QObject
{
Q_OBJECT
  static DatabaseManager         *INSTANCE;

  public:
    DatabaseManager();
    static DatabaseManager* instance() { return INSTANCE; }

    void restoreSettings();
    void saveSettings();
    bool isVersionOK();

    QString     DB_FILE();
    QString     DB_ID();
    int         DB_REVISION();

    const DB::S_dbParam&  DB_PARAM(const QString & s) {return m_params[s];}
    const DB::S_dbParam&  DB_PARAM() {return m_params[DB_NAME];}
    const QStringList DB_NAME_LIST() {return m_params.keys();}

    void CLEAR_PARAM() {m_params.clear();}

    void SET_PARAM (const QString&, const DB::S_dbParam&);

    bool        multiDb; //! multi-database support enable
    QString     DB_NAME; //! current db name

  private:
    QSettings  *s;
    QMap<QString, DB::S_dbParam> m_params;

  signals:
    void settingsChanged();
};

#endif // _DATABASE_MANAGER_H_
