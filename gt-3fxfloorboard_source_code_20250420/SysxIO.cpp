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

#include <QApplication>
#include <QMessageBox>
#include "SysxIO.h"
#include "SysxIODestroyer.h"
#include "midiIO.h"
#include "Preferences.h"
#include "sysxWriter.h"
#include "MidiTable.h"
#include "globalVariables.h"

// Platform-dependent sleep routines.
#ifdef Q_OS_WIN
#include <windows.h>
#define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds )
#else // Unix variants
#include <unistd.h>
#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

SysxIO::SysxIO()
{
    this->setConnected(false);
    this->setDevice(false);
    this->setDeviceReady(true);
    this->setSyncStatus(false);
    this->setNoError(true);
    this->setBank(0);
    this->setPatch(0);
    this->setLoadedBank(0);
    this->setLoadedPatch(0);
    this->changeCount = 0;
}

SysxIO* SysxIO::_instance = nullptr;          // initialize pointer
SysxIODestroyer SysxIO::_destroyer;

SysxIO* SysxIO::Instance()
{
    /* Multi-threading safe */
    if (!_instance /*_instance == 0*/)  // is it the first call?
    {
        _instance = new SysxIO; // create sole instance
        _destroyer.SetSysxIO(_instance);
    };
    return _instance; // address of sole instance
}

void SysxIO::setFileSource(QString area, SysxData fileSource)
{
    if (area.contains("System"))
    {
        this->systemSource = fileSource;
    }
    else
    {
        this->fileSource = fileSource;
    };
}

void SysxIO::setFileSource(QString area, QByteArray data)
{
    if (area.contains("System"))
    {
        this->systemSource.address.clear();
        this->systemSource.hex.clear();
    }
    else
    {
        this->fileSource.address.clear();
        this->fileSource.hex.clear();
    };

    QString errorList;
    QList<QString> sysxBuffer;
    int dataSize = 0; int offset = 0;
    for(int i=0;i<data.size();i++)
    {
        unsigned char byte = (char)data[i];
        unsigned int n = (int)byte;
        QString hex = QString::number(n, 16).toUpper();
        if (hex.length() < 2) hex.prepend("0");
        sysxBuffer.append(hex);

        unsigned char nextbyte = (char)data[i+1];
        unsigned int nextn = (int)nextbyte;
        QString nexthex = QString::number(nextn, 16).toUpper();
        if (nexthex.length() < 2) nexthex.prepend("0");
        if(offset >= checksumOffset && nexthex != "F7")
        {
            dataSize += n;
        };
        if(nexthex == "F7")
        {
            QString checksum = hex;

            if(getCheckSum(dataSize) != checksum)
            {
                QString errorString;
                errorString.append(tr("Address") + ": ");
                errorString.append(sysxBuffer.at(sysxAddressOffset) + " ");
                errorString.append(sysxBuffer.at(sysxAddressOffset + 1) + " ");
                errorString.append(sysxBuffer.at(sysxAddressOffset + 2) + " ");
                errorString.append(sysxBuffer.at(sysxAddressOffset + 3) + " - ");
                errorString.append(tr("checksum") + " (" + checksum + ") ");
                errorString.append(tr("should have been") + " (" + getCheckSum(dataSize) + ")");
                errorString.append("<br>");
                errorList.append(errorString);
            };
            //sysxBuffer = correctSysxMsg(sysxBuffer);
        };
        offset++;

        if(hex == "F7")
        {
            if (area.contains("System"))
            {
                this->systemSource.address.append(  sysxBuffer.at(sysxAddressOffset) + sysxBuffer.at(sysxAddressOffset + 1) + sysxBuffer.at(sysxAddressOffset + 2) /*+ sysxBuffer.at(sysxAddressOffset + 3)*/);
                this->systemSource.hex.append(sysxBuffer);
                /*QString address = (  sysxBuffer.at(sysxAddressOffset) + sysxBuffer.at(sysxAddressOffset + 1) + sysxBuffer.at(sysxAddressOffset + 2) + sysxBuffer.at(sysxAddressOffset + 3));
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setWindowTitle(QObject::tr("deBug"));
        msgBox->setText(address);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->exec(); */
            }
            else
            {
                this->fileSource.address.append( sysxBuffer.at(sysxAddressOffset + 2) + sysxBuffer.at(sysxAddressOffset + 3) );
                this->fileSource.hex.append(sysxBuffer);
            };
            sysxBuffer.clear();
            dataSize = 0;
            offset = 0;
        };
    };
    if(!errorList.isEmpty())
    {
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setWindowTitle(deviceType + tr(" Fx FloorBoard"));
        msgBox->setIcon(QMessageBox::Warning);
        msgBox->setTextFormat(Qt::RichText);
        QString msgText;
        msgText.append("<font size='+1'><b>");
        msgText.append(tr("The file opened contains one or more incorrect checksum."));
        msgText.append("<b></font><br>");
        msgText.append(tr("The incorrect values have been corrected where possible.<br>"
                          "If correction was impossible then some settings might have been reset to zero."));
        msgBox->setText(msgText);
        msgBox->setInformativeText(tr("Be aware of possible inconsistencies in this patch!"));
        msgBox->setDetailedText(errorList);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->exec();
    };
}

