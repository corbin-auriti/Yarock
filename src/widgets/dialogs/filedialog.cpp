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
#include "filedialog.h"
#include "debug.h"

#include <QFileInfo>

/**
 *   This function has been copied from Qt library
 */

// Makes a list of filters from a normal filter string "Image Files (*.png *.jpg)"
static QStringList qt_clean_filter_list(const QString &filter)
{
   const char *qt_file_dialog_filter_reg_exp =
    "([a-zA-Z0-9 -]*)\\(([a-zA-Z0-9_.*? +;#\\-\\[\\]@\\{\\}/!<>\\$%&=^~:\\|]*)\\)$";

    QRegExp regexp(QString::fromLatin1(qt_file_dialog_filter_reg_exp));
    QString f = filter;
    int i = regexp.indexIn(f);
    if (i >= 0)
        f = regexp.cap(2);

    return f.split(QLatin1Char(' '), QString::SkipEmptyParts);
}

/*
********************************************************************************
*                                                                              *
*    Class FileDialog                                                          *
*                                                                              *
********************************************************************************
*/
FileDialog::FileDialog(QWidget *parent, Mode mode, const QString& title) : DialogBase(parent,title)
{
    m_mode = mode;
    
    m_model = new QFileSystemModel(this);
    m_model->setNameFilterDisables (false);
    m_model->setReadOnly( true );
    m_model->setRootPath(QDir::homePath());
    m_rootIdx = m_model->index(QDir::rootPath());
    
    setupUi();
    
    /* logic */
    ui_listView->setModel(m_model);
    ui_listView->setRootIndex(m_model->index(QDir::homePath()));


    if (m_mode == FileDialog::AddDirs || m_mode == FileDialog::AddDir)
    {
        m_model->setFilter(QDir::AllDirs | QDir::System | QDir::Hidden); //dirs only
    }
    else
    {
        //m_model->setFilter(QDir::AllDirs | QDir::System | QDir::Files | QDir::NoDotAndDotDot);
        m_model->setFilter(QDir::AllDirs | QDir::System | QDir::Files | QDir::Hidden);
    }
 
    //set selection mode
    if (m_mode == FileDialog::AddDir ||  m_mode == FileDialog::AddFile || m_mode == FileDialog::SaveFile)
    {
        ui_listView->setSelectionMode (QAbstractItemView::SingleSelection);
    }
    else
    {
        ui_listView->setSelectionMode (QAbstractItemView::ExtendedSelection);
    }
    
    /* connect */
    if(ui_filter_cb)
      connect(ui_filter_cb, SIGNAL(currentIndexChanged(int)), SLOT(slot_onfilter_changed(int)));
    
    connect(buttonBox(), SIGNAL(accepted()), this, SLOT(on_buttonBox_accepted()));
    connect(buttonBox(), SIGNAL(rejected()), this, SLOT(on_buttonBox_rejected()));
    connect(ui_prev_button, SIGNAL(clicked()), this, SLOT(slot_on_prev_clicked()));
    connect(ui_next_button, SIGNAL(clicked()), this, SLOT(slot_on_next_clicked()));
    connect(ui_up_button, SIGNAL(clicked()), this, SLOT(slot_on_up_clicked()));
    connect(ui_home_button, SIGNAL(clicked()), this, SLOT(slot_on_home_clicked()));
    connect(ui_listView, SIGNAL(doubleClicked(const QModelIndex &)), SLOT(slot_listview_itemDoubleClicked(const QModelIndex &)));
    connect(ui_listView->selectionModel(),SIGNAL(selectionChanged (const QItemSelection&, const QItemSelection&)),
           SLOT(slot_listview_selectionChanged()));    

    connect(ui_filename_lineedit, SIGNAL(textChanged(const QString &)), this, SLOT(slot_on_filename_textChanged(const QString &)));
}

//! --------- on_buttonBox_accepted --------------------------------------------
void FileDialog::on_buttonBox_accepted()
{
    Debug::debug() << "FileDialog::on_buttonBox_accepted";
    // do something
    if(m_mode == SaveFile) 
    {
       QString path = m_model->filePath(ui_listView->rootIndex()) + "/" + ui_filename_lineedit->text();
       QString ext  = QFileInfo(path).suffix().toLower();
       
       if(ext.isEmpty()) {
         path = path + ".xspf";
       }
  
       m_result_file = path;
       Debug::debug() << "FileDialog save to file: " << path;
    }
  
    this->setResult(QDialog::Accepted);
    this->close();
    this->accept();    
}


//! --------- on_buttonBox_rejected --------------------------------------------
void FileDialog::on_buttonBox_rejected()
{
    this->setResult(QDialog::Rejected);
    QDialog::reject();
    this->close();
}


