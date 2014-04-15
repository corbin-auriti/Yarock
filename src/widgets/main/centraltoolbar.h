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

#ifndef _CENTRAL_TOOLBAR_H_
#define _CENTRAL_TOOLBAR_H_

#include "views.h"

#include <QToolBar>
#include <QToolButton>
#include <QModelIndex>
#include <QList>
#include <QString>

class SearchLineEdit;
class ExLineEdit;
class SearchPopup;

/*
********************************************************************************
*                                                                              *
*    Class CentralToolBar                                                      *
*                                                                              *
********************************************************************************
*/
class CentralToolBar : public QToolBar
{
Q_OBJECT
    static CentralToolBar* INSTANCE;
  public:
    CentralToolBar(QWidget *parent);
    static CentralToolBar* instance() { return INSTANCE; }

    //! playqueue tool bar widgets
    void showPlayqueueFilter(bool show);
    void resizePlayqueueToolWidget(int size);

    //! explorer filter
    QString explorerFilter();
    void setExplorerFilterText(const QString&);

    //! go up button
    void showHideGoUp(VIEW::Id m);
    
  private slots:
    void slot_send_explorer_filter();
    void slot_send_playqueue_filter();
    void slot_explorer_popup_setting_change();
    
  signals:
    void explorerFilterActivated(const QString&);
    void playqueueFilterActivated(const QString&);
    void dbNameChanged();

  private:
    ExLineEdit           *ui_explorer_filter;
    SearchPopup          *ui_popup_completer;
    
    ExLineEdit           *ui_playqueue_filter;
    QAction              *m_filterWidgetAction;
};

#endif // _CENTRAL_TOOLBAR_H_