void SysxIO::setFileSource(QString area, QString data)
{
    if (area.contains("System"))
    {
        this->systemSource.address.clear();
        this->systemSource.hex.clear();
    }
    else
    {
        this->fileSource.address.clear();
        this->fileSource.hex.clear();
    };

    QList<QString> sysxBuffer;
    for(int i=0;i<data.size();i++)
    {
        QString hex = data.mid(i, 2);
        sysxBuffer.append(hex);
        i++;

        if(hex == "F7")
        {
            if (area.contains("System"))
            {
                this->systemSource.address.append(  sysxBuffer.at(sysxAddressOffset) + sysxBuffer.at(sysxAddressOffset + 1) + sysxBuffer.at(sysxAddressOffset + 2)/* + sysxBuffer.at(sysxAddressOffset + 3)*/);
                this->systemSource.hex.append(sysxBuffer);
            }
            else
            {
                this->fileSource.address.append( sysxBuffer.at(sysxAddressOffset + 2) + sysxBuffer.at(sysxAddressOffset + 3) );
                this->fileSource.hex.append(sysxBuffer);
            };
            sysxBuffer.clear();

        };
    };
}

void SysxIO::setFileSource(QString area, QString hex1, QString hex2, QString hex3, QString hex4)
{
    if (!area.contains("System")){area = "Structure"; };
    MidiTable *midiTable = MidiTable::Instance();
    bool ok;
    int index = hex3.toInt(&ok, 16) + sysxDataOffset;
    QString address;
    QList<QString> sysxList;
    if(area.contains("System"))
    {
        address = area;
        address.remove("System");
        address.append(hex1);
        address.append(hex2);
        sysxList = this->systemSource.hex.at(this->systemSource.address.indexOf(address));
    }
    else
    {
        address.append(hex1);
        address.append(hex2);
        sysxList = this->fileSource.hex.at(this->fileSource.address.indexOf(address));
    };

    sysxList.replace(index, hex4);
    int dataSize = 0;
    for(int i=sysxList.size() - 3; i>=checksumOffset;i--)
    {
        dataSize += sysxList.at(i).toInt(&ok, 16);
    };
    sysxList.replace(sysxList.size() - 2, getCheckSum(dataSize));

    if(area.contains("System"))
    {
        this->systemSource.hex.replace(this->systemSource.address.indexOf(address), sysxList);
    }
    else
    {
        this->fileSource.hex.replace(this->fileSource.address.indexOf(address), sysxList);
    };
    /*QMessageBox *msgBox = new QMessageBox();
        msgBox->setWindowTitle(QObject::tr("deBug"));
        msgBox->setText(address);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->exec();*/
    QString sysxMsg = midiTable->dataChange(area, hex1, hex2, hex3, hex4);

    if(this->deviceReady())         //realtime editing


    {
        this->setDeviceReady(false);

        emit setStatusSymbol(2);

        emit setStatusMessage(tr("Sending"));

        QObject::connect(this, SIGNAL(sysxReply(QString)), this, SLOT(resetDevice(QString)));


        this->sendSysx(sysxMsg);
    }
    else //if(this->isConnected())
    {
        this->sendSpooler.append(sysxMsg);
    };
}

