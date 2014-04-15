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


#ifndef _MEDIAITEM_DIALOG_H_
#define _MEDIAITEM_DIALOG_H_

#include "dialog_base.h"
#include "core/mediaitem/mediaitem.h"
#include "widgets/ratingwidget.h"

#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLineEdit>
#include <QtGui/QDateEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>


/*
********************************************************************************
*                                                                              *
*    Class MediaItem_Edit_Dialog                                               *
*                                                                              *
********************************************************************************
*/
typedef enum {
  DIALOG_ARTIST = 0,
  DIALOG_ALBUM,
  DIALOG_GENRE,
  DIALOG_TRACK} T_DIALOG_TYPE;

class MediaItem_Edit_Dialog : public DialogBase
{
Q_OBJECT
  public:
    MediaItem_Edit_Dialog(T_DIALOG_TYPE type);
    ~MediaItem_Edit_Dialog();

    void setMediaItem(MEDIA::MediaPtr media);
    void setGenre(const QString& genre);
    bool isChanged() {return isChanged_;}

  private:
    void create_gui();
    void create_gui_artist();
    void create_gui_album();
    void create_gui_genre();
    void create_gui_track();

    void do_changes_artist();
    void do_changes_album();
    void do_changes_album_genre();
    void do_changes_track();

    void set_rating(bool isUser);

  private:
    T_DIALOG_TYPE    dialog_type_;
    MEDIA::MediaPtr  media_;
    QString          genre_;
    bool             isChanged_;

    QLineEdit*        ui_artist_name;
    QLineEdit*        ui_album_name;
    QLineEdit*        ui_album_genre;
    QDateEdit*        ui_album_year;

    QLineEdit        *ui_filename;
    QLineEdit        *ui_track_title;
    QLineEdit        *ui_track_genre;
    QLineEdit        *ui_track_number;
    QDateEdit        *ui_track_year;

    QSpinBox*         ui_playcount;
    RatingWidget*     ui_rating;
    QPushButton*      ui_clear_playcount;
    QPushButton*      ui_clear_rating;

  private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
    void slot_clear_playcount();
    void slot_rating_changed(float);
    void slot_clear_rating();
};

#endif // _MEDIAITEM_DIALOG_H_
