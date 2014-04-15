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
#ifndef _COVER_TASK_
#define _COVER_TASK_

 #include "info_system.h"

#include <QStringList>
#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QTimer>

/*
********************************************************************************
*                                                                              *
*    Class CoverTask                                                           *
*                                                                              *
********************************************************************************
*/
class CoverTask : public QObject
{
Q_OBJECT
 public:
    explicit CoverTask(QObject *parent = 0);
    ~CoverTask();
    void setExit(bool b) {m_exit = b;}
    void setRequest(const QString& artist, const QString& album);

    bool isRunning() {return m_isRunning;};
    void start();

  private slots:
    void slot_check_database();
    void slot_request_timeout();
    void slot_finish_cover_search();
    void slot_process_cover_search();
    void slot_system_info( INFO::InfoRequestData, QVariant );
    
  private:
    bool                   m_isRunning;
    QMap<quint64, INFO::InfoRequestData> m_requests;
    QTimer                 m_timeout;
    
    int                    m_max;
    bool                   m_exit;

    //! perform search on all missing cover or on specific request
    bool                   m_isFullDbSearch;
    
  signals:
    void finished();
    void progress(int);
};


#endif // _COVER_TASK_
