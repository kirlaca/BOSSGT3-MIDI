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
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <QMessageBox>
#include "bulkLoadDialog.h"
#include "Preferences.h"
#include "globalVariables.h"

bulkLoadDialog::bulkLoadDialog()
{
    failed = true;
    QLabel *startListLabel = new QLabel(tr("Starting from"));
    this->startPatchCombo = new QComboBox(this);
    QLabel *finishListLabel = new QLabel(tr("Finishing at"));
    this->finishPatchCombo = new QComboBox(this);
    QObject::connect(startPatchCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(comboValueChanged(int)));
    QObject::connect(finishPatchCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(comboValueChanged(int)));
    QVBoxLayout *comboBoxLayout = new QVBoxLayout;
    comboBoxLayout->addWidget(startListLabel);
    comboBoxLayout->addWidget(startPatchCombo);
    comboBoxLayout->addStretch(1);
    comboBoxLayout->addSpacing(12);
    comboBoxLayout->addWidget(finishListLabel);
    comboBoxLayout->addWidget(finishPatchCombo);

    QGroupBox *patchListGroup = new QGroupBox(tr("Set the Range of Patch data to restore"));
    patchListGroup->setLayout(comboBoxLayout);

    QLabel *startRangeLabel = new QLabel(tr("Start Bank."));
    QLabel *finishRangeLabel = new QLabel(tr("Finish Bank."));

    this->startRangeComboBox = new QComboBox(this);
    QObject::connect(startRangeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboValueChanged(int)));
    this->finishRange = new QLineEdit(this);
    this->finishRange->setReadOnly(true);

    QVBoxLayout *rangeLabelLayout = new QVBoxLayout;
    rangeLabelLayout->addSpacing(12);
    rangeLabelLayout->addWidget(startRangeLabel);
    rangeLabelLayout->addSpacing(12);
    rangeLabelLayout->addWidget(finishRangeLabel);

    QHBoxLayout *spinBoxLayout = new QHBoxLayout;
    spinBoxLayout->addWidget(startRangeComboBox);
    
    QVBoxLayout *rangeBoxLayout = new QVBoxLayout;
    rangeBoxLayout->addSpacing(12);
    rangeBoxLayout->addLayout(spinBoxLayout);
    rangeBoxLayout->addSpacing(12);
    rangeBoxLayout->addWidget(finishRange);

    QHBoxLayout *dataRangeLayout = new QHBoxLayout;
    dataRangeLayout->addSpacing(20);
    dataRangeLayout->addLayout(rangeLabelLayout);
    dataRangeLayout->addLayout(rangeBoxLayout);
    dataRangeLayout->addSpacing(20);

    QGroupBox *patchRangeGroup = new QGroupBox(tr("Set start location of Patch data to restore"));
    patchRangeGroup->setLayout(dataRangeLayout);

    this->startButton = new QPushButton(this);
    this->startButton->setText(tr("Start"));
    connect(startButton, SIGNAL(clicked()), this, SLOT(sendData()));

    this->completedButton = new QPushButton(this);
    this->completedButton->setText(tr("DATA TRANSFER COMPLETED"));
    this->completedButton->hide();
    connect(completedButton, SIGNAL(clicked()), this, SLOT(close()));

    this->cancelButton = new QPushButton(this);
    this->cancelButton->setText(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));

    this->progressLabel = new QLabel(this);
    this->progressLabel->setText(tr("Full Restoration may take up to 4 minutes"));
    this->bytesLabel = new QLabel(this);
    this->bytesLabel->setText("");

    this->progressBar = new QProgressBar(this);
    this->progressBar->setTextVisible(false);
    this->progressBar->setRange(0, 100);
    this->progressBar->setValue(0);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addSpacing(40);
    buttonsLayout->addWidget(startButton);
    buttonsLayout->addStretch(1);
    buttonsLayout->addSpacing(20);
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addSpacing(40);

    QHBoxLayout *progressBarLayout = new QHBoxLayout;
    progressBarLayout->addWidget(progressBar);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(patchListGroup, Qt::AlignCenter);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout, Qt::AlignCenter);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(patchRangeGroup, Qt::AlignCenter);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(progressLabel, Qt::AlignCenter);

    mainLayout->addWidget(bytesLabel, Qt::AlignCenter);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(completedButton);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(progressBarLayout, Qt::AlignCenter);
    setLayout(mainLayout);

    setWindowTitle(tr("Bulk File Restoration"));
    bulk.clear();

    SysxIO *sysxIO = SysxIO::Instance();
    QObject::connect(this, SIGNAL(setStatusSymbol(int)), sysxIO, SIGNAL(setStatusSymbol(int)));
    QObject::connect(this, SIGNAL(setStatusProgress(int)), sysxIO, SIGNAL(setStatusProgress(int)));
    QObject::connect(this, SIGNAL(setStatusMessage(QString)), sysxIO, SIGNAL(setStatusMessage(QString)));

    Preferences *preferences = Preferences::Instance();
    QString dir = preferences->getPreferences("General", "Files", "dir");

    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Choose a file"),
        dir,
        tr("GT-3 Bulk Data File (*.syx)"));
    if (!fileName.isEmpty())
    {

        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly))
        {
            this->data = file.readAll();     // read the pre-selected file, copy to 'data'
            QFile file(":default.syx");           // Read the default GT-3 sysx file .
            if (file.open(QIODevice::ReadOnly))
            {	default_data = file.readAll(); };

            QByteArray default_header = default_data.mid(0, sysxAddressOffset);     // copy sysx header from default.syx
            QByteArray file_header = data.mid(0, sysxAddressOffset);                // copy sysx header from read file.syx

            char r = (char)data[sysxAddressOffset];     // find patch number in file (msb))
            bool ok;
            int patchNum;
            patchNum = QString::number(r, 16).toUpper().toInt(&ok, 16);
            bool isPatch = false;
            if (patchNum >= bankUserStart) { isPatch = true; };    // check the sysx file is a valid patch & not system data- patchNum matches patch address.

            bool isHeader = false;
            if (file_header == default_header) {isHeader = true; };
            if (isHeader == true && isPatch == true)
            {
                failed = false;
                loadSYX();
            }
            else
            {
                failed = true;
                QMessageBox *msgBox = new QMessageBox();
                msgBox->setWindowTitle(QObject::tr("File Restore Error"));
                msgBox->setIcon(QMessageBox::Warning);
                msgBox->setTextFormat(Qt::RichText);
                QString msgText;
                msgText.append("<font size='+1'><b>");
                msgText.append(QObject::tr("This is not a known ") + deviceType + QObject::tr(" file!"));
                msgText.append("<b></font><br>");
                msgText.append(QObject::tr("Patch file not within allowable parameters or<br>"));
                msgText.append(QObject::tr("file format unknown, please try another file."));
                msgBox->setText(msgText);
                msgBox->setStandardButtons(QMessageBox::Ok);
                msgBox->exec();

            };

        };
    };
    if (failed == true)
    {
        this->startButton->hide();
    };
};

