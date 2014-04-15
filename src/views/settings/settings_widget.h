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
#ifndef _SETTINGS_WIDGETS_H_
#define _SETTINGS_WIDGETS_H_

// local
#include "core/database/databasemanager.h"
#include "widgets/dialogs/shortcutdialog.h"

// Qt
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include <QtGui>

class CategorieLayoutItem;
class ButtonItem;

/*
********************************************************************************
*                                                                              *
*    Class PageGeneral                                                         *
*                                                                              *
********************************************************************************
*/
class PageGeneral : public QGraphicsWidget
{
Q_OBJECT
  public:
    PageGeneral(QWidget* parentView = 0);

    void restoreSettings();
    void saveSettings();

    bool isSystrayChanged();
    bool isDbusChanged();
    bool isMprisChanged();

    void doLayout();
    QSize size();

    void setContentVisible(bool b);
    
  private:
    void createGui();

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );

  private slots:
    void slot_color_button_clicked();  
    void slot_on_titlebutton_clicked();
    
  private:
    QCheckBox              *ui_check_systray;
    QCheckBox              *ui_check_hideAtStartup;
    QCheckBox              *ui_check_enableMpris;
    QCheckBox              *ui_check_enableDbus;
    QPushButton            *ui_color_button;

    CategorieLayoutItem    *m_title;
    QGraphicsProxyWidget   *proxy_widget;
    QWidget                *m_parent;
    
    ButtonItem             *m_button;
    bool                   isOpen;
    
  signals:
    void layout_changed();
};


/*
********************************************************************************
*                                                                              *
*    Class PagePlayer                                                          *
*                                                                              *
********************************************************************************
*/
class PagePlayer : public QGraphicsWidget
{
Q_OBJECT
  public:
    PagePlayer(QWidget* parentView = 0);

    void restoreSettings();
    void saveSettings();

    void doLayout();
    QSize size();

    void setContentVisible(bool b);
    
  private:
    void createGui();

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );

  private slots:
    void slot_enable_replaygain();
    void slot_on_titlebutton_clicked();
  
  private:
    QCheckBox              *ui_stopOnPlayqueueClear;
    QCheckBox              *ui_restartPlayingAtStartup;
    QCheckBox              *ui_restorePlayqueue;

    QGroupBox              *ui_groupBox;
    QRadioButton           *ui_mode_track;
    QRadioButton           *ui_mode_album;
    QComboBox              *ui_comboEngine;
    
    QCheckBox              *ui_enable_replaygain;
    QComboBox              *ui_comboRGMode;
    
    CategorieLayoutItem    *m_title;
    QGraphicsProxyWidget   *proxy_widget;
    QWidget                *m_parent;
    
    ButtonItem             *m_button;
    bool                   isOpen;

  signals:
    void layout_changed();    
};



/*
********************************************************************************
*                                                                              *
*    Class PageLibrary                                                         *
*                                                                              *
********************************************************************************
*/
class PageLibrary : public QGraphicsWidget
{
Q_OBJECT
  public:
    PageLibrary(QWidget* parentView = 0);

    void restoreSettings();
    void saveSettings();

    bool isLibraryChanged() {return _isLibraryChanged;}
    bool isViewChanged();

    void doLayout();
    QSize size();

    void setContentVisible(bool b);
    
  private:
    void createGui();
    void addDatabaseParam(const QString& name, const DB::S_dbParam& dbParam);

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );

  private slots:
    void loadDatabaseParam(QString);
    void newDatabaseParam();
    void delDatabaseParam();
    void renameDatabaseParam();
    void enableMultiDatabase();
    void on_addPathButton_clicked();
    void on_removePathButton_clicked();
    void on_removeAllPathsButton_clicked();
    void slot_oncheckbox_clicked();
    void slot_on_titlebutton_clicked();

  private:
    bool         _isLibraryChanged; //! we need to rebuild database

    QListWidget            *ui_paths_list;
    QCheckBox              *ui_enable_multiDb;
    QCheckBox              *ui_auto_update;
    QCheckBox              *ui_search_cover;
    QCheckBox              *ui_group_albums;
    QComboBox              *ui_choose_db;

    QPushButton            *ui_add_path_button;
    QPushButton            *ui_remove_path_button;
    QPushButton            *ui_remove_all_button;

    QToolButton            *ui_db_new_button;
    QToolButton            *ui_db_del_button;
    QToolButton            *ui_db_rename_button;

    QMap<QString, DB::S_dbParam> m_db_params;

    CategorieLayoutItem    *m_title;
    QGraphicsProxyWidget   *proxy_widget;
    QWidget                *m_parent;
    
    ButtonItem             *m_button;
    bool                   isOpen;

  signals:
    void layout_changed();
};

