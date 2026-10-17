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
#include "bulkSaveDialog.h"
#include "Preferences.h"
#include "globalVariables.h"


// Platform-dependent sleep routines.
#ifdef Q_OS_WIN
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants & Mac
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

bulkSaveDialog::bulkSaveDialog()
{ 
  QLabel *startRangeLabel = new QLabel(tr("Start Bank."));
	QLabel *finishRangeLabel = new QLabel(tr("Finish Bank."));
	
	QCheckBox *systemCheckBox = new QCheckBox(tr("Save System Data"));
	QSpinBox *startRangeSpinBox = new QSpinBox;
	QSpinBox *finishRangeSpinBox = new QSpinBox;

	this->systemCheckBox = systemCheckBox;
		
	this->startRangeSpinBox = startRangeSpinBox;
	startRangeSpinBox->setValue(1);
	startRangeSpinBox->setRange(1, bankTotalUser);
	startRangeSpinBox->setPrefix("Start at U");
	startRangeSpinBox->setSuffix("-1");

	this->finishRangeSpinBox = finishRangeSpinBox;
	finishRangeSpinBox->setValue(bankTotalUser);    
	finishRangeSpinBox->setRange(1, bankTotalUser);
	finishRangeSpinBox->setPrefix(tr("Finish at U"));
	finishRangeSpinBox->setSuffix("-4");

	QVBoxLayout *rangeLabelLayout = new QVBoxLayout;
	rangeLabelLayout->addSpacing(12);
	rangeLabelLayout->addWidget(startRangeLabel);
	rangeLabelLayout->addWidget(finishRangeLabel);

	QVBoxLayout *rangeBoxLayout = new QVBoxLayout;
	//rangeBoxLayout->addWidget(systemCheckBox);
	rangeBoxLayout->addSpacing(12);
	rangeBoxLayout->addWidget(startRangeSpinBox);
	rangeBoxLayout->addSpacing(12);
	rangeBoxLayout->addWidget(finishRangeSpinBox);

	QHBoxLayout *dataRangeLayout = new QHBoxLayout;
	dataRangeLayout->addSpacing(20);
	dataRangeLayout->addLayout(rangeLabelLayout);
	//dataRangeLayout->addSpacing(12);
	dataRangeLayout->addLayout(rangeBoxLayout);
	dataRangeLayout->addSpacing(20);

	QGroupBox *patchRangeGroup = new QGroupBox(tr("Set range of Patch data to save"));
  patchRangeGroup->setLayout(dataRangeLayout);

  this->startButton = new QPushButton(this);
  this->startButton->setText(tr("Start"));
  connect(startButton, SIGNAL(clicked()), this, SLOT(backup()));
  
  this->completedButton = new QPushButton(this);
  this->completedButton->setText(tr("DATA TRANSFER COMPLETED"));
  this->completedButton->hide();
  connect(completedButton, SIGNAL(clicked()), this, SLOT(close()));
  
	this->cancelButton = new QPushButton(this);
	this->cancelButton->setText(tr("Cancel"));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
  
  this->progressLabel = new QLabel(this);
  this->progressLabel->setText(tr("Full Backup may take up to 2 minutes"));
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

	setWindowTitle(tr("Bulk File Backup"));
	
	//QObject::connect(this, SIGNAL( startRangeSpinBox->valueChanged(int) ), this, SLOT( bankStart(int) ));
	//QObject::connect(this, SIGNAL( finishRangeSpinBox->valueChanged(int) ), this, SLOT( bankFinish(int) ));
	SysxIO *sysxIO = SysxIO::Instance();
	QObject::connect(this, SIGNAL(setStatusSymbol(int)), sysxIO, SIGNAL(setStatusSymbol(int)));
	QObject::connect(this, SIGNAL(setStatusProgress(int)), sysxIO, SIGNAL(setStatusProgress(int)));
	QObject::connect(this, SIGNAL(setStatusMessage(QString)), sysxIO, SIGNAL(setStatusMessage(QString)));	
  
}

bulkSaveDialog::~bulkSaveDialog()
{
}

void bulkSaveDialog::backup()
{
  this->startButton->hide();
  //this->cancelButton->hide();
  this->bankStart = this->startRangeSpinBox->value();
  this->bankFinish = this->finishRangeSpinBox->value();
  this->bank=bankStart*patchPerBank;
	bulk.clear();
	this->progress = 0;
	this->patch = 1;
	range = (bankTotalUser*patchPerBank)/(bankFinish-bankStart+1);
	requestPatch(bank/patchPerBank, patch);  
}

