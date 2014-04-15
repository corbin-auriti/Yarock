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

#include "menumodel.h"
#include "views.h"
#include "utilities.h"      // CONFIGDIR
#include "global_actions.h"
#include "debug.h"


#include <QDir>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QAction>
#include <QtGui/QStandardItem>



/*
********************************************************************************
*                                                                              *
*    Class MenuItem                                                            *
*                                                                              *
********************************************************************************
*/
class MenuItem: public QStandardItem
{
  public:
     MenuItem();
     MenuItem(const QString & text, const QIcon & icon);
};

MenuItem::MenuItem(): QStandardItem()
{
    setEditable(false);
    setSelectable(true);
}

MenuItem::MenuItem(const QString & text, const QIcon & icon ) : QStandardItem(icon,text)
{
    setEditable(false);
    setSelectable(true);
}




MenuModel* MenuModel::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class MenuModel                                                           *
*                                                                              *
********************************************************************************
*/
MenuModel::MenuModel(QObject *parent) : QStandardItemModel(parent)
{
    INSTANCE = this;
    populateMenu();
    configureMenuAction();
}


MenuModel::~MenuModel()
{
    delete m_radioItem;
    delete m_computerItem;
}

/*******************************************************************************
    populateMenu
*******************************************************************************/
void MenuModel::populateMenu()
{
   QStandardItem *rootItem = this->invisibleRootItem();

   /* -----------------------*/
   /* populate Root Item     */
   /* -----------------------*/
   MenuItem* item1 = new MenuItem(tr("home"), QIcon(":/images/go-home.png"));
   item1->setData(QVariant::fromValue(new QAction(QIcon(":/images/go-home.png"), tr("home"), this)), MenuActionRole);
   rootItem->appendRow(item1);
       
   MenuItem* item2 = new MenuItem(tr("music browser"), QIcon(":/images/folder-music-48x48.png"));
   item2->setData(QVariant::fromValue(new QAction(QIcon(":/images/folder-music-48x48.png"), tr("music browser"), this)), MenuActionRole);
   rootItem->appendRow(item2);

   MenuItem* item3 = new MenuItem(tr("playlist browser"), QIcon(":/images/media-playlist-48x48.png"));
   item3->setData(QVariant::fromValue(new QAction(QIcon(":/images/media-playlist-48x48.png"), tr("playlist browser"), this)), MenuActionRole);
   rootItem->appendRow(item3);

   MenuItem* item4 = new MenuItem(tr("radio browser"), QIcon(":/images/media-url-48x48.png"));
   item4->setData(QVariant::fromValue(new QAction(QIcon(":/images/media-url-48x48.png"), tr("radio browser"), this)), MenuActionRole);
   rootItem->appendRow(item4);

   m_computerItem = new MenuItem(tr("computer"), QIcon(":/images/computer-48x48.png"));
   m_computerItem->setData(QVariant::fromValue(new QAction(QIcon(":/images/computer-48x48.png"), tr("computer"), this)), MenuActionRole);
   rootItem->appendRow(m_computerItem);
   
   
   /* -----------------------*/
   /* populate Home Item     */
   /* -----------------------*/   
   MenuItem* item10 = new MenuItem(tr("settings"), QIcon(":/images/settings.png"));
   item10->setData(QVariant::fromValue(new QAction(QIcon(":/images/settings.png"), tr("settings"), this)), MenuActionRole);
   item10->setData(QVariant(int(VIEW::ViewSettings)), ViewModeRole);
   item1->appendRow(item10);

   MenuItem* item11 = new MenuItem(tr("context"), QIcon(":/images/info-48x48.png"));
   item11->setData(QVariant::fromValue(new QAction(QIcon(":/images/info-48x48.png"), tr("context"), this)), MenuActionRole);
   item11->setData(QVariant(int(VIEW::ViewContext)), ViewModeRole);
   item1->appendRow(item11);

   MenuItem* item12 = new MenuItem(tr("dashboard"), QIcon(":/images/chart-64x64.png"));
   item12->setData(QVariant::fromValue(new QAction(QIcon(":/images/chart-64x64.png"), tr("dashboard"), this)), MenuActionRole);
   item12->setData(QVariant(int(VIEW::ViewDashBoard)), ViewModeRole);   
   item1->appendRow(item12);

   MenuItem* item13 = new MenuItem(tr("history"), QIcon(":/images/history-48x48.png"));
   item13->setData(QVariant::fromValue(new QAction(QIcon(":/images/history-48x48.png"), tr("history"), this)), MenuActionRole);
   item13->setData(QVariant(int(VIEW::ViewHistory)), ViewModeRole);   
   item1->appendRow(item13);   
   

   /* -------------------------*/
   /* populate Collection Item */
   /* -------------------------*/      
   MenuItem* item21 = new MenuItem(tr("artists"), QIcon(":/images/view-artist.png"));
   item21->setData(QVariant::fromValue(new QAction(QIcon(":/images/view-artist.png"), tr("artists"), this)), MenuActionRole);
   item21->setData(QVariant(int(VIEW::ViewArtist)), ViewModeRole);
   item2->appendRow(item21);

   MenuItem* item22 = new MenuItem(tr("albums"), QIcon(":/images/album.png"));
   item22->setData(QVariant::fromValue(new QAction(QIcon(":/images/album.png"), tr("albums"), this)), MenuActionRole);
   item22->setData(QVariant(int(VIEW::ViewAlbum)), ViewModeRole);
   item2->appendRow(item22);
   
   MenuItem* item23 = new MenuItem(tr("tracks"), QIcon(":/images/track-48x48.png"));
   item23->setData(QVariant::fromValue(new QAction(QIcon(":/images/track-48x48.png"), tr("tracks"), this)), MenuActionRole);
   item23->setData(QVariant(int(VIEW::ViewTrack)), ViewModeRole);
   item2->appendRow(item23);

   MenuItem* item24 = new MenuItem(tr("genre"), QIcon(":/images/genre.png"));
   item24->setData(QVariant::fromValue(new QAction(QIcon(":/images/genre.png"), tr("genre"), this)), MenuActionRole);
   item24->setData(QVariant(int(VIEW::ViewGenre)), ViewModeRole);
   item2->appendRow(item24);

   MenuItem* item25 = new MenuItem(tr("years"), QIcon(":/images/date-48x48.png"));
   item25->setData(QVariant::fromValue(new QAction(QIcon(":/images/date-48x48.png"), tr("years"), this)), MenuActionRole);
   item25->setData(QVariant(int(VIEW::ViewYear)), ViewModeRole);
   item2->appendRow(item25);

   MenuItem* item26 = new MenuItem(tr("favorites"), QIcon(":/images/favorites-48x48.png"));
   item26->setData(QVariant::fromValue(new QAction(QIcon(":/images/favorites-48x48.png"), tr("favorites"), this)), MenuActionRole);
   item26->setData(QVariant(int(VIEW::ViewFavorite)), ViewModeRole);
   item2->appendRow(item26);

   /* -------------------------*/
   /* populate Paylist Browser */
   /* -------------------------*/    
   MenuItem* item31 = new MenuItem(tr("playlists"), QIcon(":/images/media-playlist-48x48.png"));
   item31->setData(QVariant::fromValue(new QAction(QIcon(":/images/media-playlist-48x48.png"), tr("playlists"), this)), MenuActionRole);
   item31->setData(QVariant(int(VIEW::ViewPlaylist)), ViewModeRole);   
   item3->appendRow(item31);

   MenuItem* item32 = new MenuItem(tr("smart playlists"), QIcon(":/images/smart-playlist-48x48.png"));
   item32->setData(QVariant::fromValue(new QAction(QIcon(":/images/smart-playlist-48x48.png"), tr("smart playlists"), this)), MenuActionRole);
   item32->setData(QVariant(int(VIEW::ViewSmartPlaylist)), ViewModeRole);   
   item3->appendRow(item32);

   /* -----------------------*/
   /* populate Radio Item    */
   /* -----------------------*/    
   /* add TuneIn */
   MenuItem *item41 = new MenuItem("tunein", QIcon(":/images/tunein_48x48.png"));
   item41->setData(QVariant::fromValue(new QAction(QIcon(":/images/tunein_48x48.png"), "tunein", this)), MenuActionRole);
   item41->setData(QVariant(int(VIEW::ViewTuneIn)), ViewModeRole);       
   item4->appendRow(item41);

   /* add ShoutCast */
   MenuItem *item42 = new MenuItem("shoutcast", QIcon(":/images/shoutcast.png"));
   item42->setData(QVariant::fromValue(new QAction(QIcon(":/images/shoutcast.png"), "shoutcast", this)), MenuActionRole);
   item42->setData(QVariant(int(VIEW::ViewShoutCast)), ViewModeRole);       
   item4->appendRow(item42);

   /* add Dirble */
   MenuItem *item43 = new MenuItem("dirble", QIcon(":/images/dirble.png"));
   item43->setData(QVariant::fromValue(new QAction(QIcon(":/images/dirble.png"), "dirble", this)), MenuActionRole);
   item43->setData(QVariant(int(VIEW::ViewDirble)), ViewModeRole);       
   item4->appendRow(item43);

   /* add Favorite*/
   MenuItem *item44 = new MenuItem(tr("favorite stream"), QIcon(":/images/favorites-48x48.png"));
   item44->setData(QVariant::fromValue(new QAction(QIcon(":/images/favorites-48x48.png"), tr("favorite"), this)), MenuActionRole);
   item44->setData(QVariant(int(VIEW::ViewFavoriteRadio)), ViewModeRole);       
   item4->appendRow(item44);
   
    
   /* -----------------------*/
   /* populate Computer Item */
   /* -----------------------*/
   populateComputerItem();
}