void bulkLoadDialog::comboValueChanged(int value)
{
    Q_UNUSED(value);
    this->bankStart = this->startRangeComboBox->currentIndex();
    this->startList = this->startPatchCombo->currentIndex();
    this->finishList = this->finishPatchCombo->currentIndex();
    if (startList > finishList) {this->startPatchCombo->setCurrentIndex(finishList); }
    else if (finishList < startList) {this->finishPatchCombo->setCurrentIndex(startList); };
    int x = (bankStart+(finishList-startList));
    if (x<0) {x=0; } else if (x>((bankTotalUser*patchPerBank)-1))
    {
        x=((bankTotalUser*patchPerBank)-1);
        bankStart=((bankTotalUser*patchPerBank)-1)-(finishList-startList);
        startRangeComboBox->setCurrentIndex(((bankTotalUser*patchPerBank)-1)-(finishList-startList));
    };
    QString text = tr("Finish at U");
    int y = x/patchPerBank; y = y*patchPerBank; y=x-y;
    text.append(QString::number((x/patchPerBank)+1, 10).toUpper() );
    text.append("-");
    text.append(QString::number(y+1, 10).toUpper() );
    this->finishRange->setText(text);
    this->startRangeComboBox->setMaxVisibleItems((bankTotalUser*patchPerBank)-(finishList-startList));
}; 