void SysxIO::setFileSource(QString area, QString hex1, QString hex2, QString hex3, QString hex4, QString hex5)
{
    MidiTable *midiTable = MidiTable::Instance();
    bool ok;
    if (!area.contains("System")){area = "Structure"; };
    QString sourceHex1 = hex1;
    QString sourceHex3 = hex3;
    QString address;
    QList<QString> sysxList;
    if (area.contains("System"))
    {
        address = area;
        address.remove("System");
        address.append(hex1);
        address.append(hex2);
        //address.append(hex3);
        sysxList = this->systemSource.hex.at(this->systemSource.address.indexOf(address));
    }
    else
    {
        address.append(sourceHex1);
        address.append(hex2);
        sysxList = this->fileSource.hex.at(this->fileSource.address.indexOf(address));
    };
    int index = sourceHex3.toInt(&ok, 16) + sysxDataOffset;
    sysxList.replace(index, hex4);
    sysxList.replace(index + 1, hex5);

    int dataSize = 0;
    for(int i=sysxList.size() - 3; i>=checksumOffset;i--)
    {
        dataSize += sysxList.at(i).toInt(&ok, 16);
    };
    sysxList.replace(sysxList.size() - 2, getCheckSum(dataSize));
    if (area.contains("System"))
    {
        this->systemSource.hex.replace(this->systemSource.address.indexOf(address), sysxList);
    }
    else
    {
        this->fileSource.hex.replace(this->fileSource.address.indexOf(address), sysxList);
    };

    QString sysxMsg = midiTable->dataChange(area, hex1, hex2, hex3, hex4, hex5);

    if(/*this->isConnected() && */this->deviceReady()/* && this->getSyncStatus()*/)
    {
        this->setDeviceReady(false);

        emit setStatusSymbol(2);
        emit setStatusMessage(tr("Sending"));

        QObject::connect(this, SIGNAL(sysxReply(QString)),
                         this, SLOT(resetDevice(QString)));

        this->sendSysx(sysxMsg);
    }
    else /*if(this->isConnected())*/
    {
        this->sendSpooler.append(sysxMsg);
    };
}

void SysxIO::setFileSource(QString area, QString hex1, QString hex2, QString hex3, QList<QString> hexData)
{
    Q_UNUSED(hex3);
    QString sysxMsg;
    QList<QString> sysxList;
    QString address;
    address.append(hex1);
    address.append(hex2);

    if (area == "Structure")
    {
        sysxList = this->fileSource.hex.at(this->fileSource.address.indexOf(address));
        if(hexData.size() + sysxDataOffset + 2 == sysxList.size())
        {
            bool ok;
            for(int i=0; i<hexData.size();++i)
            {
                sysxList.replace(i + sysxDataOffset, hexData.at(i));
            };

            int dataSize = 0;
            for(int i=sysxList.size() - 3; i>=checksumOffset;i--)
            {
                dataSize += sysxList.at(i).toInt(&ok, 16);
            };
            sysxList.replace(sysxList.size() - 2, getCheckSum(dataSize));

            this->fileSource.hex.replace(this->fileSource.address.indexOf(address), sysxList);
        };
    } else {
        sysxList = this->systemSource.hex.at(this->systemSource.address.indexOf(address));
        if(hexData.size() + sysxDataOffset + 2 == sysxList.size())
        {
            bool ok;
            for(int i=0; i<hexData.size();++i)
            {
                sysxList.replace(i + sysxDataOffset, hexData.at(i));
            };

            int dataSize = 0;
            for(int i=sysxList.size() - 3; i>=checksumOffset;i--)
            {
                dataSize += sysxList.at(i).toInt(&ok, 16);
            };
            sysxList.replace(sysxList.size() - 2, getCheckSum(dataSize));

            this->systemSource.hex.replace(this->systemSource.address.indexOf(address), sysxList);
        };
    };
    for(int i=0;i<sysxList.size();++i)
    {
        sysxMsg.append(sysxList.at(i));
    };

    if(/*this->isConnected() && */this->deviceReady()/* && this->getSyncStatus()*/)  // helps with chain realtime
    {
        this->setDeviceReady(false);

        emit setStatusSymbol(2);
        emit setStatusMessage(tr("Sending"));

        QObject::connect(this, SIGNAL(sysxReply(QString)),
                         this, SLOT(resetDevice(QString)));

        this->sendSysx(sysxMsg);
    }
    else /*if(this->isConnected())*/
    {
        this->sendSpooler.append(sysxMsg);
    };
}

QList<QString> SysxIO::getSourceItems(QString area, QString hex1, QString hex2)
{
    QList<QString> items = this->getFileSource(area, hex1, hex2);
    return items;
}

int SysxIO::getSourceValue(QString area, QString hex1, QString hex2, QString hex3)
{
    MidiTable *midiTable = MidiTable::Instance();

    bool ok;
    if (!area.contains("System"))  {area = "Structure"; };
    int value;
    QList<QString> items = this->getSourceItems(area, hex1, hex2);
    if(midiTable->isData(area, hex1, hex2, hex3))
    {
        int maxRange = QString("7F").toInt(&ok, 16) + 1;
        int listindex = sysxDataOffset + QString(hex3).toInt(&ok, 16);
        int valueData1 = items.at(listindex).toInt(&ok, 16);
        int valueData2 = items.at(listindex + 1).toInt(&ok, 16);
        value = (valueData1 * maxRange) + valueData2;
    }
    else
    {
        value = items.at(sysxDataOffset + QString(hex3).toInt(&ok, 16)).toInt(&ok, 16);
    };
    return value;
}

