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
//! this part is based on GwenView (image viewer) source code


#ifndef _SPLITTERCOLLAPSER_H_
#define _SPLITTERCOLLAPSER_H_

#include <QToolButton>

class QSplitter;

struct SplitterCollapserPrivate;


/*
********************************************************************************
*                                                                              *
*    Class SplitterCollapser                                                   *
*                                                                              *
********************************************************************************
*/
/**
 * A button which appears on the side of a splitter handle and allows easy
 * collapsing of the widget on the opposite side
 */
class SplitterCollapser : public QToolButton
{
Q_OBJECT
  public:
    SplitterCollapser(QSplitter*, QWidget* widget, QAction *action);
    ~SplitterCollapser();

    virtual QSize sizeHint() const;

  protected:
    virtual bool eventFilter(QObject*, QEvent*);
    virtual bool event(QEvent* event);
    virtual void paintEvent(QPaintEvent*);
    virtual void showEvent(QShowEvent*);

  private:
    SplitterCollapserPrivate* const d;

  private Q_SLOTS:
    void slotClicked();
};


#endif // _SPLITTERCOLLAPSER_H_
