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

#include "mediaitem_edit_dialog.h"
#include "core/database/database.h"
#include "core/mediaitem/mediaitem.h"
#include "utilities.h"
#include "debug.h"

#include "models/local/local_track_model.h"

#include <QSqlField>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QVariant>
#include <QString>
#include <QtGui>

//! -------------- recupCoverArt -----------------------------------------------
//! copy old cover if exist into new cover art (according to new
//! artist or album name)
void recupCoverArt(const QString &newCoverName, const QString &oldCoverName);


void recupCoverArt(const QString &newCoverName, const QString &oldCoverName)
{
    //Debug::debug() << " ----recupCoverArt newCoverName + oldCoverName---- " << newCoverName + " - " + oldCoverName;
    QString storageLocation = UTIL::CONFIGDIR + "/albums/";

    //! quit if same filename
    if(newCoverName == oldCoverName) return;

    //! copy old into new
    QFile oldCoverFile(storageLocation + oldCoverName);
    if(oldCoverFile.exists()) 
    {
      //! remove new cover if already exist
      QFile newCoverFile(storageLocation + newCoverName);
      if(newCoverFile.exists()) 
        newCoverFile.remove();

      //Debug::debug() << " ----recupCoverArt oldCoverName exists COPY ---- ";
      oldCoverFile.rename(storageLocation + newCoverName);
    }
}


/*
********************************************************************************
*                                                                              *
*    Class MediaItem_Edit_Dialog                                               *
*                                                                              *
********************************************************************************
*/
MediaItem_Edit_Dialog::MediaItem_Edit_Dialog(T_DIALOG_TYPE type) : DialogBase()
{
    Debug::debug() << "MediaItem_Edit_Dialog";

    dialog_type_  = type;
    isChanged_    = false;
    media_        = MEDIA::MediaPtr( 0 );

    //! create gui
    create_gui();

    //! Connection
    QObject::connect(buttonBox(), SIGNAL(accepted()), this, SLOT(on_buttonBox_accepted()));
    QObject::connect(buttonBox(), SIGNAL(rejected()), this, SLOT(on_buttonBox_rejected()));
    QObject::connect(ui_clear_playcount, SIGNAL(clicked()), this, SLOT(slot_clear_playcount()));

    if(dialog_type_ != DIALOG_TRACK)
    {
      QObject::connect(ui_clear_rating, SIGNAL(clicked()), this, SLOT(slot_clear_rating()));
    }
}

MediaItem_Edit_Dialog::~MediaItem_Edit_Dialog()
{
    media_.reset();
}

void MediaItem_Edit_Dialog::setMediaItem(MEDIA::MediaPtr media)
{
    media_     = MEDIA::MediaPtr( media );
    //Debug::debug() << "MediaItem_Edit_Dialog setMediaItem : " << media_;

    //! restore values from MediaItem
    if(dialog_type_ == DIALOG_ARTIST)
    {
      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(media);
      ui_artist_name->setText( artist->name );
      ui_playcount->setValue( artist->playcount );

      set_rating(artist->isUserRating);
    }
    else if(dialog_type_ == DIALOG_ALBUM)
    {
      MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(media);
      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(media->parent());
      ui_album_name->setText( album->name );
      ui_artist_name->setText( artist->name );
      ui_album_year->setDate( QDate(album->year, 1, 1));
      ui_playcount->setValue( album->playcount );

      set_rating(album->isUserRating);
    }
    else if(dialog_type_ == DIALOG_GENRE)
    {
      MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(media);
      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(media->parent());

      ui_album_name->setText( album->name );
      ui_artist_name->setText( artist->name );
      ui_album_year->setDate( QDate(album->year, 1, 1));
      ui_album_genre->setText( genre_ );
      ui_playcount->setValue( album->playcount );

      set_rating( album->isUserRating );
    }
    else if(dialog_type_ == DIALOG_TRACK)
    {
      MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(media->parent());
      MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(media);

      ui_filename->setText( track->url );
      ui_track_title->setText( track->title );
      ui_album_name->setText( track->album );
      ui_artist_name->setText( track->artist );
      ui_track_year->setDate( QDate(track->year, 1, 1));
      ui_track_genre->setText( track->genre );
      ui_track_number->setText( QString::number(track->num) );
      ui_playcount->setValue( track->playcount );

      set_rating(true);
    }

    QObject::connect(ui_rating, SIGNAL(RatingChanged(float)), this, SLOT(slot_rating_changed(float)));
}

void MediaItem_Edit_Dialog::setGenre(const QString& genre)
{
    // shall be done before setMediaItem
    genre_     = genre;
}


/*******************************************************************************
   slot_clear_playcount
*******************************************************************************/
void MediaItem_Edit_Dialog::slot_clear_playcount()
{
    ui_playcount->setValue(0);
}

/*******************************************************************************
   slot_rating_changed
*******************************************************************************/
void MediaItem_Edit_Dialog::slot_rating_changed(float rating)
{
Q_UNUSED(rating)
    //Debug::debug() << " MediaItem_Edit_Dialog::slot_rating_changed";
    switch (media_->type()) {
      case TYPE_ARTIST : MEDIA::ArtistPtr::staticCast(media_)->isUserRating = true; break;
      case TYPE_ALBUM  : MEDIA::AlbumPtr::staticCast(media_)->isUserRating = true; break;
      default: break;
    }

    if( ui_clear_rating )
    {
        ui_clear_rating->setText(tr("clear user rating") );
        ui_clear_rating->setEnabled (true);
    }

    ui_rating->set_user_rating(true);
}

