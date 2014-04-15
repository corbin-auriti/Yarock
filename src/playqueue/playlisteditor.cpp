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

#include "playlisteditor.h"
#include "playqueue_model.h"
#include "task_manager.h"
#include "playlistview.h"

#include "threadmanager.h"
#include "widgets/statuswidget.h"
#include "widgets/dialogs/addstreamdialog.h"
#include "widgets/dialogs/filedialog.h"
#include "debug.h"

#include <QtGui>
#include <QtCore>

/*
********************************************************************************
*                                                                              *
*    Class PlaylistEditor                                                      *
*                                                                              *
********************************************************************************
*/
PlaylistEditor::PlaylistEditor(QWidget *parent) : QDialog(parent)
{
    //! init
    m_model         = new PlayqueueModel();
    m_task_manager  = m_model->manager();

    m_view  = new PlaylistView(this, m_model);
    m_view->setMode((int) PLAYQUEUE::MODE_ARTIST);

    // GUI
    create_ui();

    QObject::connect(ui_line_edit,SIGNAL(textfield_changed()),this,SLOT(slot_line_edit_change()));
    QObject::connect(m_task_manager, SIGNAL(playlistSaved()), ThreadManager::instance(), SLOT(populateLocalPlaylistModel()));
}

PlaylistEditor::~PlaylistEditor()
{
    delete m_model;
}


void PlaylistEditor::closeEvent ( QCloseEvent * event )
{
    QDialog::closeEvent(event);
    emit closed();
}

void PlaylistEditor::hideEvent ( QHideEvent * )
{
    //! force to quit if "esc" is pressed instead of hiding dialog window
    this->setResult(QDialog::Rejected);
    QDialog::reject();
    this->close();
}
/*******************************************************************************
 ui part
*******************************************************************************/
void PlaylistEditor::create_ui()
{

    //! ui part
    this->setModal(false);
    this->setWindowTitle(tr("Playlist Editor"));
    this->resize(500,620);

    //In Linux with KDE this code make a window without an close and minimize
    // and maximize buttons in title bar.
    this->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );

    // set toolbar & action
    QToolBar* toolbar = new QToolBar(this);
    toolbar->addAction( QIcon(":/images/track-48x48.png"), tr("&Add media to playlist"), this, SLOT(slot_add_file()));
    toolbar->addAction( QIcon(":/images/folder.png"), tr("&Add directory to playlist"), this, SLOT(slot_add_dir()));
    toolbar->addAction( QIcon(":/images/media-url-48x48.png"),tr("&Add Url..."), this, SLOT(slot_add_url()));
    toolbar->addAction( QIcon::fromTheme("edit-clear-list"), tr("Clear playlist"), this, SLOT(slot_clear_playlist()));

    ui_action_save_file = new QAction (QIcon(":/images/save-as-32x32.png"), tr("&Save playlist to file"), this);
    ui_action_save_auto = new QAction (QIcon(":/images/save-32x32.png"), tr("&Auto save playlist to yarock database"), this);
    ui_action_save_file->setEnabled(false);
    ui_action_save_auto->setEnabled(false);
    connect(ui_action_save_file, SIGNAL(triggered()), this, SLOT(slot_save_to_file()));
    connect(ui_action_save_auto, SIGNAL(triggered()), this, SLOT(slot_save_to_db()));

    toolbar->addAction( ui_action_save_file );
    toolbar->addAction( ui_action_save_auto );

    // set layout
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(4, 0, 0, 0);
    gridLayout->setSpacing(4);

    QLabel* label = new QLabel(tr("name"));

    ui_line_edit = new ExLineEdit(this);
    ui_line_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui_line_edit->clearFocus();
    ui_line_edit->setInactiveText(tr("enter a playlist name"));

    gridLayout->addWidget(label, 0, 0, 1, 1);
    gridLayout->addWidget(ui_line_edit, 0, 1, 1, 1);

    QFrame* frame = new QFrame();
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Sunken);

    QVBoxLayout *vl0 = new QVBoxLayout(frame);
    vl0->setSpacing(0);
    vl0->setContentsMargins(0, 0, 0, 0);
    vl0->addWidget(m_view);

    QVBoxLayout *vl1 = new QVBoxLayout(this);
    vl1->setSpacing(0);
    vl1->setContentsMargins(0, 0, 0, 0);
    vl1->addWidget(toolbar);
    vl1->addLayout(gridLayout);
    vl1->addWidget(frame);

    //! QDialogButtonBox
    QDialogButtonBox* ui_buttonBox = new QDialogButtonBox();
    ui_buttonBox->setOrientation(Qt::Horizontal);
    ui_buttonBox->setStandardButtons(QDialogButtonBox::Close);
    vl1->addWidget(ui_buttonBox);
    QObject::connect(ui_buttonBox, SIGNAL(rejected()), this, SLOT(slot_on_buttonBox_rejected()));

    // avoid focus to be on line edit
    this->setFocus(Qt::OtherFocusReason);
}