/*******************************************************************************
    populateComputerItem
*******************************************************************************/
void MenuModel::populateComputerItem()
{
   MenuItem* item1 = new MenuItem(tr("home directory"), QIcon(":/images/folder.png"));
   item1->setData(QVariant::fromValue(new QAction(QIcon(":/images/folder.png"), tr("home directory"), this)), MenuActionRole);
   item1->setData(QVariant(int(VIEW::ViewFileSystem)), ViewModeRole);
   item1->setData(QVariant(QDir::homePath()), FileRole);
   m_computerItem->appendRow(item1);

   MenuItem* item2 = new MenuItem(tr("root directory"), QIcon(":/images/folder.png"));
   item2->setData(QVariant::fromValue(new QAction(QIcon(":/images/folder.png"), tr("root directory"), this)), MenuActionRole);
   item2->setData(QVariant(int(VIEW::ViewFileSystem)), ViewModeRole);
   item2->setData(QVariant(QDir::rootPath()), FileRole);
   m_computerItem->appendRow(item2);
/*
   //! read mtab file 
   QStringList mtabMounts;
   QFile mtab("/etc/mtab");
   mtab.open(QFile::ReadOnly);
   QTextStream stream(&mtab);
   do mtabMounts.append(stream.readLine());
   while (!stream.atEnd());
   mtab.close();

   foreach(QString item, mtabMounts)     
   {
     if( item[0] == '/'                     &&
         item.split(" ").at(2) != "iso9660" &&
         item.split(" ").at(2) != "fat") 
     {
       QString place = item.split(" ").at(1);
     
       MenuItem* placeItem = new MenuItem(place, QIcon::fromTheme("drive-harddisk"));
       placeItem->setData(QVariant::fromValue(new QAction(QIcon::fromTheme("drive-harddisk"), place, this)), MenuActionRole);
  //      placeItem->setData(QVariant::fromValue(actions->value("filerview")), GlobalActionRole);
       //placeItem->setData(QVariant(place), FilerPlaceRole);
       m_computerItem->appendRow(placeItem);
     }
   }*/
}