/*******************************************************************************
   slot_clear_rating
*******************************************************************************/
void MediaItem_Edit_Dialog::slot_clear_rating()
{
    switch (media_->type()) {
      case TYPE_ARTIST : MEDIA::ArtistPtr::staticCast(media_)->isUserRating = false; break;
      case TYPE_ALBUM  : MEDIA::AlbumPtr::staticCast(media_)->isUserRating  = false; break;
      default: break;
    }

    set_rating( false );
}

void MediaItem_Edit_Dialog::set_rating(bool isUser)
{
    //Debug::debug() << "set_rating USER : " << isUser;
    switch (media_->type()) {
      case TYPE_ARTIST : MEDIA::ArtistPtr::staticCast(media_)->isUserRating = isUser; break;
      case TYPE_ALBUM  : MEDIA::AlbumPtr::staticCast(media_)->isUserRating  = isUser; break;
      default: break;
    }

    if(isUser)
    {
      ui_rating->set_rating( MEDIA::rating(media_) );

      if( ui_clear_rating )
      {
        ui_clear_rating->setText(tr("clear user rating") );
        ui_clear_rating->setEnabled (true);
      }
    }
    else
    {
      float auto_rating = LocalTrackModel::instance()->getItemAutoRating(media_);
      ui_rating->set_rating( auto_rating  );

      if( ui_clear_rating )
      {
        ui_clear_rating->setText(tr("auto rating"));
        ui_clear_rating->setEnabled (false);
      }
    }

    ui_rating->set_user_rating(isUser);
}


/*******************************************************************************
    GUI creation
*******************************************************************************/
void MediaItem_Edit_Dialog::create_gui()
{
    switch(dialog_type_)
    {
      case DIALOG_ARTIST : create_gui_artist(); break;
      case DIALOG_ALBUM : create_gui_album(); break;
      case DIALOG_GENRE : create_gui_genre(); break;
      case DIALOG_TRACK : create_gui_track(); break;
      default:break;
    }
}