void PlaylistEditor::slot_on_buttonBox_rejected()
{
    this->setResult(QDialog::Rejected);
    QDialog::reject();
    this->close();
}

void PlaylistEditor::slot_line_edit_change()
{
    ui_action_save_file->setEnabled( !ui_line_edit->text().isEmpty() );
    ui_action_save_auto->setEnabled( !ui_line_edit->text().isEmpty() );
}


/*******************************************************************************
 public interface
*******************************************************************************/
void PlaylistEditor::setMediaList(QList<MEDIA::TrackPtr> list)
{
    foreach(MEDIA::TrackPtr track, list)   {
      m_model->addMediaItem(track);
    }
}


void PlaylistEditor::setPlaylist(MEDIA::PlaylistPtr playlist)
{
    m_playlist = playlist;
    ui_line_edit->setText( playlist->name );

    m_model->clear();
    foreach(MEDIA::MediaPtr media, playlist->children())   {
      m_model->addMediaItem( MEDIA::TrackPtr::staticCast(media) );
    }
}


/*******************************************************************************
 Slots
*******************************************************************************/
void PlaylistEditor::slot_clear_playlist()
{
    m_model->clear();
}

/*******************************************************************************
 Playlist Population
*******************************************************************************/
void PlaylistEditor::slot_add_file()
{
    FileDialog fd(this, FileDialog::AddFiles, tr("Add music files or playlist"));

    if(fd.exec() == QDialog::Accepted) {
       QStringList files  = fd.addFiles();
       
       m_task_manager->playlistAddFiles(files);
    }      
}


void PlaylistEditor::slot_add_dir()
{
    /*--------------------------------------------------*/
    /* add dir to playqueue                             */
    /* -------------------------------------------------*/
    FileDialog fd(this, FileDialog::AddDirs, tr("Add music directories"));

    if(fd.exec() == QDialog::Accepted) {
      QStringList dirs  = fd.addDirectories();
      m_task_manager->playlistAddFiles(dirs);
    }    
}


void PlaylistEditor::slot_add_url()
{
    AddStreamDialog stream_dialog(this,false);

    if(stream_dialog.exec() == QDialog::Accepted)
    {
      const QString url   = stream_dialog.url();

      if(!QUrl(url).isEmpty() && QUrl(url).isValid()) {
        const QString name  = stream_dialog.name();

        MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());
        media->setType(TYPE_STREAM);
        media->id          = -1;
        media->url         = url;
        media->name        = !name.isEmpty() ? name : url ;
        media->isFavorite  = false;
        media->isPlaying   = false;
        media->isBroken    = false;
        media->isPlayed    = false;
        media->isStopAfter = false;

        m_model->addMediaItem(media);
        media.reset();
      }
      else {
        StatusWidget::instance()->startShortMessage("invalid url can not be added !!", STATUS::TYPE_WARNING, 5000);
      }
    }
}


/*******************************************************************************
 Playlist Writer
*******************************************************************************/
void PlaylistEditor::slot_save_to_file()
{
    Debug::debug() << "PlaylistEditor -> slot_save_to_file";

    //! new playlist
    if(m_playlist.isNull())
    {
      
      FileDialog fd(this, FileDialog::SaveFile, tr("Save playlist to file"));

      if(fd.exec() == QDialog::Accepted) {
        QString file  = fd.saveFile();
        if (!file.isEmpty())
          m_task_manager->playlistSaveToFile(file);
      }  
    }
    //! playlist existante
    else
    {
      m_task_manager->playlistSaveToFile( m_playlist->url );
    }
}




void PlaylistEditor::slot_save_to_db()
{
    Debug::debug() << "PlaylistEditor -> slot_save_to_db";
    const QString name = ui_line_edit->text();

    if(name.isEmpty())
    {
        DialogMessage dlg(this,tr("Playlist editor"));
        dlg.setMessage(tr("<p>Please, fill the playlist name first</p>"));
        dlg.resize(445, 120);
        dlg.exec();  
        return;
    }

    //! new playlist
    if(m_playlist.isNull())
    {
       m_task_manager->playlistSaveToDb( name );
    }
    //! playlist existante
    else
    {
       int database_id = m_playlist->id;
       m_task_manager->playlistSaveToDb( name , database_id);
    }

}