/*
********************************************************************************
*                                                                              *
*    Class ShortcutGraphicItem                                                 *
*                                                                              *
********************************************************************************
*/
class ShortcutGraphicItem : public QGraphicsObject, public QGraphicsLayoutItem
{
Q_OBJECT
Q_INTERFACES(QGraphicsLayoutItem)

  public:
    ShortcutGraphicItem(const QString&, const QString&, QPixmap);

    QString m_name;
    QString m_key;
    QPixmap m_pixmap;
    bool    m_status;
    
  protected:
    // Inherited from QGraphicsLayoutItem
    void setGeometry(const QRectF &geom);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );

  private:
    QStyleOptionViewItemV4 opt;

  signals:
    void clicked();
};

/*
********************************************************************************
*                                                                              *
*    Class PageShortcut                                                        *
*                                                                              *
********************************************************************************
*/
class PageShortcut : public QGraphicsWidget
{
Q_OBJECT
  public:
    PageShortcut(QWidget* parentView = 0);
    ~PageShortcut();

    void restoreSettings();
    void saveSettings();

    bool isChanged() {return _isChanged;}

    void doLayout();
    QSize size();

    void setContentVisible(bool b);
    
  private:
    void createGui();

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );

  private slots:
    void slot_on_shorcutItem_clicked();
    void slot_on_titlebutton_clicked();

  private:
    bool                    _isChanged;

    QMap<QString, ShortcutGraphicItem*>  m_items;
    QWidget                 *m_parent;

    CategorieLayoutItem     *m_title;
    //QCheckBox             *ui_enable_shortcut;
    QGraphicsProxyWidget    *proxy_widget;
    
    ButtonItem              *m_button;
    bool                    isOpen;

  signals:
    void layout_changed();    
};


/*
********************************************************************************
*                                                                              *
*    Class PageScrobbler                                                       *
*                                                                              *
********************************************************************************
*/
class PageScrobbler : public QGraphicsWidget
{
Q_OBJECT
  public:
    PageScrobbler(QWidget* parentView = 0);

    void restoreSettings();
    void saveSettings();

    bool isChanged() {return _isChanged;}

    void doLayout();
    QSize size();

    void setContentVisible(bool b);
    
  private slots:
    void signInClicked();
    void slotSignInDone();
    void enableChange();
    void slot_on_titlebutton_clicked();
    
  private:
    void updateSignInStatus();

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );

  private:
    CategorieLayoutItem   *m_title;
    QGraphicsProxyWidget  *proxy_widget;
    QWidget               *m_parent;

    QCheckBox             *useLastFmScrobbler;
    QGroupBox             *accountGroupB;
    QLineEdit             *lineEdit_1;
    QLineEdit             *lineEdit_2;
    QPushButton           *signInButton;
    QLabel                *statusLabel;
    QLabel                *statusPixmap;

    bool                   _isChanged;
    bool                   _isEnableOld;
    
    ButtonItem             *m_button;
    bool                   isOpen;

  signals:
    void layout_changed();
 };
 
 
/*
********************************************************************************
*                                                                              *
*    Class PageSongInfo                                                        *
*                                                                              *
********************************************************************************
*/
class PageSongInfo : public QGraphicsWidget
{
Q_OBJECT
  public:
    PageSongInfo(QWidget* parentView = 0);

    void restoreSettings();
    void saveSettings();

    bool isChanged() {return _isChanged;}
 
     void doLayout();
     QSize size();

     void setContentVisible(bool b);
    
  private slots:
    void slot_on_titlebutton_clicked();
    void slot_item_changed(QListWidgetItem*);
    void slot_current_item_changed(QListWidgetItem*);
    void slot_move_up();
    void slot_move_down();

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );

  private:
    CategorieLayoutItem   *m_title;
    QGraphicsProxyWidget  *proxy_widget;

    QWidget               *m_parent;
    QListWidget           *ui_listwidget;
    QPushButton           *ui_move_up;
    QPushButton           *ui_move_down;
    
    ButtonItem             *m_button;

    bool                   isOpen;
    bool                   _isChanged;

  private:
    void Move(int);
    
  signals:
    void layout_changed();
 };
 
/*
********************************************************************************
*                                                                              *
*    Class BottomWidget                                                        *
*                                                                              *
********************************************************************************
*/
class BottomWidget : public QWidget
{
Q_OBJECT
  public:
    BottomWidget(QWidget* parentView = 0);

  private:
    QWidget               *m_parent;

    QPushButton  *ui_save_button;
    QPushButton  *ui_cancel_button;

signals:
    void save_clicked();
    void cancel_clicked();
 }; 
 
 
#endif // _SETTINGS_WIDGETS_H_
