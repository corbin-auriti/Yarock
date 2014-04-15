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

// local
#include "playertoolbar.h"
#include "widgets/audiocontrols.h"
#include "widgets/spacer.h"
#include "widgets/seekslider.h"

#include "core/player/engine.h"
#include "utilities.h"
#include "global_actions.h"
#include "debug.h"

// Qt
#include <QTime>
#include <QSpacerItem>
#include <QSettings>
#include <QToolButton>
#include <QWidget>
#include <QWidgetAction>

/*
********************************************************************************
*                                                                              *
*    Class VolumeToolButton                                                    *
*                                                                              *
********************************************************************************
*/
VolumeToolButton::VolumeToolButton(QWidget *parent) : QToolButton( parent )
{
    this->setIconSize( QSize( 24, 24 ) );
    this->setAutoRaise(true);
    this->setIcon(QIcon(":/images/volume-icon.png"));
    this->setPopupMode (QToolButton::InstantPopup);
    setContextMenuPolicy( Qt::CustomContextMenu );

    //! Volume Label
    m_volume_label= new QLabel( this );
    m_volume_label->setAlignment( Qt::AlignHCenter );

    //! volume slider
    m_slider = new QSlider();
    m_slider->setOrientation(Qt::Vertical);
    m_slider->setMaximum(100);
    m_slider->setRange(0, 100);
    m_slider->setPageStep(5);
    m_slider->setSingleStep(1);

    //! tool bar mute action
    QToolBar *volumeToolBar = new QToolBar(this);
    volumeToolBar->addAction(ACTIONS()->value(ENGINE_VOL_MUTE));

    QVBoxLayout * mainBox = new QVBoxLayout(this);
    mainBox->setSpacing(0);
    mainBox->setContentsMargins(4,4,4,4);
    mainBox->addWidget(m_volume_label);
    mainBox->addWidget(m_slider);
    mainBox->addWidget(volumeToolBar);

    QWidget* main_widget = new QWidget(this);
    main_widget->setLayout(mainBox);

    QWidgetAction * sliderActionWidget = new QWidgetAction( this );
    sliderActionWidget->setDefaultWidget( main_widget );

     //! Popup volume menu
    QMenu* volumeMenu = new QMenu(this);
    volumeMenu->addAction( sliderActionWidget );
    this->setMenu(volumeMenu);

    connect(ACTIONS()->value(ENGINE_VOL_MUTE), SIGNAL(triggered()), this, SLOT(slot_mute_toggle_action()));
    connect(Engine::instance() , SIGNAL(muteStateChanged ()), this, SLOT(slot_mute_change()));
    connect(Engine::instance() , SIGNAL(volumeChanged ()), this, SLOT(slot_volume_change()));
    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(slot_apply_volume(int)));

    /* init ui */
    Engine::instance()->setMuted(false);

    slot_mute_change();
    slot_volume_change();
    m_slider->setValue(Engine::instance()->volume());
}


void VolumeToolButton::slot_volume_change()
{
    //Debug::debug() << "- VolumeToolButton -> slot_volume_change percent: ";
    int volume = Engine::instance()->volume();
    m_volume_label->setText( QString::number( volume ) + '%' );
}

void VolumeToolButton::slot_apply_volume(int vol)
{
    Debug::debug() << "- VolumeToolButton -> slot_apply_volume : " << vol;

    Engine::instance()->setVolume(vol);
}

void VolumeToolButton::slot_mute_change()
{
    //Debug::debug() << "- VolumeToolButton -> volumeMuteChange()";

    if (!Engine::instance()->isMuted())
    {
      this->setIcon(QIcon(":/images/volume-icon.png"));
      (ACTIONS()->value(ENGINE_VOL_MUTE))->setIcon(QIcon(":/images/volume-icon.png"));
    }
    else
    {
      this->setIcon(QIcon(":/images/volume-muted.png"));
      (ACTIONS()->value(ENGINE_VOL_MUTE))->setIcon(QIcon(":/images/volume-muted.png"));
    }
}


