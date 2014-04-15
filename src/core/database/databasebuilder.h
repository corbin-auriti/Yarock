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


#ifndef _DATABASE_BUILDER_H_
#define _DATABASE_BUILDER_H_

#include <QThread>
#include <QObject>
#include <QStringList>
#include <QSqlDatabase>
#include <QString>
#include <QHash>

/*
********************************************************************************
*                                                                              *
*    Class DataBaseBuilder                                                     *
*                                                                              *
********************************************************************************
*/
// Thread thread that :
//   - parse collection directory
//   - read track file metada (using Taglib)
//   - write sql database with track information
class DataBaseBuilder :  public QThread
{
  Q_OBJECT
  public:
    DataBaseBuilder();
    void setExit(bool b) {m_exit = b;}

  protected:
    void run();

  public slots:
    void rebuildFolder(QStringList folder);

  private:
    void readFsFiles();

    void insertTrack(const QString& filename);
    void updateTrack(const QString& filename);
    void removeTrack(const QString& filename);

    void insertPlaylist(const QString& filename);
    void updatePlaylist(const QString& filename);
    void removePlaylist(const QString& filename);

    void cleanUpDatabase();

    void storeCoverArt(const QString& coverFilePath, const QString& trackFilename);
    void recupCoverArtFromDir(const QString& coverFilePath, const QString& trackFilename);

    int insertGenre(const QString & genre);
    int insertYear(int year);
    int insertArtist(const QString & artist);
    int insertAlbum(const QString & album, int artist_id,const QString& cover,int year,int disc);


  private:
    // filename, mtime
    QHash<QString,uint>  m_db_files;
    QList<QString>       m_fs_files;
    QStringList          m_folders;

    bool                 m_exit;

    QSqlDatabase        *m_sqlDb;

  signals:
    void buildingFinished();
    void buildingProgress(int);
};

#endif // _DATABASE_BUILDER_H_