void MediaItem_Edit_Dialog::create_gui_artist()
{
    this->resize(445, 150);
    this->setTitle(tr("Edit artist"));

    ui_artist_name = new QLineEdit();
    ui_playcount   = new QSpinBox();
    ui_rating      = new RatingWidget();

    ui_clear_playcount = new QPushButton(tr("clear playcount"));
    ui_clear_rating    = new QPushButton();

    ui_artist_name->setMinimumHeight(25);
    ui_playcount->setMinimumHeight(25);
    ui_rating->setMinimumHeight(25);

    ui_artist_name->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_playcount->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    sizePolicy.setHeightForWidth(ui_rating->sizePolicy().hasHeightForWidth());
    ui_rating->setSizePolicy(sizePolicy);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(0, 0, 0, 0);

    gridLayout->addWidget(new QLabel(tr("Artist")), 0, 0, 1, 1);
    gridLayout->addWidget(ui_artist_name, 0, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Playcount")), 1, 0, 1, 1);
    gridLayout->addWidget(ui_playcount, 1, 1, 1, 1);
    gridLayout->addWidget(ui_clear_playcount, 1, 2, 1, 1);
    gridLayout->addWidget(new QLabel(tr("Rating")), 2, 0, 1, 1);
    gridLayout->addWidget(ui_rating, 2, 1, 1, 1);
    gridLayout->addWidget(ui_clear_rating, 2, 2, 1, 1);

    /* layout content */
    setContentLayout(gridLayout);    
}

void MediaItem_Edit_Dialog::create_gui_album()
{
    this->resize(445, 150);
    this->setTitle(tr("Edit album"));

    ui_album_name  = new QLineEdit();
    ui_artist_name = new QLineEdit();
    ui_album_year  = new QDateEdit();
    ui_playcount   = new QSpinBox();
    ui_rating      = new RatingWidget();

    ui_album_year->setDisplayFormat("yyyy");

    ui_clear_playcount = new QPushButton(tr("clear playcount"));
    ui_clear_rating    = new QPushButton();

    ui_album_name->setMinimumHeight(25);
    ui_artist_name->setMinimumHeight(25);
    ui_album_year->setMinimumHeight(25);
    ui_playcount->setMinimumHeight(25);
    ui_rating->setMinimumHeight(25);

    ui_album_name->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_artist_name->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_album_year->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_playcount->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    sizePolicy.setHeightForWidth(ui_rating->sizePolicy().hasHeightForWidth());
    ui_rating->setSizePolicy(sizePolicy);


    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(0, 0, 0, 0);

    gridLayout->addWidget(new QLabel(tr("Album")), 0, 0, 1, 1);
    gridLayout->addWidget(ui_album_name, 0, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Artist")), 1, 0, 1, 1);
    gridLayout->addWidget(ui_artist_name, 1, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Year")), 2, 0, 1, 1);
    gridLayout->addWidget(ui_album_year, 2, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Playcount")), 3, 0, 1, 1);
    gridLayout->addWidget(ui_playcount, 3, 1, 1, 1);
    gridLayout->addWidget(ui_clear_playcount, 3, 2, 1, 1);
    gridLayout->addWidget(new QLabel(tr("Rating")), 4, 0, 1, 1);
    gridLayout->addWidget(ui_rating, 4, 1, 1, 1);
    gridLayout->addWidget(ui_clear_rating, 4,2,1,1);

   
    /* layout content */
    setContentLayout(gridLayout);       
}

void MediaItem_Edit_Dialog::create_gui_genre()
{
    this->resize(445, 150);
    this->setTitle(tr("Edit album"));    

    ui_album_name  = new QLineEdit();
    ui_artist_name = new QLineEdit();
    ui_album_year  = new QDateEdit();
    ui_album_genre = new QLineEdit();
    ui_playcount   = new QSpinBox();
    ui_rating      = new RatingWidget();

    ui_album_year->setDisplayFormat("yyyy");

    ui_clear_playcount = new QPushButton(tr("clear playcount"));
    ui_clear_rating    = new QPushButton();

    ui_album_name->setMinimumHeight(25);
    ui_artist_name->setMinimumHeight(25);
    ui_album_year->setMinimumHeight(25);
    ui_playcount->setMinimumHeight(25);
    ui_album_genre->setMinimumHeight(25);
    ui_rating->setMinimumHeight(25);

    ui_album_name->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_artist_name->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_album_year->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_playcount->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_album_genre->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    sizePolicy.setHeightForWidth(ui_rating->sizePolicy().hasHeightForWidth());
    ui_rating->setSizePolicy(sizePolicy);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(0, 0, 0, 0);

    gridLayout->addWidget(new QLabel(tr("Album")), 0, 0, 1, 1);
    gridLayout->addWidget(ui_album_name, 0, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Artist")), 1, 0, 1, 1);
    gridLayout->addWidget(ui_artist_name, 1, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Year")), 2, 0, 1, 1);
    gridLayout->addWidget(ui_album_year, 2, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Genre")), 3, 0, 1, 1);
    gridLayout->addWidget(ui_album_genre, 3, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Playcount")), 4, 0, 1, 1);
    gridLayout->addWidget(ui_playcount, 4, 1, 1, 1);
    gridLayout->addWidget(ui_clear_playcount, 4, 2, 1, 1);
    gridLayout->addWidget(new QLabel(tr("Rating")), 5, 0, 1, 1);
    gridLayout->addWidget(ui_rating, 5, 1, 1, 1);
    gridLayout->addWidget(ui_clear_rating, 5,2,1,1);

    /* layout content */
    setContentLayout(gridLayout);      
}

void MediaItem_Edit_Dialog::create_gui_track()
{
    this->resize(445, 150);
    this->setTitle(tr("Edit track"));    

    ui_filename     = new QLineEdit();
    ui_track_title  = new QLineEdit();
    ui_album_name   = new QLineEdit();
    ui_artist_name  = new QLineEdit();
    ui_track_year   = new QDateEdit();
    ui_track_genre  = new QLineEdit();
    ui_track_number = new QLineEdit();
    ui_playcount    = new QSpinBox();
    ui_rating       = new RatingWidget();

    ui_filename->setStyleSheet(QString::fromUtf8("QLineEdit {background: transparent;}"));
    ui_filename->setFrame(false);
    ui_filename->setReadOnly(true);

    ui_track_year->setDisplayFormat("yyyy");

    ui_clear_playcount = new QPushButton(tr("clear playcount"));
    ui_clear_rating    = 0;

    ui_track_title->setMinimumHeight(25);
    ui_album_name->setMinimumHeight(25);
    ui_artist_name->setMinimumHeight(25);
    ui_track_year->setMinimumHeight(25);
    ui_track_genre->setMinimumHeight(25);
    ui_track_number->setMinimumHeight(25);
    ui_playcount->setMinimumHeight(25);
    ui_rating->setMinimumHeight(25);

    ui_track_title->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_album_name->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_artist_name->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_track_year->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_track_genre->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_track_number->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui_playcount->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    sizePolicy.setHeightForWidth(ui_rating->sizePolicy().hasHeightForWidth());
    ui_rating->setSizePolicy(sizePolicy);


    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(0, 0, 0, 0);

    gridLayout->addWidget(new QLabel(tr("File")), 0, 0, 1, 1);
    gridLayout->addWidget(ui_filename, 0, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Title")), 1, 0, 1, 1);
    gridLayout->addWidget(ui_track_title, 1, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Album")), 2, 0, 1, 1);
    gridLayout->addWidget(ui_album_name, 2, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Artist")), 3, 0, 1, 1);
    gridLayout->addWidget(ui_artist_name, 3, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Year")), 4, 0, 1, 1);
    gridLayout->addWidget(ui_track_year, 4, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Genre")), 5, 0, 1, 1);
    gridLayout->addWidget(ui_track_genre, 5, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Number")), 6, 0, 1, 1);
    gridLayout->addWidget(ui_track_number, 6, 1, 1, 2);

    gridLayout->addWidget(new QLabel(tr("Playcount")), 7, 0, 1, 1);
    gridLayout->addWidget(ui_playcount, 7, 1, 1, 1);
    gridLayout->addWidget(ui_clear_playcount, 7, 2, 1, 1);

    gridLayout->addWidget(new QLabel(tr("Rating")), 8, 0, 1, 1);
    gridLayout->addWidget(ui_rating, 8, 1, 1, 1);

    /* layout content */
    setContentLayout(gridLayout);      
}


/*******************************************************************************
    on_buttonBox_rejected
*******************************************************************************/
void MediaItem_Edit_Dialog::on_buttonBox_rejected()
{
    this->setResult(QDialog::Rejected);
    QDialog::reject();
    this->close();
}
/*******************************************************************************
    on_buttonBox_accepted
*******************************************************************************/
void MediaItem_Edit_Dialog::on_buttonBox_accepted()
{
    Debug::debug() << "MediaItem_Edit_Dialog::on_buttonBox_accepted";

    switch(dialog_type_)
    {
      case DIALOG_ARTIST : do_changes_artist(); break;
      case DIALOG_ALBUM : do_changes_album(); break;
      case DIALOG_GENRE : do_changes_album_genre(); break;
      case DIALOG_TRACK : do_changes_track(); break;
      default:break;
    }

    this->setResult(QDialog::Accepted);
    QDialog::accept();
    this->close();
}

/*******************************************************************************
    do_changes_artist
*******************************************************************************/
void MediaItem_Edit_Dialog::do_changes_artist()
{
    MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(media_);

    //! evaluate change
    if( artist->name        == ui_artist_name->text() &&
        artist->playcount   == ui_playcount->value()  &&
       ((artist->rating == ui_rating->rating())&& artist->isUserRating)
    )
    {
      Debug::debug() << "no change done --> close";
      isChanged_ = false;
      this->close();
      return;
    }

    Debug::debug() << "change done --> apply";
    isChanged_ = true;


    //! apply new tag values
    QString  old_artist_name         = artist->name;
    artist->name                     = ui_artist_name->text();
    artist->playcount                = ui_playcount->value();
    artist->rating                   = artist->isUserRating ? ui_rating->rating() : -1.0;

    //! apply changes to database
    Database db;
    if (!db.connect()) return;

    QSqlQuery("BEGIN TRANSACTION;",*db.sqlDb());

    //! ARTIST part in database
    int new_artist_id;
    {
      QSqlQuery q("",*db.sqlDb());
      q.prepare("SELECT id FROM artists WHERE name=?;");
      q.addBindValue( artist->name );
      Debug::debug() << "database -> select artist : " << q.exec();

      if ( !q.next() )
      {
          q.prepare("INSERT INTO artists(name,favorite,playcount,rating) VALUES(?,?,?,?);");
          q.addBindValue(artist->name);
          q.addBindValue((int)artist->isFavorite);
          q.addBindValue(artist->playcount);
          q.addBindValue(artist->rating);
          Debug::debug() << "database -> insert artist : " << q.exec();

          q.prepare("SELECT id FROM artists WHERE name=?;");
          q.addBindValue(artist->name);
          q.exec();
          q.next();
          new_artist_id = q.value(0).toInt();
      }
      else
      {
          new_artist_id = q.value(0).toInt();

          q.prepare("UPDATE artists SET name=?,playcount=?,rating=? WHERE id=?;");
          q.addBindValue(artist->name);
          q.addBindValue(artist->playcount);
          q.addBindValue(artist->rating);
          q.addBindValue(new_artist_id);
          Debug::debug() << "database -> update artist : " << q.exec();
      }
      Debug::debug() << "database -> artist id : " << new_artist_id;
    }

    int new_album_id;
    //! ALBUM Part in database
    foreach(MEDIA::MediaPtr media, artist->children())
    {
      MEDIA::AlbumPtr album   = MEDIA::AlbumPtr::staticCast(media);

      QSet<int> disc_numbers;
      if(album->isMultiset()) {
        foreach(MEDIA::MediaPtr media, album->children())
          disc_numbers.insert( MEDIA::TrackPtr::staticCast(media)->disc_number );
      }
      else
       disc_numbers.insert( album->disc_number );
      

      foreach (const int &disc_number, disc_numbers)
      {
        //Debug::debug() << "QSet value " << value;

        const QString  old_cover_name = MEDIA::coverName(old_artist_name,album->name);
        const QString  new_cover_name = MEDIA::coverName(artist->name,album->name);
        if(old_cover_name != new_cover_name)
          recupCoverArt(new_cover_name,old_cover_name);

        // check if same album with new artist exist
        QSqlQuery q("",*db.sqlDb());
        q.prepare("SELECT `id` FROM `albums` WHERE `name`=? AND `disc`=? AND `artist_id`=?;");
        q.addBindValue( album->name );
        q.addBindValue( disc_number );
        q.addBindValue( new_artist_id );
        Debug::debug() << "database -> select album/artist : " << q.exec();


        if ( !q.next() )
        {
           q.prepare("INSERT INTO albums(name,artist_id,cover,year,favorite,playcount,rating,disc) VALUES(?,?,?,?,?,?,?,?);");
           q.addBindValue( album->name );
           q.addBindValue( new_artist_id );
           q.addBindValue( new_cover_name );
           q.addBindValue( album->year );
           q.addBindValue( (int)album->isFavorite );
           q.addBindValue( album->playcount );
           q.addBindValue( album->rating );
           q.addBindValue( disc_number );
           Debug::debug() << "database -> insert album : " << q.exec();

         q.prepare("SELECT `id` FROM `albums` WHERE `name`=? AND `disc`=? AND `artist_id`=?;");
         q.addBindValue( album->name );
         q.addBindValue( disc_number );
         q.addBindValue( new_artist_id );
         q.exec();
         q.next();
         new_album_id = q.value(0).toInt();
       }
        else {
          new_album_id = q.value(0).toInt();
          q.prepare("UPDATE albums SET name=?,artist_id=?,cover=?,year=?,playcount=?,rating=? WHERE id=?;");
          q.addBindValue( album->name );
          q.addBindValue( new_artist_id );
          q.addBindValue( new_cover_name );
          q.addBindValue( album->year );
          q.addBindValue( album->playcount );
          q.addBindValue( album->rating );
          q.addBindValue( new_album_id );
          Debug::debug() << "database -> update album : " << q.exec();
        }
        Debug::debug() << "database -> album_id: " << new_album_id;


        foreach(MEDIA::MediaPtr media, album->children())
        {
          MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(media);

          /* skip track that not belong to disc number/album */
          if(album->isMultiset() && ( track->disc_number != disc_number ))
              continue;

          int track_id = track->id;

          QSqlQuery q("", *db.sqlDb());
          q.prepare("UPDATE tracks SET album_id=?, artist_id=? WHERE id=?;");
          q.addBindValue(new_album_id);
          q.addBindValue(new_artist_id);
          q.addBindValue(track_id);

          Debug::debug() << "database -> update track : " << q.exec();
        } // END FOREACH tracks
      } // END for each albums set
    } // END FOREACH albums


    //! Database Clean
    QSqlQuery query("DELETE FROM artists WHERE id NOT IN (SELECT artist_id FROM tracks GROUP BY artist_id);", *db.sqlDb());

    QSqlQuery("COMMIT TRANSACTION;",*db.sqlDb());

}

/*******************************************************************************
    do_changes_album
*******************************************************************************/
void MediaItem_Edit_Dialog::do_changes_album()
{
    MEDIA::AlbumPtr  album  = MEDIA::AlbumPtr::staticCast(media_);
    MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(media_->parent());

    //! evaluate change
    if( album->name            == ui_album_name->text()   &&
        artist->name           == ui_artist_name->text()  &&
        QDate(QString(album->year).toInt(),1,1) == ui_album_year->date()   &&
        album->playcount       == ui_playcount->value()   &&
        ((album->rating == ui_rating->rating()) && album->isUserRating)
    )
    {
      Debug::debug() << "no change done --> close";
      isChanged_ = false;
      this->close();
      return;
    }

    Debug::debug() << "change done --> apply";
    isChanged_ = true;


    //! keep old values to be restored
    const QString  old_cover_name  = album->coverpath;

    //! apply new tag values
    album->name            = ui_album_name->text();
    album->year            = ui_album_year->date().year();
    artist->name           = ui_artist_name->text();
    album->playcount       = ui_playcount->value();
    album->rating          = album->isUserRating ? ui_rating->rating() : -1.0;

    foreach ( MEDIA::MediaPtr media, media_->children())
    {
      MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(media);
      track->album   = ui_album_name->text();
      track->artist  = ui_artist_name->text();
      track->year    = ui_album_year->date().year();
    }

    const QString  new_cover_name = MEDIA::coverName(artist->name,album->name);


    //! apply to database
    Database db;
    if (!db.connect()) return;
    QSqlQuery("BEGIN TRANSACTION;",*db.sqlDb());

    //! YEAR part in database
    {
       QSqlQuery q("", *db.sqlDb());
       q.prepare("SELECT `id` FROM `years` WHERE `year`=:val;");
       q.bindValue(":val", album->year );
       q.exec();

       if ( !q.next() ) {
         q.prepare("INSERT INTO `years`(`year`) VALUES (:val);");
         q.bindValue(":val", album->year);
         Debug::debug() << "database -> insert year :" << q.exec();
       }
    }

    //! ARTIST part in database
    int artist_id;
    {
      QSqlQuery q("",*db.sqlDb());
      q.prepare("SELECT id FROM artists WHERE name=?;");
      q.addBindValue( artist->name );
      Debug::debug() << "database -> select artist : " << q.exec();

      if ( !q.next() )
      {
          q.prepare("INSERT INTO artists(name,favorite,playcount,rating) VALUES(?,?,?,?);");
          q.addBindValue(artist->name);
          q.addBindValue((int)artist->isFavorite);
          q.addBindValue(artist->playcount);
          q.addBindValue(artist->rating);
          Debug::debug() << "database -> insert artist : " << q.exec();

          q.prepare("SELECT id FROM artists WHERE name=?;");
          q.addBindValue(artist->name);
          q.exec();
          q.next();
      }
      artist_id = q.value(0).toInt();
      Debug::debug() << "database -> artist id : " << artist_id;
    }

    int album_id;
    //! ALBUM part in database
    {
      /* handle multiset album */
      QSet<int> disc_numbers;
      if(album->isMultiset()) {
        foreach(MEDIA::MediaPtr media, album->children())
          disc_numbers.insert( MEDIA::TrackPtr::staticCast(media)->disc_number );
      }
      else
       disc_numbers.insert( album->disc_number );      
      
      
      foreach (const int &disc_number, disc_numbers)
      {
        QSqlQuery q("",*db.sqlDb());
        q.prepare("SELECT `id` FROM `albums` WHERE `name`=? AND `disc`=? AND `artist_id`=?;");
        q.addBindValue( album->name );
        q.addBindValue( disc_number );
        q.addBindValue( artist_id );
        Debug::debug() << "database -> select album/artist : " << q.exec();

        if ( !q.next() )
        {
           q.prepare("INSERT INTO albums(name,artist_id,cover,year,favorite,playcount,rating,disc) VALUES(?,?,?,?,?,?,?,?);");
           q.addBindValue( album->name );
           q.addBindValue( artist_id );
           q.addBindValue( new_cover_name );
           q.addBindValue( album->year );
           q.addBindValue( (int)album->isFavorite );
           q.addBindValue( album->playcount );
           q.addBindValue( album->rating );
           q.addBindValue( disc_number );
           Debug::debug() << "database -> insert album : " << q.exec();

           q.prepare("SELECT `id` FROM `albums` WHERE `name`=? AND `disc`=? AND `artist_id`=?;");
           q.addBindValue( album->name );
           q.addBindValue( disc_number );
           q.addBindValue( artist_id );
           q.exec();
           q.next();
           album_id = q.value(0).toInt();
        }
        else {
          album_id = q.value(0).toInt();
          q.prepare("UPDATE albums SET name=?,artist_id=?,cover=?,year=?,playcount=?,rating=? WHERE id=?;");
          q.addBindValue( album->name );
          q.addBindValue( artist_id );
          q.addBindValue( new_cover_name );
          q.addBindValue( album->year );
          q.addBindValue( album->playcount );
          q.addBindValue( album->rating );
          q.addBindValue( album_id );
          Debug::debug() << "database -> update album : " << q.exec();
        }
        Debug::debug() << "database -> album_id: " << album_id;
      

        //! TRACK part in database
        foreach ( MEDIA::MediaPtr media, album->children())
        {
            MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(media);
            int trackId = track->id;

            /* skip track that not belong to disc number/album */
            if(album->isMultiset() && ( track->disc_number != disc_number ))
              continue;


            QSqlQuery q("",*db.sqlDb());
            q.prepare("UPDATE tracks SET artist_id=?,album_id=?,year_id=(SELECT id FROM years WHERE year=?) WHERE id=?;");
            q.addBindValue( artist_id );
            q.addBindValue( album_id );
            q.addBindValue( track->year );
            q.addBindValue( trackId );
            Debug::debug() << "database -> update tracks id  " << trackId << " : " << q.exec();
        }
      } // end multiset album loop
    } // end album

    //! Just once , If old cover exist -> take it !!
    recupCoverArt(new_cover_name,old_cover_name);

    //! Database Clean
    QSqlQuery q1("DELETE FROM `albums` WHERE `id` NOT IN (SELECT `album_id` FROM `tracks` GROUP BY `album_id`);", *db.sqlDb());
    QSqlQuery q2("DELETE FROM `genres` WHERE `id` NOT IN (SELECT `genre_id` FROM `tracks` GROUP BY `genre_id`);", *db.sqlDb());
    QSqlQuery q3("DELETE FROM `artists` WHERE `id` NOT IN (SELECT `artist_id` FROM `tracks` GROUP BY `artist_id`);", *db.sqlDb());
    QSqlQuery q4("DELETE FROM `years` WHERE `id` NOT IN (SELECT `year_id` FROM `tracks` GROUP BY `year_id`);", *db.sqlDb());

    QSqlQuery("COMMIT TRANSACTION;",*db.sqlDb());
}


/*******************************************************************************
    do_changes_album_genre
*******************************************************************************/
void MediaItem_Edit_Dialog::do_changes_album_genre()
{
    MEDIA::AlbumPtr album   = MEDIA::AlbumPtr::staticCast(media_);
    MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(media_->parent());

    //! evaluate change
    if( album->name            == ui_album_name->text()  &&
        artist->name           == ui_artist_name->text() &&
        QDate(QString(album->year).toInt(),1,1) == ui_album_year->date()  &&
        album->playcount       == ui_playcount->value()  &&
        genre_                 == ui_album_genre->text() &&
        ((album->rating == ui_rating->rating()) && album->isUserRating)
    )
    {
      Debug::debug() << "no change done --> close";
      isChanged_ = false;
      this->close();
      return;
    }

    Debug::debug() << "change done --> apply";
    isChanged_ = true;


    //! keep old values to be restored
    const QString  old_cover_name       = album->coverpath;

    //! apply new tag values
    album->name            = ui_album_name->text();
    album->year            = ui_album_year->date().year();
    artist->name           = ui_artist_name->text();
    album->playcount       = ui_playcount->value();
    album->rating          = album->isUserRating ? ui_rating->rating() : -1.0;

    foreach ( MEDIA::MediaPtr media, album->children())
    {
      MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(media);
      track->album   = ui_album_name->text();
      track->artist  = ui_artist_name->text();
      track->year    = ui_album_year->date().year();
      track->genre   = ui_album_genre->text();
    }

    const QString  new_cover_name = MEDIA::coverName(artist->name,album->name );


    //! apply to database
    Database db;
    if (!db.connect()) return;
    QSqlQuery("BEGIN TRANSACTION;",*db.sqlDb());


    //! GENRE part in database
    {
       QSqlQuery q("", *db.sqlDb());
       q.prepare("SELECT `id` FROM `genres` WHERE `genre`=:val;");
       q.bindValue(":val", ui_album_genre->text() );
       q.exec();

       if ( !q.next() ) {
         q.prepare("INSERT INTO `genres`(`genre`) VALUES (:val);");
         q.bindValue(":val", ui_album_genre->text());
         Debug::debug() << "database -> insert year :" << q.exec();
       }
    }

    //! YEAR part in database
    {
       QSqlQuery q("", *db.sqlDb());
       q.prepare("SELECT `id` FROM `years` WHERE `year`=:val;");
       q.bindValue(":val", album->year );
       q.exec();

       if ( !q.next() ) {
         q.prepare("INSERT INTO `years`(`year`) VALUES (:val);");
         q.bindValue(":val", album->year);
         Debug::debug() << "database -> insert year :" << q.exec();
       }
    }


    //! ARTIST part in database
    int artist_id;
    {
      QSqlQuery q("",*db.sqlDb());
      q.prepare("SELECT id FROM artists WHERE name=?;");
      q.addBindValue( artist->name );
      Debug::debug() << "database -> select artist : " << q.exec();

      if ( !q.next() )
      {
          q.prepare("INSERT INTO artists(name,favorite,playcount,rating) VALUES(?,?,?,?);");
          q.addBindValue(artist->name);
          q.addBindValue((int)artist->isFavorite);
          q.addBindValue(artist->playcount);
          q.addBindValue(artist->rating);
          Debug::debug() << "database -> insert artist : " << q.exec();

          q.prepare("SELECT id FROM artists WHERE name=?;");
          q.addBindValue(artist->name);
          q.exec();
          q.next();
      }
      artist_id = q.value(0).toInt();
      Debug::debug() << "database -> artist id : " << artist_id;
    }


    int album_id;
    //! ALBUM part in databasealbum
    {
      
      /* handle multiset album */
      QSet<int> disc_numbers;
      if(album->isMultiset()) {
        foreach(MEDIA::MediaPtr media, album->children())
          disc_numbers.insert( MEDIA::TrackPtr::staticCast(media)->disc_number );
      }
      else
       disc_numbers.insert( album->disc_number );      
      
      
      foreach (const int &disc_number, disc_numbers)
      {      
        QSqlQuery q("",*db.sqlDb());
        q.prepare("SELECT `id` FROM `albums` WHERE `name`=? AND `disc`=? AND `artist_id`=?;");
        q.addBindValue( album->name );
        q.addBindValue( disc_number );
        q.addBindValue( artist_id );
        Debug::debug() << "database -> select album/artist : " << q.exec();

        if ( !q.next() )
        {
           q.prepare("INSERT INTO albums(name,artist_id,cover,year,favorite,playcount,rating,disc) VALUES(?,?,?,?,?,?,?,?);");
           q.addBindValue( album->name );
           q.addBindValue( artist_id );
           q.addBindValue( new_cover_name );
           q.addBindValue( album->year );
           q.addBindValue( (int)album->isFavorite );
           q.addBindValue( album->playcount );
           q.addBindValue( album->rating );
           q.addBindValue( disc_number );
           Debug::debug() << "database -> insert album : " << q.exec();

           q.prepare("SELECT `id` FROM `albums` WHERE `name`=? AND `disc`=? AND `artist_id`=?;");
           q.addBindValue( album->name );
           q.addBindValue( disc_number );
           q.addBindValue( artist_id );
           q.exec();
           q.next();
           album_id = q.value(0).toInt();
        }
        else {
          album_id = q.value(0).toInt();
          q.prepare("UPDATE albums SET name=?,artist_id=?,cover=?,year=?,playcount=?,rating=? WHERE id=?;");
          q.addBindValue( album->name );
          q.addBindValue( artist_id );
          q.addBindValue( new_cover_name );
          q.addBindValue( album->year );
          q.addBindValue( album->playcount );
          q.addBindValue( album->rating );
          q.addBindValue( album_id );
          Debug::debug() << "database -> update album : " << q.exec();
        }
        Debug::debug() << "database -> album_id: " << album_id;

   
        //! TRACK part in database
        foreach ( MEDIA::MediaPtr media, media_->children())
        {
          MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(media);

          /* skip track that not belong to disc number/album */
          if(album->isMultiset() && ( track->disc_number != disc_number ))
            continue;

          int trackId = track->id;

          QSqlQuery q("",*db.sqlDb());
          q.prepare("UPDATE tracks SET artist_id=?,album_id=?,year_id=(SELECT id FROM years WHERE year=?),genre_id=(SELECT id FROM genres WHERE genre=?) WHERE id=?;");
          q.addBindValue( artist_id );
          q.addBindValue( album_id );
          q.addBindValue( track->year );
          q.addBindValue( ui_album_genre->text() );
          q.addBindValue( trackId );
          Debug::debug() << "database -> update tracks id  " << trackId << " : " << q.exec();
        }
      } // end multiset album loop
    }


    //! Just once , If old cover exist -> take it !!
    recupCoverArt(new_cover_name,old_cover_name);

    //! Database Clean
    QSqlQuery q1("DELETE FROM `albums` WHERE `id` NOT IN (SELECT `album_id` FROM `tracks` GROUP BY `album_id`);", *db.sqlDb());
    QSqlQuery q2("DELETE FROM `genres` WHERE `id` NOT IN (SELECT `genre_id` FROM `tracks` GROUP BY `genre_id`);", *db.sqlDb());
    QSqlQuery q3("DELETE FROM `artists` WHERE `id` NOT IN (SELECT `artist_id` FROM `tracks` GROUP BY `artist_id`);", *db.sqlDb());
    QSqlQuery q4("DELETE FROM `years` WHERE `id` NOT IN (SELECT `year_id` FROM `tracks` GROUP BY `year_id`);", *db.sqlDb());

    QSqlQuery("COMMIT TRANSACTION;",*db.sqlDb());
}


/*******************************************************************************
    do_changes_track
*******************************************************************************/
void MediaItem_Edit_Dialog::do_changes_track()
{
    MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(media_);

    //! evaluate change
    if( track->title           == ui_track_title->text() &&
        track->album           == ui_album_name->text()  &&
        track->artist          == ui_artist_name->text() &&
        track->genre           == ui_track_genre->text() &&
        QDate(track->year,1,1) == ui_track_year->date()  &&
        track->playcount       == ui_playcount->value()  &&
        track->rating          == ui_rating->rating()
    )
    {
      Debug::debug() << "no change done --> close";
      isChanged_ = false;
      this->close();
      return;
    }

    Debug::debug() << "change done --> apply";
    isChanged_ = true;

    //! apply new tag values
    track->title      =  ui_track_title->text();
    track->album      =  ui_album_name->text();
    track->artist     =  ui_artist_name->text();
    track->genre      =  ui_track_genre->text();
    track->year       =  ui_track_year->date().year();
    track->playcount  =  ui_playcount->value();
    track->rating     =  ui_rating->rating();

    MEDIA::AlbumPtr  album_media   = MEDIA::AlbumPtr::staticCast(media_->parent());
    MEDIA::ArtistPtr artist_media  = MEDIA::ArtistPtr::staticCast(album_media->parent());

    //! apply to database
    Database db;
    if (!db.connect()) return;
    QSqlQuery("BEGIN TRANSACTION;",*db.sqlDb());


    //! GENRE part in database
    {
       QSqlQuery q("", *db.sqlDb());
       q.prepare("SELECT `id` FROM `genres` WHERE `genre`=:val;");
       q.bindValue(":val", track->genre );
       q.exec();

       if ( !q.next() ) {
         q.prepare("INSERT INTO `genres`(`genre`) VALUES (:val);");
         q.bindValue(":val", track->genre);
         Debug::debug() << "database -> insert year :" << q.exec();
       }
    }

    //! YEAR part in database
    {
       QSqlQuery q("", *db.sqlDb());
       q.prepare("SELECT `id` FROM `years` WHERE `year`=:val;");
       q.bindValue(":val", track->year );
       q.exec();

       if ( !q.next() ) {
         q.prepare("INSERT INTO `years`(`year`) VALUES (:val);");
         q.bindValue(":val", track->year);
         Debug::debug() << "database -> insert year :" << q.exec();
       }
    }


    //! ARTIST part in database
    int artist_id;
    {
      QSqlQuery q("",*db.sqlDb());
      q.prepare("SELECT id FROM artists WHERE name=?;");
      q.addBindValue( track->artist );
      Debug::debug() << "database -> select artist : " << q.exec();

      if ( !q.next() )
      {
          q.prepare("INSERT INTO artists(name,favorite,playcount,rating) VALUES(?,?,?,?);");
          q.addBindValue(track->artist);
          q.addBindValue((int)artist_media->isFavorite);
          q.addBindValue(artist_media->playcount);
          q.addBindValue(artist_media->rating);
          Debug::debug() << "database -> insert artist : " << q.exec();

          q.prepare("SELECT id FROM artists WHERE name=?;");
          q.addBindValue(track->artist);
          q.exec();
          q.next();
      }
      artist_id = q.value(0).toInt();
      Debug::debug() << "database -> artist id : " << artist_id;
    }


    //! ALBUM part in database
    int album_id;
    const QString  new_cover_name  = MEDIA::coverName(track->artist,track->album );
    {
      //! WARNING problem if string of album contain quote
      QSqlQuery q("",*db.sqlDb());
      
      q.prepare("SELECT `id` FROM `albums` WHERE `name`=? AND `disc`=? AND `artist_id`=?;");
      q.addBindValue( track->album );
      q.addBindValue( track->disc_number );
      q.addBindValue( artist_id );

      if ( !q.next() )
      {
         q.prepare("INSERT INTO albums(name,artist_id,cover,year,favorite,playcount,rating,disc) VALUES(?,?,?,?,?,?,?,?);");
         q.addBindValue( track->album );
         q.addBindValue( artist_id );
         q.addBindValue( new_cover_name );
         q.addBindValue( track->year );
         q.addBindValue( (int)album_media->isFavorite );
         q.addBindValue( album_media->playcount );
         q.addBindValue( album_media->rating );
         q.addBindValue( track->disc_number );
         Debug::debug() << "database -> insert album : " << q.exec();

         q.prepare("SELECT `id` FROM `albums` WHERE `name`=? AND `disc`=? AND `artist_id`=?;");
         q.addBindValue( track->album );
         q.addBindValue( track->disc_number );
         q.addBindValue( artist_id );
         q.exec();
         q.next();
         album_id = q.value(0).toInt();
      }
      else {
        album_id = q.value(0).toInt();
        q.prepare("UPDATE albums SET name=?,artist_id=?,cover=?,year=?,playcount=?,rating=? WHERE id=?;");
        q.addBindValue( track->album );
        q.addBindValue( artist_id );
        q.addBindValue( new_cover_name );
        q.addBindValue( track->year );
        q.addBindValue( album_media->playcount );
        q.addBindValue( album_media->rating );
        q.addBindValue( album_id );
        Debug::debug() << "database -> update album : " << q.exec();
      }
      Debug::debug() << "database -> album_id: " << album_id;
    }


    //! TRACK part in database
    {
        int trackId = track->id;

        QSqlQuery q("",*db.sqlDb());
        q.prepare("UPDATE tracks SET trackname=?,artist_id=?,album_id=?,year_id=(SELECT id FROM years WHERE year=?),genre_id=(SELECT id FROM genres WHERE genre=?), rating=? WHERE id=?;");
        q.addBindValue( track->title );
        q.addBindValue( artist_id );
        q.addBindValue( album_id );
        q.addBindValue( track->year );
        q.addBindValue( track->genre );
        q.addBindValue( track->rating );
        q.addBindValue( trackId );
        Debug::debug() << "database -> update tracks id  " << trackId << " : " << q.exec();
    }

    //! Database clean
    QSqlQuery q1("DELETE FROM `albums` WHERE `id` NOT IN (SELECT `album_id` FROM `tracks` GROUP BY `album_id`);", *db.sqlDb());
    QSqlQuery q2("DELETE FROM `genres` WHERE `id` NOT IN (SELECT `genre_id` FROM `tracks` GROUP BY `genre_id`);", *db.sqlDb());
    QSqlQuery q3("DELETE FROM `artists` WHERE `id` NOT IN (SELECT `artist_id` FROM `tracks` GROUP BY `artist_id`);", *db.sqlDb());
    QSqlQuery q4("DELETE FROM `years` WHERE `id` NOT IN (SELECT `year_id` FROM `tracks` GROUP BY `year_id`);", *db.sqlDb());

    QSqlQuery("COMMIT TRANSACTION;",*db.sqlDb());
}