void FileDialog::setupUi()
{
    if(m_mode == FileDialog::SaveFile)
      buttonBox()->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);
    else
      buttonBox()->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Open);     
    
    enableButton(false);

    /* setup ui */
    ui_prev_button = new QPushButton( QIcon::fromTheme("go-previous", QIcon(":/images/go-previous.png")), tr("Back"), this );
    ui_next_button = new QPushButton( QIcon::fromTheme("go-next", QIcon(":/images/go-next.png")),tr("Forward"),this);
    ui_up_button   = new QPushButton( QIcon::fromTheme("go-up", QIcon(":/images/go-up.png")),tr("Go up"),this);
    ui_home_button = new QPushButton( QIcon(":/images/go-home.png"), tr("home"), this );

    ui_prev_button->setEnabled(false);
    ui_next_button->setEnabled(false);
    
    ui_filename_lineedit = new QLineEdit();
    
    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->addWidget(ui_home_button);
    hbox->addWidget(ui_prev_button);
    hbox->addWidget(ui_next_button);
    hbox->addWidget(ui_up_button);
    
    ui_listView = new QListView();
    ui_listView->setAlternatingRowColors(false);
    ui_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_listView->setUniformItemSizes(true); //!optim
    ui_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);    
    ui_listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    /* filter */
    ui_filter_cb = 0;
    if(m_mode == FileDialog::AddFile || FileDialog::m_mode == AddFiles || FileDialog::m_mode == SaveFile)
    {
      ui_filter_cb = new QComboBox();
      
      if(FileDialog::m_mode != SaveFile) 
      {
        ui_filter_cb->addItem(tr("Audio files (*.mp3 *.wav *.ogg *.flac *.m4a *.aac)"));
        ui_filter_cb->addItem(tr("Playlists files (*.m3u *.pls *.xspf)"));
        ui_filter_cb->addItem(tr("All files (*.*)"));
      }
      else
      {
        ui_filter_cb->addItem(tr("All playlist (*.m3u *.pls *.xspf)"));
        ui_filter_cb->addItem(tr("m3u playlist (*.m3u)"));
        ui_filter_cb->addItem(tr("pls playlist (*.pls)"));
        ui_filter_cb->addItem(tr("xspf playlist (*.xspf)"));
      }
      slot_onfilter_changed(0);
    }

    /* layout */    
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addLayout(hbox);
    vbox->addWidget(ui_listView);
    vbox->addWidget(ui_filename_lineedit);
    
    if(m_mode == AddFile || m_mode == AddFiles || m_mode == SaveFile)
      vbox->addWidget(ui_filter_cb);
    
    setContentLayout(vbox);
}

void FileDialog::setFilters(QStringList filters)
{
    if(!filters.isEmpty())
    {
     ui_filter_cb->clear();
     foreach(QString filter, filters)
       ui_filter_cb->addItem(filter);
     
      slot_onfilter_changed(0);
    }
}


QString FileDialog::addDirectory() 
{
    return m_result_file;
}

QStringList FileDialog::addDirectories()
{
    return m_result_files;  
}  


QString FileDialog::addFile()
{
    return m_result_file;  
}     

QStringList FileDialog::addFiles() 
{
    return m_result_files;  
}

QString FileDialog::saveFile() 
{
    return m_result_file;    
}  
    
   
void FileDialog::slot_onfilter_changed(int index)
{
    m_model->setNameFilters(qt_clean_filter_list(ui_filter_cb->itemText(index)));
}

void FileDialog::slot_listview_selectionChanged()
{
    //Debug::debug() << "FileDialog -> slot_listview_selectionChanged";

    QModelIndexList ml = ui_listView->selectionModel()->selectedIndexes(); 
    QStringList list;
    QString linefield;
    m_result_files.clear();
    
    /* ------------ update buttons status ------------ */
    if(ml.isEmpty()) 
    {
      enableButton(false);
    }
    else
    {
      bool only_files = true;
      bool only_dirs  = true;
      foreach (QModelIndex idx, ml) {
        if(m_model->fileInfo(idx).isDir())
          only_files = false;
        else
          only_dirs = false;
      }
    
      if(m_mode == AddFile || m_mode == AddFiles || m_mode == SaveFile)
        enableButton(only_files);
      else
        enableButton(only_dirs);
    }
    
    /* ------------update line edit field ------------*/
    foreach (QModelIndex idx, ml) {
      if(m_model->fileInfo(idx).isDir())
        list << m_model->fileInfo(idx).completeBaseName();
      else
        list << m_model->fileInfo(idx).fileName();

      m_result_files << m_model->fileInfo(idx).canonicalFilePath();
      m_result_file = m_result_files.first();
    }
    
    if (list.size() == 1)
      linefield = list.at(0);
    else {
      linefield = list.join(",");
    }
    
    ui_filename_lineedit->setText(linefield);
}