void bulkSaveDialog::requestPatch(int bank, int patch) 
{	
SysxIO *sysxIO = SysxIO::Instance();
	if(sysxIO->isConnected() && sysxIO->deviceReady() )
	{
	  SysxIO *sysxIO = SysxIO::Instance();
		QObject::connect(sysxIO, SIGNAL(sysxReply(QString)), this, SLOT(updatePatch(QString)));					
		sysxIO->requestPatch(bank, patch);    		
	};
}

void bulkSaveDialog::updatePatch(QString replyMsg)
{
	SysxIO *sysxIO = SysxIO::Instance(); 
	QObject::disconnect(sysxIO, SIGNAL(sysxReply(QString)), this, SLOT(updatePatch(QString)));		
	sysxIO->setDeviceReady(true); // Free the device after finishing interaction.
	
	replyMsg = replyMsg.simplified().toUpper();      
	if (replyMsg.size()/2 == patchSize){              // size of patch sent from gt
	                               bulk.append(replyMsg); 	    // add patch to the bulk string.
	                             }; 	       
	      ++patch; 
        if(patch>patchPerBank) {patch=1; bank=bank+patchPerBank;};	                      // increment patch.
        progress=progress+range;
        bulkStatusProgress(this->progress);                         // advance the progressbar.
        int bf = (bankFinish+1)*4 -2;
  if (bank >= bf) 
      {                                                            // check if nearly finished.
        this->completedButton->show();        
        this->progressLabel->setText(tr("Bulk data transfer completed!!"));              
      };              
  if (bank<(bankFinish+1)*patchPerBank )
  {      
        bool ok;
        QString patchText;
        QString name = replyMsg.mid(sysxNameOffset*2, nameLength*2);         // get name from loaded patch. 
        QList<QString> x;        
        for (int b=0;b<(nameLength*2);b++)
        {
          x.append(name.mid(b*2, 2));       
        };
        for (int b=0;b<(nameLength*2);b++)
        {
        	QString hexStr = x.at(b);			
		      patchText.append( (char)(hexStr.toInt(&ok, 16)) );      // convert name to readable text characters.
        };
        
  QString patchNumber = QString::number(bank/patchPerBank, 10).toUpper();
  if (patchNumber.size()<2) { patchNumber.prepend("0"); };
  patchNumber.prepend(tr("User Patch U"));
  patchNumber.append("-");
  patchNumber.append( QString::number(patch, 10).toUpper() );
  patchNumber.append("     ");
  patchNumber.append(patchText);
  patchText=patchNumber;   
  this->progressLabel->setText(patchText);                        //display the patch number and name.
  
  patchNumber = tr("File build size = ");
  int size = (bulk.size()/2)+patchSize;
  patchNumber.append(QString::number(size, 10).toUpper() );
  patchNumber.append(tr(" bytes"));
  this->bytesLabel->setText(patchNumber);                         //display the bulk data size.
  
  setStatusMessage(tr("Bulk Download"));  
  requestPatch(bank/patchPerBank, patch);                                   //request the next patch.
  } else {
          setStatusMessage(tr("Ready"));
          sysxIO->bulk = this->bulk;
          writeSYX();
         };  
}

void bulkSaveDialog::bulkStatusProgress(int value)
{
   value=value/8;
  if (value >100) {value = 100;};
  if (value<0) {value = 0; };
	this->progressBar->setValue(value);
}

void bulkSaveDialog::writeSYX()        //********************************* SYX File Format *****************************
{	
  	Preferences *preferences = Preferences::Instance();
	          QString dir = preferences->getPreferences("General", "Files", "dir");

          	QString fileName = QFileDialog::getSaveFileName(
                    this,
                    tr("Save Bulk Data"),
                    dir,
                    tr("System Exclusive Backup File (*.syx)"));
	             if (!fileName.isEmpty())	
                	{
	                  if(!fileName.contains(".syx"))
	                     	{
		                    	fileName.append(".syx");
	                     	};		
		
	QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
	{	
     bool ok;	
		QByteArray out;
		int size = this->bulk.size();
		for (int x=0;x<size;x++)
		{
		   QString hexStr = bulk.mid(x, 2);
		   out.append( (char)(hexStr.toInt(&ok, 16)) );
		    ++x;
		};
   file.write(out); 
	};	 
 };
 close();
}

void bulkSaveDialog::cancel()
{
    this->bank=200;
    SysxIO *sysxIO = SysxIO::Instance();
    QObject::disconnect(sysxIO, SIGNAL(sysxReply(QString)), this, SLOT(updatePatch(QString)));
    sysxIO->setDeviceReady(true);
    setStatusMessage(tr("Ready"));
    this->close();
}