void VolumeToolButton::slot_mute_toggle_action()
{
    bool isMuted = Engine::instance()->isMuted();
    Engine::instance()->setMuted(!isMuted);
}


/*
********************************************************************************
*                                                                              *
*    Class PlayerToolBar                                                       *
*                                                                              *
********************************************************************************
*/
PlayerToolBar::PlayerToolBar(QWidget *parent) : QToolBar( "PlayerToolBar", parent )
{
    m_player = Engine::instance();

    this->setObjectName(QString::fromUtf8("playerToolBar"));
    this->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    this->setIconSize( QSize( 32, 32 ) );
    this->setFloatable(false);
    this->setMovable(false);
    this->setContentsMargins( 0, 0, 0, 0);
    this->layout()->setSpacing( 0 );
    this->setAllowedAreas( Qt::TopToolBarArea );
    
    //! track time position
    QFont font;
    font.setPointSize(font.pointSize()*.90);
    
    m_currentTime = new QLabel(this);
    m_currentTime->setFont(font);
    m_currentTime->setAlignment( Qt::AlignLeft );

    m_totalTime = new QLabel(this);
    m_totalTime->setFont(font);
    m_totalTime->setAlignment( Qt::AlignRight );

    m_currentTime->setText("0:00:00");
    int width = m_currentTime->sizeHint().width();
    m_currentTime->setMinimumWidth(width);
    m_currentTime->clear();  
    m_totalTime->setMinimumWidth(width);
  
    //! phonon slider
    QHBoxLayout *hbl1 = new QHBoxLayout();
    hbl1->setContentsMargins( 0, 0, 0, 0);
    hbl1->setSpacing(1);
    hbl1->setMargin(0);

    QHBoxLayout *hbl2 = new QHBoxLayout();
    hbl2->setContentsMargins( 0, 0, 0, 0);
    hbl2->setSpacing( 0 );
    hbl2->setMargin(0);

    m_playingTrack = new QLabel(this);
    m_playingTrack->setFont(font);

    SeekSlider* seekSlider = new SeekSlider(this);

    hbl1->addWidget(m_currentTime);
    hbl1->addWidget(seekSlider);
    hbl1->addWidget(m_totalTime);

    hbl2->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hbl2->addWidget(m_playingTrack);
    hbl2->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    QWidget *w = new QWidget();
    QVBoxLayout *vbl = new QVBoxLayout(w);
    vbl->setSpacing( 0 );
    vbl->setContentsMargins(0, 0, 0, 0);
    vbl->setMargin(0);
    //vbl->addItem(new QSpacerItem(20, 4, QSizePolicy::Minimum, QSizePolicy::Expanding));
    vbl->addLayout(hbl2, Qt::AlignTop);
    vbl->addLayout(hbl1, Qt::AlignTop);
    vbl->addItem(new QSpacerItem(20, 4, QSizePolicy::Minimum, QSizePolicy::Expanding));

    vbl->addSpacerItem(new QSpacerItem(20, 4, QSizePolicy::Minimum, QSizePolicy::Expanding));

    //! ToolButton creation
    QToolButton* ui_prev_button = new QToolButton(this);
    ui_prev_button->setDefaultAction(ACTIONS()->value(ENGINE_PLAY_PREV));

    QToolButton* ui_next_button = new QToolButton(this);
    ui_next_button->setDefaultAction(ACTIONS()->value(ENGINE_PLAY_NEXT));

    QToolButton* ui_play_button = new QToolButton(this);
    ui_play_button->setDefaultAction(ACTIONS()->value(ENGINE_PLAY));

    QToolButton* ui_stop_button = new QToolButton(this);
    ui_stop_button->setDefaultAction(ACTIONS()->value(ENGINE_STOP));

    //! volume tool button and menu
    VolumeToolButton* ui_volume_button = new VolumeToolButton(this);

    //! Audio controls (REPEAT & SHUFFLE)
    // la taille est fixée dans les QtoolButton par setFixedSize
    RepeatControl*   repeat_control  = new RepeatControl(this);
    ShuffleControl*  shuffle_control = new ShuffleControl(this);

    //! PlayerToolBar setup
    this->addWidget( new FixedSpacer(this, QSize(5,0)) );
    this->addWidget( ui_prev_button );
    this->addWidget( ui_play_button );
    this->addWidget( ui_stop_button );
    this->addWidget( ui_next_button);
    this->addWidget( new FixedSpacer(this, QSize(40,0)) );
    this->addWidget(w);
    this->addWidget( new FixedSpacer(this, QSize(40,0)) );
    this->addWidget( repeat_control );
    this->addWidget( shuffle_control );
    this->addWidget( ui_volume_button );
    this->addWidget( new FixedSpacer(this, QSize(5,0)) );

    //! signals
    connect(this->m_player, SIGNAL(mediaTick(qint64)), this, SLOT(slot_update_time_position(qint64)));
    connect(this->m_player, SIGNAL(mediaTotalTimeChanged(qint64)), this, SLOT(slot_update_total_time(qint64)));

    connect(this->m_player, SIGNAL(mediaMetaDataChanged()), this, SLOT(slot_update_track_playing_info()));
    connect(this->m_player, SIGNAL(mediaChanged()), this, SLOT(slot_update_track_playing_info()));
    connect(this->m_player, SIGNAL(engineStateChanged()), this, SLOT(slot_update_track_playing_info()));
}


