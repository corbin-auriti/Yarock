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

#include "nowplayingview.h"
#include "core/player/engine.h"
#include "core/mediaitem/mediaitem.h"
#include "covers/covercache.h"
#include "widgets/ratingwidget.h"

#include "global_actions.h"
#include "debug.h"

#include <QPainter>
#include <QApplication>
#include <QPalette>
/*
********************************************************************************
*                                                                              *
*    Class NowPlayingView                                                      *
*                                                                              *
********************************************************************************
*/
NowPlayingView::NowPlayingView(QWidget *parent) : QWidget(parent)
{
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    this->setPalette(palette);

    this->setFixedHeight (130);
    this->setFocusPolicy( Qt::NoFocus );
    this->setAutoFillBackground(true);

    m_context_menu        = 0;

    // first UI update
    update();

    //! connection
    connect(Engine::instance(), SIGNAL(mediaChanged()), this, SLOT(update()));
    connect(Engine::instance(), SIGNAL(mediaMetaDataChanged()), this, SLOT(update()));
    connect(Engine::instance(), SIGNAL(engineStateChanged()), this, SLOT(update()));
}



/*******************************************************************************
   paintEvent
*******************************************************************************/
void NowPlayingView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    const int width = this->width();

    //! Get data
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();

    //! font
    QFont font_normal = QApplication::font();
    font_normal.setBold(false);
    font_normal.setPointSize(font_normal.pointSize()*1.1);

    QFont font_bold   = QApplication::font();
    font_bold.setBold(true);
    font_bold.setPointSize(font_bold.pointSize()*1.1);


    /*-------------------------------------------------*/
    /*  PLAYING state                                  */
    /* ------------------------------------------------*/
    if(Engine::instance()->state() != ENGINE::STOPPED && track)
    {
      //Debug::debug() << "##### NowPlayingView::paintEvent";

      if(track->type() == TYPE_TRACK)
      {
          //Debug::debug() << "##### NowPlayingView::paintEvent ###### TYPE_TRACK";
          //! paint title
          painter.setPen(QApplication::palette().color(QPalette::Normal, QPalette::WindowText));
          painter.setFont(font_bold);
          QFontMetrics fm(font_bold);
          const QString titleTruncated = fm.elidedText ( track->title, Qt::ElideRight, width-135 );
          painter.drawText(QRect(130, 15, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,titleTruncated );

          //! paint album
          painter.setFont(font_normal);
          QFontMetrics fm2( font_normal);
          const QString albumTruncated = fm2.elidedText ( track->album, Qt::ElideRight, width-135 );
          painter.drawText(QRect(130, 35, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,albumTruncated );

          //! paint artist
          painter.setPen(QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
          const QString artistTruncated = fm2.elidedText ( track->artist, Qt::ElideRight, width-135 );
          painter.drawText(QRect(130, 55, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,artistTruncated );

          //! paint duration
          if(track->duration > 0) {
            const QString durationText = track->durationToString();
            const QString dureeTruncated = fm2.elidedText ( durationText, Qt::ElideRight, width-135 );
            painter.drawText(QRect(130, 75, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,dureeTruncated );
          }

          //! paint track rating
          RatingPainter::instance()->Paint(&painter, QRect(130, 95, 75, 22),track->rating, true);

          //! paint pixmap (coverart)
          QPixmap pix = CoverCache::instance()->cover(track);
          painter.drawPixmap(10,10,110,110, pix);
      }
      else if(track->type() == TYPE_STREAM)
      {
          //Debug::debug() << "##### NowPlayingView::paintEvent ###### TYPE_STREAM";

          //! paint stream name
          painter.setPen(QApplication::palette().color(QPalette::Normal, QPalette::WindowText));
          painter.setFont(font_bold);
          QFontMetrics fm(font_bold);
          const QString streamTruncated = fm.elidedText ( track->name, Qt::ElideRight, width-135 );
          painter.drawText(QRect(130, 15, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,streamTruncated );


          //! paint title
          painter.setPen(QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
          painter.setFont(font_normal);
          QFontMetrics fm2(font_normal);
          const QString title_or_url = track->title.isEmpty() ? track->url : track->title;

          const QString titleTruncated = fm2.elidedText ( title_or_url, Qt::ElideRight, width-135 );
          painter.drawText(QRect(130, 35, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,titleTruncated );

          //! paint album
          const QString albumTruncated = fm2.elidedText ( track->album, Qt::ElideRight, width-135 );
          painter.drawText(QRect(130, 55, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,albumTruncated );

          //! paint artist
          const QString artistTruncated = fm2.elidedText ( track->artist, Qt::ElideRight, width-135 );
          painter.drawText(QRect(130, 75, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,artistTruncated );

          //! paint pixmap (cover art)
          QPixmap pix = CoverCache::instance()->cover(track);
          painter.drawPixmap(8,8,110,110, pix);
      }
    }
    /*-------------------------------------------------*/
    /*  STOPPED state                                  */
    /* ------------------------------------------------*/
    //else if(EnginePlayer::instance()->engineState() == ENGINE::STOPPED)
    else
    {
        //Debug::debug() << "##### NowPlayingView::paintEvent ###### ENGINE STOPPED";

        // paint icon
        QFont font   = QApplication::font();
        font.setBold(true);
        font.setPointSize(font.pointSize()*1.4);

        painter.setFont(font);
        painter.setPen(QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
        painter.drawText(QRect(-10, 0, width-10, 50), Qt::AlignCenter,QString("Yarock") );

        int Xpos = width-64 - 20 > 0 ? (width-64)/2 -10: 10;

        painter.drawPixmap(Xpos,40,64,64, QPixmap(":/icon/64x64/yarock.png"));
    }
    painter.end();
}


/*******************************************************************************
   contextMenuEvent
*******************************************************************************/
void NowPlayingView::contextMenuEvent(QContextMenuEvent * event)
{
    if(!m_context_menu)
    {
      m_context_menu = new QMenu(this);
      m_context_menu->addAction(ACTIONS()->value(APP_SHOW_NOW_PLAYING));
    }
    // show the menu
    m_context_menu->popup(event->globalPos());
}