/************************ resetDevice() ******************************
* Reset the device after sending a sysexmesage.
* And starts to processes the spooler if the device is free.
**********************************************************************/
void SysxIO::resetDevice(QString replyMsg)
{
    Q_UNUSED(replyMsg);
    if(this->sendSpooler.size() == 0)
    {
        QObject::disconnect(this, SIGNAL(sysxReply(QString)),
                            this, SLOT(resetDevice(QString)));
        this->setDeviceReady(true);	// Free the device after finishing interaction.
        emit setStatusMessage(tr("Ready"));
    }
    else
    {
        processSpooler();
    };
}

/************************ processSpooler() ******************************
* Send message that are in the spooler due to that the device was busy.
* And eliminates multiple messages where only the value changes.
**********************************************************************/
void SysxIO::processSpooler()
{
    QString sysxMsg;
    for(int i=0; i<this->sendSpooler.size(); ++i)
    {
        if(i + 1 < this->sendSpooler.size())
        {
            QString currentMsg = this->sendSpooler.at(i).left(2 * 3);
            QString nextMsg = this->sendSpooler.at(i + 1).left(2 * 3);
            if(currentMsg != nextMsg)
            {
                sysxMsg.append(this->sendSpooler.at(i));
            };
        }
        else
        {
            sysxMsg.append(this->sendSpooler.at(i));
        };
    };
    this->sendSysx(sysxMsg);
    this->sendSpooler.clear();
}

void SysxIO::setFileName(QString fileName)
{
    this->fileName = fileName;
}

QString SysxIO::getFileName()
{
    return this->fileName;
}

SysxData SysxIO::getFileSource()
{
    return this->fileSource;
}

SysxData SysxIO::getSystemSource()
{
    return this->systemSource;
}

QList<QString> SysxIO::getFileSource(QString area, QString hex1, QString hex2)
{
    QString address;
    QString temp = area;
    address.append(hex1);
    address.append(hex2);
    QList<QString> sysxMsg;
    if (area.contains("System"))
    {
        address.prepend(temp.remove("System"));
        if(this->systemSource.address.indexOf(address) == -1)
        {
            sysxWriter file;
            file.setFile(":system.syx");  // Read the default sysex file so whe don't start empty handed.
            if(file.readFile())
            {
                setFileSource(area, file.getSystemSource());
            };
        };
        sysxMsg = this->systemSource.hex.at( this->systemSource.address.indexOf(address) );
    }
    else
    {
        area = "Structure";
        if(this->fileSource.address.indexOf(address) == -1)
        {
            sysxWriter file;
            file.setFile(":default.syx");  // Read the default sysex file so whe don't start empty handed.
            if(file.readFile())
            {
                setFileSource(area, file.getFileSource());
            };
        };
        sysxMsg = this->fileSource.hex.at( this->fileSource.address.indexOf(address) );
    };

    return sysxMsg;
}

QString SysxIO::getCheckSum(int dataSize)
{
    MidiTable *midiTable = MidiTable::Instance();
    return midiTable->getCheckSum(dataSize);
}

