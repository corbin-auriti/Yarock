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

#include "centralwidget.h"
#include "menumodel.h"
#include "widgets/splittercollapser.h"
#include "widgets/exlineedit.h"

#include "settings.h"
#include "global_actions.h"
#include "debug.h"

/*
********************************************************************************
*                                                                              *
*    Class CentralWidget                                                       *
*                                                                              *
********************************************************************************
*/
CentralWidget::CentralWidget(
                       QWidget *parent,
                       CentralToolBar     *centralToolBar,
                       NowPlayingView     *nowPlayingView,
                       PlaylistWidget     *playlistWidget,
                       BrowserView        *browser
                       ) : QWidget(parent)
{
    m_parent             = parent;
    m_centraltoolbar     = centralToolBar;
    m_nowplayingview     = nowPlayingView;
    m_playlistwidget     = playlistWidget;
    m_browserview        = browser;

    //! Gui setup
    createGui();

    //! Show/Hide actions
    connect(ACTIONS()->value(APP_SHOW_PLAYQUEUE),   SIGNAL(triggered()), SLOT(slot_show_playlist()));
    connect(ACTIONS()->value(APP_SHOW_MENU),        SIGNAL(triggered()), SLOT(slot_show_menu()));
    connect(ACTIONS()->value(APP_SHOW_NOW_PLAYING), SIGNAL(triggered()), SLOT(slot_show_nowplaying()));

    //! Event filter for PlaylistWidget
    m_playlistwidget->installEventFilter(this);
}

/*******************************************************************************
    createGui
*******************************************************************************/
void CentralWidget::createGui()
{
    this->setObjectName(QString::fromUtf8("Main widget"));
    //this->setAutoFillBackground(false);

    QVBoxLayout* centralWidgetLayout = new QVBoxLayout(this);
    centralWidgetLayout->setSpacing(0);
    centralWidgetLayout->setContentsMargins(0, 0, 0, 0);

    //! top Frame
    QWidget *topFrame = new QFrame(this);
    topFrame->setMaximumHeight(28);

    QHBoxLayout* topFrameLayout = new QHBoxLayout(topFrame);
    topFrameLayout->setSpacing(0);
    topFrameLayout->setContentsMargins(0, 0, 0, 0);
    topFrameLayout->addWidget(m_centraltoolbar);

    //! main Frame
    QFrame *mainFrame = new QFrame(this);
    mainFrame->setFrameShape(QFrame::StyledPanel);
    mainFrame->setFrameShadow(QFrame::Sunken);
    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout* mainFrameLayout = new QHBoxLayout(mainFrame);
    mainFrameLayout->setSpacing(0);
    mainFrameLayout->setContentsMargins(0, 0, 0, 0);

    m_viewsSplitter = new CustomSplitter(this);
    m_viewsSplitter->setObjectName(QString::fromUtf8("viewsSplitter"));
    
    //! right pane
    QFrame* right_widget = new QFrame(this);
    right_widget->setFrameShape(QFrame::NoFrame);
    right_widget->setFrameShadow(QFrame::Plain);
    right_widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding );

    QVBoxLayout* vl0 = new QVBoxLayout(right_widget);
    vl0->setSpacing(0);
    vl0->setContentsMargins(0, 0, 0, 0);
    vl0->addWidget(m_nowplayingview);
    vl0->addWidget(m_playlistwidget);

    //! left pane
    QFrame* left_widget = new QFrame(this);
    left_widget->setFrameShape(QFrame::NoFrame);
    left_widget->setFrameShadow(QFrame::Plain);
    left_widget->setMinimumWidth(50);
    left_widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding );

    QVBoxLayout* vl1 = new QVBoxLayout(left_widget);
    vl1->setSpacing(0);
    vl1->setContentsMargins(0, 0, 0, 0);
   
    //!  menu widget
    m_menu_widget = new MenuWidget(this);
    m_menu_widget->setSplitter(m_viewsSplitter);
    vl1->addWidget(m_menu_widget);

    //! splitter population
    m_viewsSplitter->addWidget(left_widget);
    m_viewsSplitter->addWidget(m_browserview);
    m_viewsSplitter->addWidget(right_widget);

    QMap<ENUM_ACTION, QAction*> *actions = ACTIONS();

    //new SplitterCollapser(m_viewsSplitter, left_widget,  actions->value(APP_SHOW_MENU));
    new SplitterCollapser(m_viewsSplitter, right_widget, actions->value(APP_SHOW_PLAYQUEUE));

    mainFrameLayout->addWidget(m_viewsSplitter);

    centralWidgetLayout->addWidget(topFrame);
    centralWidgetLayout->addWidget(mainFrame);
}


