/****************************************************************************
**
** Copyright (C) 2007~2025 Colin Willcocks.
** Copyright (C) 2005~2007 Uco Mesdag. 
** All rights reserved.
** This file is part of "GT-3 Fx FloorBoard".
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**
****************************************************************************/

#include <QtWidgets>
#include "fileDialog.h"


fileDialog::fileDialog(QString fileName, QList<QString> patchList, QByteArray fileData, QByteArray default_data, QString type)
{
    Q_UNUSED(type);
    //QObject::connect(this, SIGNAL(patchIndex(int)),
    //this->parent(), SLOT(patchIndex(int)));
    this->fileData = fileData;
    this->default_data = default_data;

    QLabel *patchLabel = new QLabel(tr("Select patch to load"));
    QLabel *nameLabel = new QLabel(fileName);
    QComboBox *patchCombo = new QComboBox;
    patchCombo->addItems(patchList);
    patchCombo->setMaxVisibleItems(200);

    QObject::connect(patchCombo, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(valueChanged(int)));

    QObject::connect(patchCombo, SIGNAL(highlighted(int)),
                     this, SLOT(highlighted(int)));

    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));


    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(patchLabel);
    horizontalLayout->addWidget(patchCombo);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(nameLabel);
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Bulk File Patch Extraction"));
};

void fileDialog::valueChanged(int value)
{
    SysxIO *sysxIO = SysxIO::Instance();
    sysxIO->patchListValue = value;
    this->close();
}; 

void fileDialog::highlighted(int value)
{
    int a = 0;
    if (value>1)
    {
        a = (value-1)*650;                    // offset is calculated.
    };
    QByteArray temp;
    temp = fileData.mid(a, 650);
    default_data.replace(0, 650, temp);      //copy patch data from bulk file
    
    if (value>0)
    {
        SysxIO *sysxIO = SysxIO::Instance();
        QString area = "Structure";
        sysxIO->setFileSource(area, default_data);
        sysxIO->setFileName("file audition");
        //this->fileSource = sysxIO->getFileSource();
        sysxIO->writeToBuffer();
        // QApplication::beep();
    };
    // };
};