/*******************************************************************************
    configureMenuAction
*******************************************************************************/
void MenuModel::configureMenuAction()
{
   for (int i=0; i < this->rowCount(QModelIndex()); i++)
   {
    const QModelIndex childIdx = this->index(i, 0, QModelIndex());
    QAction *a = childIdx.data(MenuActionRole).value<QAction*>();
    a->setData(QVariant::fromValue(childIdx));
    connect(a, SIGNAL(triggered()),this, SLOT(slot_on_menu_triggered()));

    QStandardItem *childItem = itemFromIndex( childIdx );

    for (int j=0; j < childItem->rowCount(); j++) {
         const QModelIndex child2Idx = this->index(j, 0, childIdx);
         QAction *a = child2Idx.data(MenuActionRole).value<QAction*>();
         a->setData(QVariant::fromValue(child2Idx));
         connect(a, SIGNAL(triggered()),this, SLOT(slot_on_menu_triggered()));
      }
  }
}

/*******************************************************************************
    slot_on_menu_triggered
     -> Quand une action provenant du menu (action associé au item et visible
     par les menu de la menubar --> on retrouve l'index dans le modele
*******************************************************************************/
void MenuModel::slot_on_menu_triggered()
{
   QAction *action = qobject_cast<QAction *>(sender());
   if(!action) return;

   QModelIndex idx = action->data().value<QModelIndex>();

   //Debug::debug() << "-- MenuModel --> slot_on_menu_triggered idx" << idx;
   emit modelItemActivated(idx);
}

/*******************************************************************************
    slot_on_menu_view_activated
*******************************************************************************/
void MenuModel::slot_on_menu_view_activated(QModelIndex index)
{
   emit modelItemActivated(index);
}