/*******************************************************************************
    eventFilter
*******************************************************************************/
bool CentralWidget::eventFilter(QObject *obj, QEvent *ev)
{
    //Debug::debug() << "CentralWidget eventFilter  obj" << obj;
    int type = ev->type();
    QWidget *wid = qobject_cast<QWidget*>(obj);

    if (obj == this)
    {
        return false;
    }

    // hide conditions of the SearchPopup
    if (wid && (wid == m_playlistwidget ))
    {
      if(type == QEvent::Resize) {
        //Debug::debug() << "CentralWidget eventFilter  RESIZE EVENT";
        m_centraltoolbar->resizePlayqueueToolWidget(m_playlistwidget->width());
        return false;
      }
      else if(type == QEvent::Hide) {
          //Debug::debug() << "CentralWidget eventFilter  HIDE EVENT";
          m_centraltoolbar->showPlayqueueFilter(false);
      }
      else if(type == QEvent::Show) {
          m_centraltoolbar->showPlayqueueFilter(true);
      }
    }

    return QWidget::eventFilter(obj, ev);
}

/*******************************************************************************
    restoreState
*******************************************************************************/
/* only called at startup through MainWindow */
void CentralWidget::restoreState()
{
    slot_show_playlist();
    slot_show_menu();
    slot_show_nowplaying();

    // splitter state
    if(!SETTINGS()->_splitterState.isEmpty()) {
      m_viewsSplitter->restoreState(SETTINGS()->_splitterState);
    }
    else {
      QList<int> list;
      list << 180 << 700 << 320;
      m_viewsSplitter->setSizes (list); // 1200
    }
    
    m_menu_widget->restoreState();
}


/*******************************************************************************
    slot_show_playlist
*******************************************************************************/
void CentralWidget::slot_show_playlist( )
{
    SETTINGS()->_showPlayQueuePanel = ACTIONS()->value(APP_SHOW_PLAYQUEUE)->isChecked();

    if(SETTINGS()->_showPlayQueuePanel) {
      m_viewsSplitter->widget(2)->show();
      m_centraltoolbar->showPlayqueueFilter(true);
    }
    else {
      m_viewsSplitter->widget(2)->hide();
      m_centraltoolbar->showPlayqueueFilter(false);
    }
}


/*******************************************************************************
    slot_show_menu (Hide/Show Menu Widget)
*******************************************************************************/
void CentralWidget::slot_show_menu( )
{
    SETTINGS()->_showMenuPanel = ACTIONS()->value(APP_SHOW_MENU)->isChecked();

    if( SETTINGS()->_showMenuPanel )
      m_viewsSplitter->widget(0)->show();
    else
      m_viewsSplitter->widget(0)->hide();
}

/*******************************************************************************
    slot_show_nowplaying (Hide/Show Now playing View)
*******************************************************************************/
void CentralWidget::slot_show_nowplaying( )
{
    SETTINGS()->_showNowPlaying= ACTIONS()->value(APP_SHOW_NOW_PLAYING)->isChecked();

    if( SETTINGS()->_showNowPlaying)
      m_nowplayingview->show();
    else
      m_nowplayingview->hide();
}

