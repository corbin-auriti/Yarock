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

#include "dboperationdialog.h"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>

/*
********************************************************************************
*                                                                              *
*    Class DbOperationDialog                                                   *
*                                                                              *
********************************************************************************
*/
DbOperationDialog::DbOperationDialog(QWidget *parent) : DialogBase(parent, tr("Database operation"))
{
    // create Gui
    this->resize(445, 150);

    QGroupBox *groupBox = new QGroupBox(tr("Choose database operation"));
    groupBox->setFlat ( true );

    ui_check_updateScan = new QRadioButton( tr("Rescan medias files and update database") );
    ui_check_fullScan   = new QRadioButton( tr("Delete and rebuild database (*)") );

    QLabel *label =  new QLabel(tr("* all changes into collection database will be discarded !!"));

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(ui_check_updateScan);
    vbox->addWidget(ui_check_fullScan);
    vbox->addWidget(label);
    groupBox->setLayout(vbox);

    /* layout content */
    setContentWidget(groupBox);

    QObject::connect(buttonBox(), SIGNAL(accepted()), this, SLOT(on_buttonBox_accepted()));
    QObject::connect(buttonBox(), SIGNAL(rejected()), this, SLOT(on_buttonBox_rejected()));
}


bool DbOperationDialog::isFullRescan()
{
    return ui_check_fullScan->isChecked();
}



//! --------- on_buttonBox_accepted --------------------------------------------
void DbOperationDialog::on_buttonBox_accepted()
{
    this->setResult(QDialog::Accepted);
    QDialog::accept();
    this->close();
}


//! --------- on_buttonBox_rejected --------------------------------------------
void DbOperationDialog::on_buttonBox_rejected()
{
    this->setResult(QDialog::Rejected);
    QDialog::reject();
    this->close();
}

