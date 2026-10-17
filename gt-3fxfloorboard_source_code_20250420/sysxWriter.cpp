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

#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <QMessageBox>
#include "sysxWriter.h"
#include "fileDialog.h"	
#include "globalVariables.h"

sysxWriter::sysxWriter() 
{

};

sysxWriter::~sysxWriter()
{

};

void sysxWriter::setFile(QString fileName)
{
    this->fileName = fileName;
    this->fileSource.address.clear();
    this->fileSource.hex.clear();
};

bool sysxWriter::readFile()
{	
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.readAll();
        QByteArray default_data;
        QFile file(":default.syx");           // Read the default GT-6 sysx file .
        if (file.open(QIODevice::ReadOnly))
        {	default_data = file.readAll(); };

        QByteArray default_header = default_data.mid(0, 6);           // copy header from default.syx
        QByteArray text_header = default_data.mid(650, 6);            // copy header from default.syx from text area
        QByteArray file_header = data.mid(0, 6);                      // copy header from read file.syx
        bool isHeader = false;
        //bool isOther = false;
        bool isSystem = false;
        if (default_header == file_header) { isHeader = true; };
        //if (text_header == file_header) { isOther = true; };
        char byte = (char)data[6];
        int n = (int)byte;
        if (n < 6){ isSystem = true; };

        if(data.size() == 898 && isHeader && !isSystem){
            SysxIO *sysxIO = SysxIO::Instance();
            QString area = "Structure";
            sysxIO->setFileSource(area, data);
            sysxIO->setFileName(this->fileName);

            this->fileSource = sysxIO->getFileSource();
            return true;
        }
        else if (data.size() == 650 && isHeader && !isSystem)
        {
            QByteArray standard_data = data;
            QFile file(":default.syx");   // Read the default GT-3 sysx file so we don't start empty handed.
            if (file.open(QIODevice::ReadOnly))
            {	data = file.readAll(); };

            QByteArray temp;
            temp = data.mid(650, 228);           // copy patch description from default.syx  address 00 17 00 00

            standard_data.append(temp);
            data = standard_data;

            SysxIO *sysxIO = SysxIO::Instance();
            QString area = "Structure";
            sysxIO->setFileSource(area, data);
            sysxIO->setFileName(this->fileName);

            this->fileSource = sysxIO->getFileSource();
            return true;
        }
        else if (data.size() > patchSize && isHeader && !isSystem)   // bulk patch file
        {
            /* QByteArray patch_data = data;
	  QFile file(":default.syx");   // Read the default GT-3 sysx file so we don't start empty handed.
    if (file.open(QIODevice::ReadOnly))
	  {	data = file.readAll(); };

      QByteArray test = default_data.mid(patchSize, 6);     // check for GT-Manager data in the file.
      test.remove(2, 1);
     if (data.contains(test))
     {
       QMessageBox *msgBox = new QMessageBox();
	msgBox->setWindowTitle(QObject::tr("File compatibility Error!"));
	msgBox->setIcon(QMessageBox::Warning);
	msgBox->setTextFormat(Qt::RichText);
	QString msgText;
	msgText.append("<font size='+1'><b>");
	msgText.append(QObject::tr("This is not recognised as a supported ") + deviceType + QObject::tr(" patch file"));
	msgText.append("<b></font><br>");
	msgText.append(QObject::tr("but appears to be a GT-Manager proprietry format<br>"));
	msgText.append(QObject::tr("please try another file."));
	msgBox->setText(msgText);
	msgBox->setStandardButtons(QMessageBox::Ok);
	msgBox->exec();
	return false;
     };       */
            index = 1;
            int patchCount = data.size()/patchSize;       // calculate how many patches are in the file.
            if (patchCount>1)
            {
                QString msgText;
                QString patchText;
                QString patchNumber;
                char r;
                this->patchList.clear();
                QString text = QObject::tr("Select Patch");
                this->patchList.append(text);
                int a = sysxNameOffset; // locate patch text start position from the start of the file
                for (int h=0;h<patchCount;h++)
                {
                    for (int b=0;b<nameLength;b++)
                    {
                        r = (char)data[a+b];
                        patchText.append(r);         // extract the text characters from the current patch name.
                    };
                    patchNumber = QString::number(h+1, 10).toUpper();
                    msgText.append(patchNumber + " : ");
                    msgText.append(patchText + "   ");
                    this->patchList.append(msgText);
                    patchText.clear();
                    msgText.clear();
                    a=a+patchSize;                      // offset is set in front of marker
                };
                QString type = "syx";
                fileDialog *dialog = new fileDialog(fileName, patchList, data, default_data, type);
                dialog->exec();
                patchIndex(this->index);
            };
            int a=0;
            if (patchCount>1)
            {
                int q=index-1;      // find start of required patch
                a = q*patchSize;
            };
            data = data.mid(a, patchSize);
            data.append(default_data.mid(patchSize, 228));
            if (index>0)
            {
                SysxIO *sysxIO = SysxIO::Instance();
                QString area = "Structure";
                sysxIO->setFileSource(area, data);
                sysxIO->setFileName(this->fileName);

                this->fileSource = sysxIO->getFileSource();
                return true;
            } else {return false; }
        }

        else if (data.size() > 650 && data.size() < 4206 && isSystem)   // most likely an other type file with system data at front
        {
            QByteArray patch_data = data;
            QFile file(":default.syx");   // Read the default GT-6 sysx file so we don't start empty handed.
            if (file.open(QIODevice::ReadOnly))
            {	data = file.readAll(); };
            int x=1;
            QByteArray temp;
            for(int b=0; b<2000; b++)
            {
                x = patch_data.indexOf(default_header, b);
                byte = (char)patch_data[x+6];
                n = (int)byte;
                if (n < 6){b=b+x;}else {b=2000;};

                temp = patch_data.mid(4206, 650);
                temp.append(data.mid(650, 228));           // copy patch description from default.syx  address 00 17 00 00
            };
            data = temp;

            SysxIO *sysxIO = SysxIO::Instance();
            QString area = "Structure";
            sysxIO->setFileSource(area, data);
            sysxIO->setFileName(this->fileName);

            this->fileSource = sysxIO->getFileSource();
            return true;
        }
        else if (data.size() == 5258 && isSystem)   // SYSytem File
        {
            SysxIO *sysxIO = SysxIO::Instance();
            QString area = "System";
            sysxIO->setFileSource(area, data);

            this->systemSource = sysxIO->getSystemSource();
            return true;
        }
        else
        {
            QMessageBox *msgBox = new QMessageBox();
            msgBox->setWindowTitle(QObject::tr("Patch size Error!"));
            msgBox->setIcon(QMessageBox::Warning);
            msgBox->setTextFormat(Qt::RichText);
            QString msgText;
            msgText.append("<font size='+1'><b>");
            msgText.append(QObject::tr("This is not a known ") + deviceType + QObject::tr(" patch!"));
            msgText.append("<b></font><br>");
            msgText.append(QObject::tr("Patch file not within allowable parameters or<br>"));
            msgText.append(QObject::tr("file format unknown, please try another file."));
            msgBox->setText(msgText);
            msgBox->setStandardButtons(QMessageBox::Ok);
            msgBox->exec();
            return false;};
    }
    else
    {
        return false;
    };
};