QList<QString> SysxIO::correctSysxMsg(QList<QString> sysxMsg)
{
    QString address1 = sysxMsg.at(sysxAddressOffset + 2);
    QString address2 = sysxMsg.at(sysxAddressOffset + 3);
    bool isWrong = false;
    QString badHex;
    bool ok;

    MidiTable *midiTable = MidiTable::Instance();
    for(int i=sysxDataOffset;i<sysxMsg.size() - 3;i++)
    {
        QString address3 = QString::number(i - sysxDataOffset, 16).toUpper();
        if(address3.length()<2) address3.prepend("0");

        int range = midiTable->getRange("Structure", address1, address2, address3);
        int rangeMin = midiTable->getRangeMinimum("Structure", address1, address2, address3);

        if(midiTable->isData("Structure", address1, address2, address3))
        {
            int maxRange = QString("7F").toInt(&ok, 16) + 1; // index starts at 0 -> 0-127 = 128 entry's.
            int value1 = sysxMsg.at(i).toInt(&ok, 16);
            int value2 = sysxMsg.at(i + 1).toInt(&ok, 16);
            int value = (value1 * maxRange) + value2;

            if(value > range || value < rangeMin)
            {
                isWrong = true;
                if(value > range) { value = range;}
                else {value = rangeMin; };
                int dif = (int)(value/maxRange);
                QString valueHex1 = QString::number(dif, 16).toUpper();
                if(valueHex1.length() < 2) valueHex1.prepend("0");
                QString valueHex2 = QString::number(value - (dif * maxRange), 16).toUpper();
                if(valueHex2.length() < 2) valueHex2.prepend("0");

                badHex.append(" <br>");
                badHex.append(tr("Location = "));
                badHex.append(QString::number(i-sysxDataOffset, 16).toUpper());
                badHex.append(tr(": bad data = "));
                badHex.append(sysxMsg.at(i)+sysxMsg.at(i+1));
                badHex.append(tr(": new data = "));
                badHex.append(valueHex1+valueHex2);

                sysxMsg.replace(i, valueHex1);
                sysxMsg.replace(i + 1, valueHex2);
            };

            i++;
        }
        else
        {
            if(sysxMsg.at(i).toInt(&ok, 16) > range || sysxMsg.at(i).toInt(&ok, 16) < rangeMin)
            {
                isWrong = true;
                int value = (range / 2);
                QString valueHex = QString::number(value, 16).toUpper();
                if(valueHex.length() < 2) valueHex.prepend("0");

                badHex.append(" <br>");
                badHex.append(tr("Location = "));
                badHex.append(QString::number(i-sysxDataOffset, 16).toUpper());
                badHex.append(tr(": bad data = "));
                badHex.append(sysxMsg.at(i));
                badHex.append(tr(": new data = "));
                badHex.append(valueHex);
                sysxMsg.replace(i, valueHex);
            };
        };
    };

    int dataSize = 0;
    for(int i=checksumOffset; i<sysxMsg.size() - 1;i++)
    {
        dataSize += sysxMsg.at(i).toInt(&ok, 16);
    };
    sysxMsg.replace(sysxMsg.size() - 1, getCheckSum(dataSize));

    if (isWrong)
    {
        QString snork;
        for(int i=0;i<sysxMsg.size();i++)
        {
            snork.append(sysxMsg.at(i));
            snork.append(" ");
        };
        snork.append("F7");
        snork.append(tr("<br><br>bad data has been repaired with default values"));
        snork.append(tr("<br>loaction of bad data was at ")+ badHex);
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setWindowTitle(tr("File contains out of range data"));
        msgBox->setIcon(QMessageBox::Information);
        msgBox->setText(snork);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->exec();
    };
    return sysxMsg;
}

/***************************** isConnected() ******************************
* Connection status that's globaly accesible.
****************************************************************************/
bool SysxIO::isConnected()
{
    return this->connected;
}

void SysxIO::setConnected(bool connected)
{
    this->connected = connected;
    emit setStatusMessage(tr("Ready"));
}

/***************************** deviceReady() ******************************
* Midi busy or READY status that's globaly accesible.
* To prevent multiple messages sent at once.
****************************************************************************/
bool SysxIO::deviceReady()
{
    return this->status;
}

void SysxIO::setDeviceReady(bool status)
{
    this->status = status;
}

/***************************** isDevice() **********************************
* Flag that hold if the current sysex data we are editing is from file or DEVICE.
****************************************************************************/
bool SysxIO::isDevice()
{
    return this->isdevice;
}

void SysxIO::setDevice(bool isdevice)
{
    this->isdevice = isdevice;
}

/***************************** getSyncStatus() **********************************
* Flag that hold if the sysex data we are editing is synchronized with what's
* on the device.
****************************************************************************/
bool SysxIO::getSyncStatus()
{
    return this->syncStatus;
}

void SysxIO::setSyncStatus(bool syncStatus)
{
    this->syncStatus = syncStatus;
}

void SysxIO::setBank(int bank)
{
    this->bank = bank;
}

void SysxIO::setPatch(int patch)
{
    this->patch = patch;
}

int SysxIO::getBank(){
    return this->bank;
}

int SysxIO::getPatch(){
    return this->patch;
}

void SysxIO::setLoadedBank(int bank)
{
    this->loadedBank = bank;
}

void SysxIO::setLoadedPatch(int patch)
{
    this->loadedPatch = patch;
}

int SysxIO::getLoadedBank(){
    return this->loadedBank;
}

int SysxIO::getLoadedPatch(){
    return this->loadedPatch;
}

/*********************** getRequestName() ***********************************
* Set the name for check of the patch that we are going to load.
***************************************************************************/
void SysxIO::setRequestName(QString requestName)
{
    this->requestName = requestName;
}

/*********************** returnRequestName() ***********************************
* Return the name for check of the patch that should have been loaded.
***************************************************************************/
QString SysxIO::getRequestName()
{
    return this->requestName;
}