void PlayerToolBar::clear()
{
    //Debug::debug() << "    [PlayerToolBar] clear";
    m_playingTrack->clear();
    m_currentTime->clear();
    m_totalTime->clear();
}


void PlayerToolBar::slot_update_track_playing_info()
{
   //Debug::debug() << "    [PlayerToolBar] slot_update_track_playing_info";

   if(m_player->state() == ENGINE::STOPPED && !m_player->playingTrack())
   {
       this->clear();
       return;
   }
   else if(m_player->playingTrack())
   {
     slot_update_total_time( m_player->currentTotalTime() );

     MEDIA::TrackPtr track = m_player->playingTrack();
     if(track->type() == TYPE_TRACK)
     {
       m_playingTrack->setText(
         QString(tr("<b>%1</b> by <b>%2</b> on <b>%3</b>"))
                    .arg(track->title,track->artist,track->album));

     }
     else
     {
       if(!track->title.isEmpty() && !track->album.isEmpty() && !track->artist.isEmpty())
       {
          //! after meta data update
          m_playingTrack->setText(  QString(tr("<b>%1</b> by <b>%2</b> on <b>%3</b>"))
                                    .arg(track->title,track->artist,track->album)
                                 );
       }
       else
       {
         //! before meta data update
         m_playingTrack->setText( QString(tr("<b>%1</b> stream")).arg(track->name ));
       }
    }
   }
}


void PlayerToolBar::slot_update_time_position(qint64 newPos)
{
    //Debug::debug() << "    [PlayerToolBar] slot_update_time_position " << newPos;

    if(m_player->state() == ENGINE::STOPPED && !m_player->playingTrack()) {
       m_currentTime->clear();
       return;
    }

    if (newPos <= 0) {
        m_currentTime->clear();
        return;
    }

    const QTime displayTime = displayTime.addMSecs(newPos);
    QString timeString;

    if (newPos > 3600000)
        timeString = displayTime.toString("h:mm:ss");
    else
        timeString = displayTime.toString("m:ss");

    m_currentTime->setText(timeString);
}


void PlayerToolBar::slot_update_total_time(qint64 newTotalTime)
{
    //Debug::debug() << "    [PlayerToolBar] slot_update_total_time";

    if(m_player->state() == ENGINE::STOPPED && !m_player->playingTrack()) {
       this->clear();
       return;
    }

    if (newTotalTime <= 0) {
        m_totalTime->clear();
        return;
    }

    const QTime displayTime = displayTime.addMSecs(newTotalTime);
    QString timeString;

    if (newTotalTime > 3600000)
        timeString = displayTime.toString("h:mm:ss");
    else
        timeString = displayTime.toString("m:ss");

    m_totalTime->setText(timeString);
}