void FileDialog::enableButton(bool enable)
{
    if(m_mode == FileDialog::SaveFile)
      buttonBox()->button ( QDialogButtonBox::Save )->setEnabled(enable);
    else
      buttonBox()->button ( QDialogButtonBox::Open )->setEnabled(enable); 
}


void FileDialog::slot_listview_itemDoubleClicked(const QModelIndex &index)
{
    //Debug::debug() << "FileDialog -> slot_listview_itemDoubleClicked" << index;
    if (index.isValid())
    {
        QFileInfo info = m_model->fileInfo(index);
        if (info.isDir())
        {
            ui_listView->selectionModel()->clear ();
            updateCurrentIndex(index);
        }
        else
        {
            if (m_mode == FileDialog::AddFiles || m_mode == FileDialog::AddFile) 
            {
              m_result_file = m_model->filePath(index);
              m_result_files.clear();
              m_result_files << m_result_file;
              accept();
            }
        }
    }    
}

void FileDialog::slot_on_filename_textChanged(const QString & text)
{
    //Debug::debug() << "FileDialog -> slot_on_filename_textChanged" << text;
    if(m_mode == FileDialog::SaveFile && !text.isEmpty())
      enableButton(true);
}


void FileDialog::updateCurrentIndex(const QModelIndex &idx)
{
    //Debug::debug() << "- FileDialog -> updateCurrentIndex";

    //! nouveau déclenchement --> on supprime les next
    for(int i = 0; i < m_current_index; i++) {
      m_browserIdxs.removeAt(i);
    }
    
    ui_listView->setRootIndex(idx);
    m_model->setRootPath(m_model->filePath(idx));

    m_browserIdxs.prepend(idx);
    m_current_index = 0;
    
    if(m_mode == AddDir || m_mode == AddDirs) 
    {
      ui_filename_lineedit->setText(m_model->fileInfo(idx).completeBaseName());
    }
    else 
    {
      ui_filename_lineedit->setText(m_model->fileInfo(idx).fileName());
    }
    ui_prev_button->setEnabled(m_current_index < m_browserIdxs.size() -1);
    ui_next_button->setEnabled(m_current_index > 0);
    ui_up_button->setEnabled( idx !=  m_rootIdx);
  
    //! limite de la taille de l'historique de navigation
    if(m_browserIdxs.size() >= 20 && m_current_index != m_browserIdxs.size() -1)
      m_browserIdxs.takeLast();
}




void FileDialog::slot_on_prev_clicked() 
{
    if(m_current_index < m_browserIdxs.size() -1) 
    {
      m_current_index++;
      QModelIndex newIdx = m_browserIdxs.at(m_current_index);
      
      ui_listView->setRootIndex( newIdx );

      if(m_mode == AddDir || m_mode == AddDirs) 
      {
        ui_filename_lineedit->setText(m_model->fileInfo(newIdx).completeBaseName());
      }
      else 
      {
        ui_filename_lineedit->setText(m_model->fileInfo(newIdx).fileName());
      }
    
      ui_prev_button->setEnabled(m_current_index < m_browserIdxs.size() -1);
      ui_next_button->setEnabled(m_current_index > 0);
      ui_up_button->setEnabled( newIdx !=  m_rootIdx);
    }
}

void FileDialog::slot_on_next_clicked()
{
    if(m_current_index > 0) 
    {
      m_current_index--;
      QModelIndex newIdx = m_browserIdxs.at(m_current_index);

      ui_listView->setRootIndex( newIdx );

      if(m_mode == AddDir || m_mode == AddDirs) 
      {
        ui_filename_lineedit->setText(m_model->fileInfo(newIdx).completeBaseName());
      }
      else 
      {
        ui_filename_lineedit->setText(m_model->fileInfo(newIdx).fileName());
      }
      
      ui_prev_button->setEnabled(m_current_index < m_browserIdxs.size() -1);
      ui_next_button->setEnabled(m_current_index > 0);
      ui_up_button->setEnabled( newIdx !=  m_rootIdx);
    }
}

void FileDialog::slot_on_up_clicked()
{
    QModelIndex parent = m_model->parent(ui_listView->rootIndex()) ;
    if( parent.isValid() )
      updateCurrentIndex( parent );
}

void FileDialog::slot_on_home_clicked()
{
    QModelIndex idx = m_model->index(QDir::homePath()); 
    updateCurrentIndex(idx);  
}