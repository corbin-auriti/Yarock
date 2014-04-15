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

#include "minimalwidget.h"

#include "core/player/engine.h"
#include "core/mediaitem/mediaitem.h"
#include "covers/covercache.h"
#include "widgets/spacer.h"
#include "global_actions.h"


#include <QtGui>
#include <QPainter>



/*
********************************************************************************
*                                                                              *
*    NowPlaying                                                                *
*                                                                              *
********************************************************************************
*/
class NowPlaying : public QWidget
{
  public:
    NowPlaying(QWidget *parent) : QWidget(parent)
    {
       this->setFixedHeight (180);
       this->setFocusPolicy( Qt::NoFocus );
       this->setAutoFillBackground(false);
       this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

       connect(Engine::instance(), SIGNAL(mediaChanged()), this, SLOT(update()));
       connect(Engine::instance(), SIGNAL(mediaMetaDataChanged()), this, SLOT(update()));
       connect(Engine::instance(), SIGNAL(engineStateChanged()), this, SLOT(update()));
    }

  protected:
    void paintEvent(QPaintEvent *event);

};



//! -------- NowPlaying::paintEvent --------------------------------------------
void NowPlaying::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  QRectF rect(10, 0, 180, 20);

  painter.setPen(QApplication::palette().color(QPalette::Normal, QPalette::WindowText));

  //! Get data
  MEDIA::TrackPtr track  =  Engine::instance()->playingTrack();

  //! -- Playing State
  if(Engine::instance()->state() != ENGINE::STOPPED && !Engine::instance()->playingTrack())
  {
    if(track->type() == TYPE_TRACK)
    {
        //! paint artist
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        QFontMetrics fm( QFont("Arial", 10, QFont::Bold));
        QString artistTruncated = fm.elidedText ( track->artist, Qt::ElideRight, 180 );
        painter.drawText(rect, Qt::AlignVCenter | Qt::AlignLeft,artistTruncated );

        //! paint album
        painter.setFont(QFont("Arial", 10, QFont::Normal));
        QFontMetrics fm2( QFont("Arial", 10, QFont::Normal));
        QString albumTruncated = fm2.elidedText ( track->album, Qt::ElideRight, 180 );
        painter.drawText(rect.adjusted(0,15,0,15), Qt::AlignVCenter | Qt::AlignLeft,albumTruncated );

        //! paint title
        QString titleTruncated = fm2.elidedText ( track->title, Qt::ElideRight, 180 );
        painter.drawText(rect.adjusted(0,145,0,145), Qt::AlignVCenter | Qt::AlignLeft,titleTruncated );

        //! paint pixmap (coverart)
        QPixmap pix =  CoverCache::instance()->cover(track);
        painter.drawPixmap(45,35,110,110, pix);
    }
    else if(track->type() == TYPE_STREAM)
    {
        if(track->album.isEmpty()) { //! paint short stream name
            painter.setFont(QFont("Arial", 10, QFont::Bold));
            QFontMetrics fm( QFont("Arial", 10, QFont::Bold));
            QString streamTruncated = fm.elidedText ( track->name, Qt::ElideRight, 180 );
            painter.drawText(rect, Qt::AlignVCenter | Qt::AlignLeft,streamTruncated );
        }
        else { //! paint album
            painter.setFont(QFont("Arial", 10, QFont::Bold));
            QFontMetrics fm( QFont("Arial", 10, QFont::Bold));
            QString albumTruncated = fm.elidedText ( track->album, Qt::ElideRight, 180 );
            painter.drawText(rect, Qt::AlignVCenter | Qt::AlignLeft,albumTruncated );
       }

       //! paint artist - title
       painter.setFont(QFont("Arial", 10, QFont::Normal));
       QFontMetrics fm2( QFont("Arial", 10, QFont::Normal));
       QString titleTruncated = fm2.elidedText ( track->title, Qt::ElideRight, 180 );
       painter.drawText(rect.adjusted(0,15,0,15), Qt::AlignVCenter | Qt::AlignLeft,titleTruncated );

       //! paint pixmap (coverart)
       QPixmap pix =  CoverCache::instance()->cover(track);
       painter.drawPixmap(45,40,110,110, pix);
    }
  }
  //! -- Stopped State
  else
  {
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    QFontMetrics fm( QFont("Arial", 12, QFont::Bold));
    QString text = fm.elidedText ( "Yarock Player", Qt::ElideRight, 180);
    painter.drawText(rect.adjusted(0,30,0,50), Qt::AlignHCenter,text );

    painter.drawPixmap(65,50,64,64, QPixmap(":/icon/64x64/yarock.png"));
   }
   painter.end();
}
/*
********************************************************************************
*                                                                              *
*    MinimalWidget                                                             *
*                                                                              *
********************************************************************************
*/
MinimalWidget::MinimalWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setWindowFlags(Qt::Window);

    QMap<ENUM_ACTION, QAction*> *actions  = ACTIONS();

    //! create toolbar
    QToolButton* _prevButton = new QToolButton(this);
    _prevButton->setDefaultAction(actions->value(ENGINE_PLAY_PREV));
    _prevButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _prevButton->setAutoRaise(true);
    _prevButton->setIconSize( QSize( 28, 28 ) );

    QToolButton* _nextButton = new QToolButton(this);
    _nextButton->setDefaultAction(actions->value(ENGINE_PLAY_NEXT));
    _nextButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _nextButton->setAutoRaise(true);
    _nextButton->setIconSize( QSize( 28, 28 ) );

    QToolButton* _playButton = new QToolButton(this);
    _playButton->setDefaultAction(actions->value(ENGINE_PLAY));
    _playButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _playButton->setAutoRaise(true);
    _playButton->setIconSize( QSize( 28, 28 ) );

    QToolButton* _stopButton = new QToolButton(this);
    _stopButton->setDefaultAction(actions->value(ENGINE_STOP));
    _stopButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _stopButton->setAutoRaise(true);
    _stopButton->setIconSize( QSize( 28, 28 ) );

    QToolButton* _normalModeButton = new QToolButton(this);
    _normalModeButton->setDefaultAction(actions->value(APP_MODE_NORMAL));
    _normalModeButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _normalModeButton->setAutoRaise(true);
    _normalModeButton->setIconSize( QSize( 18, 18 ) );

    QHBoxLayout* hl0 = new QHBoxLayout();
    hl0->setSpacing(0);
    hl0->setContentsMargins(0, 0, 0, 0);
    hl0->addWidget( new FixedSpacer(this, QSize(15,0)) );
    hl0->addWidget( _prevButton);
    hl0->addWidget( _playButton);
    hl0->addWidget( _stopButton);
    hl0->addWidget( _nextButton);
    hl0->addWidget( new FixedSpacer(this, QSize(5,0)) );
    hl0->addWidget( _normalModeButton );
    hl0->addWidget( new FixedSpacer(this, QSize(15,0)) );

    //! create Gui
    this->setObjectName(QString::fromUtf8("MinimalWidget"));
    this->setWindowTitle("yarock");
    this->setContentsMargins(0, 0, 0, 0);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setStyleSheet("background:transparent;");

    NowPlaying *nowPlaying = new NowPlaying(this);

    QVBoxLayout* vl0 = new QVBoxLayout(this);
    vl0->setSpacing(0);
    vl0->setContentsMargins(0, 0, 0, 0);
    vl0->addWidget(nowPlaying);
    vl0->addLayout(hl0);

    this->setMinimumSize(QSize(200,200));
    this->adjustSize();
    this->setFixedSize( sizeHint() );
    this->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}



//! --------- paintEvent -------------------------------------------------------
void MinimalWidget::paintEvent(QPaintEvent *)
{
    //! draw background
    QPainter painter(this);
    //QColor backColor = QColor(23, 23, 34);
    QColor back_color = QApplication::palette().color(QPalette::Normal, QPalette::Background);
    back_color.setAlpha(80);

    painter.setBrush(QBrush(back_color));
    painter.fillRect(rect(), back_color);
}