void sysxWriter::patchIndex(int listIndex)
{
    Q_UNUSED(listIndex);
    SysxIO *sysxIO = SysxIO::Instance();
    this->index=sysxIO->patchListValue;
};

void sysxWriter::writeFile(QString fileName)
{	
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        SysxIO *sysxIO = SysxIO::Instance();
        this->fileSource = sysxIO->getFileSource();

        QByteArray out;
        for (QList< QList<QString> >::iterator dev = fileSource.hex.begin(); dev != fileSource.hex.end(); ++dev)
        {
            QList<QString> data(*dev);
            for (QList<QString>::iterator code = data.begin(); code != data.end(); ++code)
            {
                QString str(*code);
                bool ok;
                int n = str.toInt(&ok, 16);
                out.append((char)n);
            };
        };
        file.write(out);
    };

};

void sysxWriter::writeSystemFile(QString fileName)
{	
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        SysxIO *sysxIO = SysxIO::Instance();
        this->systemSource = sysxIO->getSystemSource();

        QByteArray out;
        for (QList< QList<QString> >::iterator dev = systemSource.hex.begin(); dev != systemSource.hex.end(); ++dev)
        {
            QList<QString> data(*dev);
            for (QList<QString>::iterator code = data.begin(); code != data.end(); ++code)
            {
                QString str(*code);
                bool ok;
                int n = str.toInt(&ok, 16);
                out.append((char)n);
            };
        };
        file.write(out);
    };

};

SysxData sysxWriter::getFileSource()
{
    return fileSource;
};

SysxData sysxWriter::getSystemSource()
{
    return systemSource;
};

QString sysxWriter::getFileName()
{
    return fileName;
};