/*********************** getPatchChangeMsg() *****************************
* Formats the midi message for bank changing (temp buffer) and returns it.
*************************************************************************/
QString SysxIO::getPatchChangeMsg(int bank, int patch)
{
    int bankOffset = ((bank - 1) * patchPerBank) + (patch - 1);
    int bankMsbNum = (int)(bankOffset / bankSize);   // bankSize set in globalVariables.h
    int programChangeNum = bankOffset - (bankSize * bankMsbNum);
    QString bankMsb = QString::number(bankMsbNum, 16);
    QString programChange = QString::number(programChangeNum, 16);

    if (bankMsb.length() < 2) bankMsb.prepend("0");
    if (programChange.length() < 2) programChange.prepend("0");
    QString midiMsg;
    midiMsg.append("B000"+bankMsb);
    midiMsg.append("B02000");
    midiMsg.append("C0"+programChange);
    midiMsg = midiMsg.toUpper();
    return midiMsg;
}

/***************************** sendMidi() ***********************************
* Sends a midi message over the midiOut device sellected in the preferences.
****************************************************************************/
void SysxIO::sendMidi(QString midiMsg)
{
    //if(isConnected())
    {
        Preferences *preferences = Preferences::Instance(); bool ok;
        int midiOutPort = preferences->getPreferences("Midi", "MidiOut", "device").toInt(&ok, 10);	// Get midi out device from preferences.
        midiIO *midi = new midiIO();
        midi->sendMidi(midiMsg, midiOutPort);

        /*DeBugGING OUTPUT */
        if(preferences->getPreferences("Midi", "DBug", "bool")=="true")
        {
            QString dBug =("      ");
            dBug.append(midiMsg);
            emit setStatusdBugMessage(dBug);
        }
    };
};

/***************************** finishedSending() *************************************
* Signals that we are finished sending a midi message, so we can go one with our life.
*************************************************************************************/
void SysxIO::finishedSending()
{
    emit isFinished();
}

/***************************** requestPatchChange() *************************
* Send a patch change request. Result will be checked with checkPatchChange.
****************************************************************************/
void SysxIO::requestPatchChange(int bank, int patch)
{
    this->bankChange = bank;
    this->patchChange = patch;

    QObject::connect(this, SIGNAL(isFinished()),	// Connect the result of the request
                     this, SLOT(namePatchChange()));				// to returnPatchName function.

    QString midiMsg = getPatchChangeMsg(bank, patch);
    emit setStatusMessage(tr("Patch change"));
    this->sendMidi(midiMsg);
}

/***************************** namePatchChange() *************************
* Get the name of the patch we are switching to and check it with the
* one requested.
****************************************************************************/
void SysxIO::namePatchChange()
{
    QObject::disconnect(this, SIGNAL(isFinished()),
                        this, SLOT(namePatchChange()));
    QObject::disconnect(SIGNAL(patchName(QString)));

    QObject::connect(this, SIGNAL(patchName(QString)),
                     this, SLOT(checkPatchChange(QString)));
}

/***************************** checkPatchChange() *************************
* Emits a signal if the patch change was confirmed else it will retry until
* the maximum retry's has been reached.
****************************************************************************/
void SysxIO::checkPatchChange(QString name)
{
    Q_UNUSED(name);
    QObject::disconnect(this, SIGNAL(patchName(QString)),
                        this, SLOT(checkPatchChange(QString)));
    emit isChanged();
    this->changeCount = 0;
}

/***************************** sendSysx() ***********************************
* Sends a sysex message over the midiOut device sellected in the preferences.
*****************************************************************************/
void SysxIO::sendSysx(QString sysxMsg)
{
    Preferences *preferences = Preferences::Instance();  bool ok;
    int midiOutPort = preferences->getPreferences("Midi", "MidiOut", "device").toInt(&ok, 10);	// Get midi out device from preferences.
    int midiInPort = preferences->getPreferences("Midi", "MidiIn", "device").toInt(&ok, 10);	// Get midi in device from preferences.
    midiIO *midi = new midiIO();
    midi->sendSysxMsg(sysxMsg, midiOutPort, midiInPort);

    /*DeBugGING OUTPUT */
    if(preferences->getPreferences("Midi", "DBug", "bool")=="true")
    {
        dBug =(sysxMsg);
        emit setStatusdBugMessage(dBug);
    }
}