void bulkLoadDialog::sendData() 
{	
    bankStart = this->startRangeComboBox->currentIndex()+1;
    startList = this->startPatchCombo->currentIndex();
    finishList = this->finishPatchCombo->currentIndex();
    startButton->hide();
    cancelButton->hide();
    progress = 0;
    patch = 1;
    range = (1000)/(((finishList-startList)+1)*10);

    int z = (this->bankStart);
    QString q;
    QString address;
    QString msg;
    QString v;
    QString addrMSB = "07";
    QString replyMsg;
    for (int a=startList;a<(finishList+1);a++)
    {
        if (z>128) {z=z-128; addrMSB = "08"; };          // next address range when > 07 7F.
        address = QString::number(z-1, 16).toUpper();
        if (address.size()<2){ address.prepend("0"); };
        int b = a*patchSize;                                // multiples of patch size.
        msg = this->bulk.mid(b*2, patchSize*2);             // copy next patch from bulk patch list.
        for (int g=0;g<msg.size()/2;g++)
        {
            v = msg.mid(g*2, 2);
            if (v == "F0") {msg.replace((g*2)+(sysxAddressOffset*2), 2, addrMSB); msg.replace((g*2)+((sysxAddressOffset*2)+2), 2, address); };   // replace the message address
        };
        replyMsg.append(msg);
        ++z;
    };

    QString reBuild = "";       // Add correct checksum to patch strings
    QString sysxEOF = "";
    QString hex = "";
    int msgLength = replyMsg.length()/2;
    for(int i=0;i<msgLength*2;++i)
    {
        hex.append(replyMsg.mid(i*2, 2));
        sysxEOF = (replyMsg.mid((i*2)+4, 2));
        if (sysxEOF == "F7")
        {
            int dataSize = 0; bool ok;
            for(int h=checksumOffset;h<hex.size()-1;++h)
            { dataSize += hex.mid(h*2, 2).toInt(&ok, 16); };
            QString base = "80";                       // checksum calculate.
            int sum = dataSize % base.toInt(&ok, 16);
            if(sum!=0) { sum = base.toInt(&ok, 16) - sum; };
            QString checksum = QString::number(sum, 16).toUpper();
            if(checksum.length()<2) {checksum.prepend("0");};
            hex.append(checksum);
            hex.append("F7");
            reBuild.append(hex);
            hex = "";
            sysxEOF = "";
            i=i+2;
        };
    };
    this->bulk = reBuild.simplified().toUpper().remove("0X").remove(" ");
    bank = (bankStart+(patchPerBank-1));
    patch = bank/patchPerBank; patch = patch*patchPerBank; patch=bank-patch;
    steps=0;
    dataSent=0;
    sendSequence("");
};	

void bulkLoadDialog::sendPatch(QString data)
{
    SysxIO *sysxIO = SysxIO::Instance();
    QObject::connect(sysxIO, SIGNAL(sysxReply(QString)), this, SLOT(sendSequence(QString)));
    sysxIO->sendSysx(data);
};                                                           

void bulkLoadDialog::sendSequence(QString value)
{
    Q_UNUSED(value);
    SysxIO *sysxIO = SysxIO::Instance();
    QObject::disconnect(sysxIO, SIGNAL(sysxReply(QString)), this, SLOT(sendSequence(QString)));
    sysxIO->setDeviceReady(true); // Free the device after finishing interaction.
    msg=bulk.mid(steps*(patchSize*2), (patchSize*2));

    progress=progress+range;
    bulkStatusProgress(this->progress);                         // advance the progressbar.

    if (steps<((finishList-startList)+2) )
    {
        bool ok;
        QString patchText;
        QString name = msg.mid((sysxNameOffset*2), (nameLength*2));  // get name from loaded patch. 
        QList<QString> x;        
        for (int b=0;b<nameLength;b++)
        {
            x.append(name.mid(b*2, 2));
        };
        for (int b=0;b<nameLength;b++)
        {
            QString hexStr = x.at(b);
            patchText.append( (char)(hexStr.toInt(&ok, 16)) );      // convert name to readable text characters.
        };
        int bf = (finishList-startList);
        if(steps>bf) {this->completedButton->show();        
            this->progressLabel->setText(tr("Bulk data transfer completed!!"));
            this->progress=100;  };
        
        QString patchNumber = QString::number(bank/4, 10).toUpper();
        patchNumber.prepend(tr("User Patch U" ));
        patchNumber.append("-");
        patchNumber.append( QString::number(patch, 10).toUpper() );
        patchNumber.append("     ");
        patchNumber.append(patchText);
        patchText=patchNumber;
        this->progressLabel->setText(patchText);                        //display the patch number and name.

        patchNumber = tr("File transfer size = ");
        dataSent = dataSent+(msg.size()/2);
        patchNumber.append(QString::number(dataSent, 10).toUpper() );
        patchNumber.append(tr(" bytes"));
        this->bytesLabel->setText(patchNumber);                         //display the bulk data size.
        ++steps;
        ++patch;
        if(patch>patchPerBank) {patch=1; bank=bank+patchPerBank;};	       // increment patch.
        sendPatch(msg);                                 //request the next patch.
        setStatusMessage(tr("Sending Data"));
    } else {
        QObject::disconnect(sysxIO, SIGNAL(sysxReply(QString)), this, SLOT(sendSequence(QString)));
        sysxIO->setDeviceReady(true); // Free the device after finishing interaction.
        setStatusMessage(tr("Ready"));
        close();
    };
};


