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

#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <QString>
#include <QSqlDatabase>

/*
********************************************************************************
*                                                                              *
*    Class Database                                                            *
*                                                                              *
********************************************************************************
*/
class Database
{
  public:
    Database();
    ~Database();

    QSqlDatabase* sqlDb();

    bool connect(bool create = false);
    static void close();

    QString name() {return m_name;}

    void create();

  private:
    QString          m_name;
    QString          m_database_path;
    int              m_revision;
    QString          m_connection_name;
};

#endif // _DATABASE_H_