/***************************** receiveSysx() *******************************
* Receives possible replied sysex message on sendSysex.
****************************************************************************/
void SysxIO::receiveSysx(QString sysxMsg)
{
    /*DeBugGING OUTPUT */
   /* Preferences *preferences = Preferences::Instance(); // Load the preferences.
    if(preferences->getPreferences("Midi", "DBug", "bool")=="true")
    {
        if (sysxMsg.size() > 0 && isConnected() ){
            QString snork;
            snork.append(tr("{ size="));
            snork.append(QString::number(sysxMsg.size()/2, 10));
            snork.append("}");
            snork.append(tr("<br> midi data received\n"));
            if (sysxMsg == dBug){
                snork.append(tr("<br> WARNING: midi data received = data sent"));
                snork.append(tr("<br> caused by a midi loopback, port change is required<br>"));
            };
            snork.append("<font size='-2'><b>");
            for(int i=0;i<sysxMsg.size();++i)
            {
                snork.append(sysxMsg.mid(i, 2));
                snork.append(" ");
                i++;
            };
            snork.replace("F7", "F7 }");
            snork.replace("F0", "{ F0");
            snork.append("<b></font><br>");
            QMessageBox *msgBox = new QMessageBox();
            msgBox->setTextFormat(Qt::RichText);
            msgBox->setWindowTitle(tr("dBug Result for formatted syx message"));
            msgBox->setIcon(QMessageBox::Information);
            msgBox->setText(snork);
            msgBox->setStandardButtons(QMessageBox::Ok);
            msgBox->exec();
        };
    };*/
    emit sysxReply(sysxMsg);
}

/***************************** requestPatchName() ***************************
* Send a patch name request. Result will be send with the returnPatchName
* function.
****************************************************************************/
void SysxIO::requestPatchName(int bank, int patch)
{
    QObject::disconnect(this, SIGNAL(sysxReply(QString)),
                        this, SLOT(returnPatchName(QString)));

    QObject::connect(this, SIGNAL(sysxReply(QString)),	// Connect the result of the request
                     this, SLOT(returnPatchName(QString)));	    	// to returnPatchName function.
    /* Patch name request.*/
    MidiTable *midiTable = MidiTable::Instance();
    QString sysxMsg = midiTable->nameRequest(bank, patch);
    sendSysx(sysxMsg);
}

/***************************** returnPatchName() ***************************
* Emits a signal with the retrieved patch name.
****************************************************************************/
void SysxIO::returnPatchName(QString sysxMsg)
{
    QObject::disconnect(this, SIGNAL(sysxReply(QString)),
                        this, SLOT(returnPatchName(QString)));

    QString name;
    if(sysxMsg.size()/2 == patchSize)
    {
        int dataStartOffset = sysxNameOffset;   //pointer to start of names in patch file at 403 bytes.
        QString hex1, hex2, hex3, hex4;
        for(int i=dataStartOffset*2; i<(dataStartOffset*2)+(nameLength*2); ++i)   //read the length of name string.
        {
            QString hexStr = sysxMsg.mid(i, 2);
            if(hexStr == "7E")
            {
                name.append((QChar)(0x2192));
            }
            else if (hexStr == "7F")
            {
                name.append((QChar)(0x2190));
            }
            else
            {
                bool ok;
                name.append( (char)(hexStr.toInt(&ok, 16)) );
            };

            i=i+1;
        };
    } else if (!sysxMsg.isEmpty() && sysxMsg.size()/2 < patchSize)
    {name = tr("bad data"); }
    else {name = tr("no reply"); };

    emit patchName(name);
}

/***************************** requestPatch() ******************************
* Send a patch request. Result will be send directly with receiveSysx signal
****************************************************************************/
void SysxIO::requestPatch(int bank, int patch)
{
    /* Patch request. */
    MidiTable *midiTable = MidiTable::Instance();
    QString sysxMsg = midiTable->patchRequest(bank, patch);
    sendSysx(sysxMsg);
}

/***************************** errorSignal() ******************************
* Displays all midi related error messages.
****************************************************************************/
void SysxIO::errorSignal(QString errorType, QString errorMsg)
{
    setNoError(false);
    emit setStatusdBugMessage(errorType + "  " + errorMsg);
    this->errorType.clear();
    this->errorMsg.clear();
}

/***************************** noError() ******************************
* Error flag set on midi error to prevent (double) connection failure
* messages and a midi messages.
************************************************************************/
bool SysxIO::noError()
{
    return this->noerror;
}

void SysxIO::setNoError(bool status)
{
    this->noerror = status;
}

/***************************** CurrentPatchName() ******************************
* This is to make it possible to verify the patch name that we are trying to receive
* corresponds in case we had a name change that was not yet written permanantly
* to the device. (See floorBoardDisplay.cpp for more info)
************************************************************************/
void SysxIO::setCurrentPatchName(QString patchName)
{
    this->currentName = patchName;
}

QString SysxIO::getCurrentPatchName()
{
    return this->currentName;
}

/***************************** emit() *********************************
* Added to make status update possible from static methods and classes
* like the CALLBACK in midiIO.
************************************************************************/
void SysxIO::emitStatusSymbol(int value)
{
    emit setStatusSymbol(value);
}

void SysxIO::emitStatusProgress(int value)
{
    emit setStatusProgress(value);
}

void SysxIO::emitStatusMessage(QString message)
{
    emit setStatusMessage(message);
}