void bulkLoadDialog::updatePatch()
{
    patchCount = sysxPatches.size()/(patchSize);
    QString msgText;
    QString patchText;
    QString patchNumber;
    char r;
    this->patchList.clear();
    int a = sysxNameOffset;             // locate patch text start position from the start of the file
    for (int h=0;h<patchCount;h++)
    {
        for (int b=0;b<nameLength;b++)
        {
            r = (char)sysxPatches[a+b];
            patchText.append(r);
        };
        patchNumber = QString::number(h+1, 10).toUpper();
        msgText.append(patchNumber + " : ");
        msgText.append(patchText + "   ");
        this->patchList.append(msgText);
        patchText.clear();
        msgText.clear();
        a=a+patchSize;                                // advance to the next patch in the bulk file.
    };
    this->startPatchCombo->addItems(patchList);            // add patch names to the combobox lists.
    this->finishPatchCombo->addItems(patchList);
    this->finishPatchCombo->setCurrentIndex(patchCount-1);     // set the finish combobox index to the end of the list.
    this->startPatchCombo->setCurrentIndex(0);
    QString text = tr("Finish at U");
    if (patchCount<4) {patchCount=4; };
    text.append(QString::number(patchCount/4, 10).toUpper() );
    text.append("-x");
    this->finishRange->setText(text);

    QString U = "U";
    for (int x=0; x<bankTotalUser; x++)
    {
        QString bnk = "U" + QString::number(x+1, 10).toUpper() + "-1";
        this->startRangeComboBox->addItem(bnk);
        bnk = "U" + QString::number(x+1, 10).toUpper() + "-2";
        this->startRangeComboBox->addItem(bnk);
        bnk = "U" + QString::number(x+1, 10).toUpper() + "-3";
        this->startRangeComboBox->addItem(bnk);
        bnk = "U" + QString::number(x+1, 10).toUpper() + "-4";
        this->startRangeComboBox->addItem(bnk);
    };
    this->startRangeComboBox->setCurrentIndex(0);

    QString sysxBuffer;
    for(int i=0;i<sysxPatches.size();i++)
    {
        char byte = (char)sysxPatches[i];
        int n = (int)byte;
        QString hex = QString::number(n, 16).toUpper();     // convert QByteArray to QString
        if (hex.length() < 2) hex.prepend("0");
        sysxBuffer.append(hex);
    };
    bulk.append(sysxBuffer);                                // add patch to the bulk string.
};

void bulkLoadDialog::bulkStatusProgress(int value)
{
    if (value >100) {value = 100;};
    if (value<0) {value = 0; };
    this->progressBar->setValue(value);
};

void bulkLoadDialog::loadGTE()         // ************************************ GTE File Format***************************
{	
 
};

void bulkLoadDialog::loadSYX()        //********************************* SYX File Format *****************************
{	 close();  
    sysxPatches = data;
    updatePatch();  
};

void bulkLoadDialog::loadSMF()    // **************************** SMF FILE FORMAT ***************************
{	
 	
};