void SysxIO::emitStatusdBugMessage(QString dBug)
{
    emit setStatusdBugMessage(dBug);
}

void SysxIO::errorReturn(QString errorType, QString errorMsg)
{    
    this->errorType = errorType;
    this->errorMsg = errorMsg;
}

void SysxIO::systemWrite()
{    
    setDeviceReady(false);			// Reserve the device for interaction.
    QString sysxMsg;
    QList< QList<QString> > systemData = getSystemSource().hex; // Get the loaded system data.
    for(int i=0;i<systemData.size();++i)
    {
        QList<QString> data = systemData.at(i);
        for(int x=0;x<data.size();++x)
        {
            QString hex;
            hex = data.at(x);
            if (hex.length() < 2) hex.prepend("0");
            sysxMsg.append(hex);
        };
    };
    sendSysx(sysxMsg);	// Send the data.
    setDeviceReady(true);
}

void SysxIO::systemDataRequest()
{
    emit setStatusProgress(100);
    QString replyMsg;
    if (isConnected())
    {
        emit setStatusSymbol(2);
        emit setStatusMessage(tr("Request System data"));
        setDeviceReady(false); // Reserve the device for interaction.
        QObject::disconnect(this, SIGNAL(sysxReply(QString)));
        QObject::connect(this, SIGNAL(sysxReply(QString)), this, SLOT(systemReply(QString)));
        sendSysx(systemRequest); // GT-3 System area data Request.
        emit setStatusProgress(0);
    }
    else
    {
        QString snork = tr("Ensure connection is active and retry");
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setWindowTitle(deviceType + tr(" not connected !!"));
        msgBox->setIcon(QMessageBox::Information);
        msgBox->setText(snork);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->exec();
    };
}

void SysxIO::systemReply(QString replyMsg)
{
    QObject::disconnect(this, SIGNAL(sysxReply(QString)), this, SLOT(systemReply(QString)));
    setDeviceReady(true); // Free the device after finishing interaction.

    if(noError())
    {
        if(replyMsg.size()/2 == 5258)
        {
            QString area = "System";
            setFileSource(area, replyMsg);		// Set the source to the data received.
            setFileName(tr("System Data from ") + deviceType);	// Set the file name to GT-10B system for the display.
            setDevice(true);				// Patch received from the device so this is set to true.
            setSyncStatus(true);			// We can't be more in sync than right now! :)
        }
        else
        {
            QMessageBox *msgBox = new QMessageBox();
            msgBox->setWindowTitle(deviceType + tr(" Fx FloorBoard connection Error !!"));
            msgBox->setIcon(QMessageBox::Warning);
            msgBox->setTextFormat(Qt::RichText);
            QString msgText;
            msgText.append("<font size='+1'><b>");
            msgText.append(tr("The Boss ") + deviceType + tr(" Effects Processor was not found."));
            msgText.append("<b></font><br>");
            msgBox->setText(msgText);
            msgBox->setStandardButtons(QMessageBox::Ok);
            msgBox->exec();
        };
    };
    emit setStatusMessage(tr("Ready"));
}

void SysxIO::writeToBuffer()
{
    setDeviceReady(false);			// Reserve the device for interaction.
    QObject::disconnect(this, SIGNAL(isChanged()),
                        this, SLOT(writeToBuffer()));

    QString sysxMsg;
    QList< QList<QString> > patchData = getFileSource().hex; // Get the loaded patch data.
    emit setStatusSymbol(2);
    emit setStatusMessage(tr("Sync to ")+deviceType);
    QString addr1 = "0C";   // temp address
    QString addr2 = "00";

    for(int i=0;i<patchData.size();++i)
    {
        QList<QString> data = patchData.at(i);
        for(int x=0;x<data.size();++x)
        {
            QString hex;
            if(x == sysxAddressOffset)
            {
                hex = addr1;
            }
            else if(x == sysxAddressOffset + 1)
            {
                hex = addr2;
            }
            else
            {
                hex = data.at(x);
            };
            if (hex.length() < 2) hex.prepend("0");
            sysxMsg.append(hex);
        };
    };
    setSyncStatus(true);		// In advance of the actual data transfer we set it already to sync.

    QObject::connect(this, SIGNAL(sysxReply(QString)),	// Connect the result signal
                     this, SLOT(resetDevice(QString)));					// to a slot that will reset the device after sending.
    sendSysx(sysxMsg);	// Send the data.

    emit setStatusProgress(33); // time wasting sinusidal statusbar progress
    SLEEP(150);
    emit setStatusProgress(66);
    SLEEP(150);
    emit setStatusProgress(100);
    SLEEP(150);
    emit setStatusProgress(75);
    SLEEP(150);
    emit setStatusProgress(42);
    SLEEP(150);
}



