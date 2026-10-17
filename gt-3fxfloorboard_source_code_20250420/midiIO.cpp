/****************************************************************************
**
** Copyright (C) 2007~2025 Colin Willcocks.
** Copyright (C) 2005~2007 Uco Mesdag.
** All rights reserved.
** This file is part of "GT-100 Fx FloorBoard".
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

#include "midiIO.h"
#include "midiIODestroyer.h"
#include "SysxIO.h"
#include "globalVariables.h"
#include "RtMidi.h"
#include "Preferences.h"
//#include "MidiTable.h"

int midiIO::bytesTotal = 0;
int midiIO::bytesReceived = 0;
int loopCount;
bool midiIO::dataReceive = false;
QString midiIO::sysxBuffer;
QString midiIO::msgType = "name";
std::string clientName = "FxFloorBoard";
midiIO* midiIO::_instance = 0;// initialize pointer
midiIODestroyer midiIO::_destroyer;

midiIO::midiIO()
{
    this->midi = false; // Set this to false until required;
    /* Connect signals */
    SysxIO *sysxIO = SysxIO::Instance();
    QObject::connect(this, SIGNAL(setStatusSymbol(int)), sysxIO, SIGNAL(setStatusSymbol(int)));
    QObject::connect(this, SIGNAL(setStatusProgress(int)), sysxIO, SIGNAL(setStatusProgress(int)));
    QObject::connect(this, SIGNAL(setStatusMessage(QString)), sysxIO, SIGNAL(setStatusMessage(QString)));
    QObject::connect(this, SIGNAL(setStatusdBugMessage(QString)), sysxIO, SIGNAL(setStatusdBugMessage(QString)));
    QObject::connect(this, SIGNAL(errorSignal(QString, QString)), sysxIO, SLOT(errorSignal(QString, QString)));
    QObject::connect(this, SIGNAL(replyMsg(QString)),	sysxIO, SLOT(receiveSysx(QString)));
    QObject::connect(this, SIGNAL(midiFinished()), sysxIO, SLOT(finishedSending()));
#if defined(Q_OS_MAC)
    RtMidiOut *midiout = new RtMidiOut();
    RtMidiIn *midiin = new RtMidiIn();
#endif
}

midiIO* midiIO::Instance()
{
    /* Multi-threading safe */
    if (!_instance)  // is it the first call?
    {
        _instance = new midiIO; // create sole instance
        _destroyer.SetmidiIO(_instance);
    };
    return _instance; // address of sole instance
}

/*********************** queryMidiOutDevices() *****************************
 * Retrieves all MIDI Out devices installed on your system and stores them
 * as a QList of QStrings and device id's.
 *************************************************************************/
void midiIO::queryMidiOutDevices()
{
    RtMidiOut *midiout = new RtMidiOut();
    std::string portName;
    unsigned int outPorts;
    try
    {
        outPorts = midiout->getPortCount();      /* Check outputs. */
        for ( unsigned int i=0; i<outPorts; i++ )
        {
            portName = midiout->getPortName(i);
            QString name = QString::fromStdString(portName);
            name.chop(2);
            this->midiOutDevices.append(name);
        };
    }
    catch (RtMidiError &error)
    {
        emit errorSignal(tr("Midi Output Error"), tr("port availability error"));
        emit setStatusdBugMessage(QString::fromStdString( error.getMessage()));
        this->midiOutDevices.append("Device access error");
        goto cleanup;
    };

    if (outPorts < 1) { this->midiOutDevices.push_back(tr("no midi device available")); };
    /* Clean up */
cleanup:
    delete midiout;
}

QList<QString> midiIO::getMidiOutDevices()
{
    queryMidiOutDevices();
    return this->midiOutDevices;
}

/*********************** queryMidiInDevices() ******************************
 * Retrieves all MIDI In devices installed on your system and stores them
 * as a QList of QStrings and device id's.
 *************************************************************************/
void midiIO::queryMidiInDevices()
{
    RtMidiIn *midiin = new RtMidiIn();
    std::string portName;
    unsigned int inPorts;
    try
    {
        inPorts = midiin->getPortCount();      /* Check inputs. */
        for ( unsigned int i=0; i<inPorts; i++ )
        {
            portName = midiin->getPortName(i);
            QString name = QString::fromStdString(portName);
            name.chop(2);
            this->midiInDevices.append(name);
        };
    }
    catch (RtMidiError &error)
    {
        emit errorSignal(tr("Midi Input Error"), tr("port availability error"));
        emit setStatusdBugMessage(QString::fromStdString( error.getMessage()));
        this->midiInDevices.append("Device access error");
        goto cleanup;
    };

    if (inPorts < 1)
    { this->midiInDevices.push_back(tr("no midi device available")); };
    // Clean up
cleanup:
    delete midiin;
}

QList<QString> midiIO::getMidiInDevices()
{
    queryMidiInDevices();
    return this->midiInDevices;
}

/*********************** sendMsg() **********************************
 * Prepares the sysx message before sending on the MIDI Out device. It
 * converts the message from a QString to a std::vector and opens, sends
 * and closes the MIDI device.
 *************************************************************************/
void midiIO::sendSyxMsg(QString sysxOutMsg, int midiOutPort)
{
    RtMidiOut *midiout = new RtMidiOut();
    QString hex;
    int wait = 0;
    int s = sysxOutMsg.size()/100;
    int p=0;
    int retryCount = 0;
    std::vector<unsigned char> message;
    message.reserve(256);
    int msgLength = 0;
    msgLength = sysxOutMsg.length()/2;
    char *ptr  = new char[msgLength];		// Convert QString to char* (hex value)
    int nPorts = midiout->getPortCount();   // Check available ports.
    if ( nPorts < 1 )
    {
        goto cleanup;
    };
RETRY:
    try {
        if(!midiout->isPortOpen())
        {
            midiout->openPort(midiOutPort);	    // Open selected port.
            for(int i=0;i<msgLength*2;++i)
            {
                unsigned int n;
                hex = sysxOutMsg.mid(i, 2);
                bool ok;
                n = hex.toInt(&ok, 16);
                *ptr = (char)n;
                message.push_back(*ptr);		    // insert the char* string into a std::vector
                wait = wait + 1;
                p=p+s;
                emit setStatusProgress(wait);
                ptr++; i++;
                if(hex=="F7")
                {
                    midiout->sendMessage(&message);
                    message.clear();
                    msleep(5);
                };
            };
        };
        /*DeBugGING OUTPUT */
        emit setStatusdBugMessage(sysxOutMsg);
        goto cleanup;
    }
    catch (RtMidiError &error)
    {
        msleep(100);
        retryCount = retryCount + 1;
        if (retryCount < 4)
        {
            goto RETRY;
        };

        emit errorSignal(tr("Syx Output Error"), tr("data error"));
        emit setStatusdBugMessage(QString::fromStdString( error.getMessage()));
        goto cleanup;
    };
    /* Clean up */
cleanup:

    if(midiout->isPortOpen())
    {
        midiout->closePort();
    };
    msleep(25);						// wait as long as the message is sending.
    delete midiout;
}

/*********** send midi messages *******************************/
void midiIO::sendMidiMsg(QString sysxOutMsg, int midiOutPort)
{
    RtMidiOut *midiout = new RtMidiOut();
    unsigned int nPorts = midiout->getPortCount();   // Check available ports.
    if ( nPorts < 1 )
    {
        goto cleanup;
    };
    try {
        if(!midiout->isPortOpen()) {midiout->openPort(midiOutPort/*, clientName*/);	// Open selected port.
            std::vector<unsigned char> message;
            int msgLength = sysxOutMsg.length()/2;
            char *ptr  = new char[msgLength];		// Convert QString to char* (hex value)
            for(int i=0;i<msgLength*2;++i)
            {
                int n;
                QString hex;
                hex.append(sysxOutMsg.mid(i, 2));
                bool ok;
                n = hex.toInt(&ok, 16);
                *ptr = (char)n;
                message.push_back(*ptr);		// insert the char* string into a std::vector
                ptr++; i=i+1;
            };
            midiout->sendMessage(&message);  // send the midi data as a std::vector
        };
        goto cleanup;
    }
    catch (RtMidiError &error)
    {

        emit errorSignal(tr("Midi Output Error"), tr("data error"));
        emit setStatusdBugMessage(QString::fromStdString( error.getMessage()));
        goto cleanup;
    };
    /* Clean up*/
cleanup:
    if(midiout->isPortOpen()) { midiout->closePort(); };
    delete midiout;
}
/*********************** receiveMsg() **********************************
 * Prepares the sysx message after receiving on the MIDI In device. It
 *  and opens, receives and closes the MIDI device then converts the
 *  message from a std::vector to a QString.
 *************************************************************************/
void midicallback(double deltatime, std::vector<unsigned char> *message, void *userData)
{
    Q_UNUSED(deltatime);
    Q_UNUSED(userData);
    QString rxData;

    int nBytes = (int)message->size();
    for (int i=0; i<nBytes; i++ )
    {
        int n = ((int)message->at(i));					// convert std::vector to QString
        QString hex = QString::number(n, 16).toUpper();
        if (hex.length() < 2) hex.prepend("0");
        rxData.append(hex);
    };

    midiIO *midi = new midiIO();
    if (rxData.contains("F0") || rxData.contains("F7"))   // filter out non-syx events
    { midi->callbackMsg(rxData); };
}

void midiIO::callbackMsg(QString rxData)
{
    this->sysxBuffer.append(rxData);
}

void midiIO::receiveMsg(int midiInPort)
{
    RtMidiIn *midiin = new RtMidiIn();
    count = 0;
    emit setStatusSymbol(3);
    int x = 1;
    if (msgType == "patch") { loopCount = x*400; count = patchSize; }
    else if (msgType == "system"){ loopCount = x*900; count = 5258; } //gt-3 system size.
    else if (msgType == "name")  { loopCount = x*400; count = patchSize; }
    else if (msgType == "identity")   {loopCount = x*50; count = 15;}   // id reply size
    else { loopCount = x*1; count = 0; };
    unsigned int nPorts = midiin->getPortCount();	   // check we have a midiIn port
    if ( nPorts < 1 || QString::fromStdString(midiin->getPortName(midiInPort))=="")
    {
        goto cleanup;
    };
    try
    {
        midiin->ignoreTypes(false, true, true);  //don,t ignore sysex messages, but ignore other crap like active-sensing
        if(!midiin->isPortOpen())
            midiin->openPort(midiInPort);             // open the midi in port
        midiin->setCallback(&midicallback);    // set the callback

        sendSyxMsg(sysxOutMsg, midiOutPort);      // send the data request message out
        int x = 0;
        unsigned int s = 1;
        unsigned int t = 1;
        while (x<loopCount && sysxBuffer.size()/2 < count)  // wait until exact bytes received or timeout
        {
            msleep(5);
            s = (this->sysxBuffer.size()*50)/(count+1);
            t = (x*100)/loopCount;
            if (s>t) {t=s;};
            emit setStatusProgress(t);
            x++;
        };                   // time it takes to get all sysx messages in.
        goto cleanup;
    }
    catch (RtMidiError &error)
    {
        emit errorSignal(tr("Midi Input Error"), tr("data error"));
        emit setStatusdBugMessage(QString::fromStdString( error.getMessage()));
        goto cleanup;
    };
    /*Clean up */
cleanup:
    emit setStatusProgress(100);
    this->sysxInMsg = this->sysxBuffer;		   //get the returning data string
    dataReceive = true;
    if(midiin->isPortOpen())
    {
        midiin->cancelCallback(); midiin->closePort();
    };             // close the midi in port
    //msleep(25);
    delete midiin;
}


/**************************** run() **************************************
 * New QThread that processes the sysex or midi message and handles if yes 
 * or no it has to start receiving a reply on the MIDI In device midiIn. If 
 * so midiCallback() will handle the receive of the incomming sysx message.
 *************************************************************************/
void midiIO::run()
{
    int repeat = 0;
    if(midi && midiMsg.size() > 1)	// Check if we are going to send sysx or midi data & have an actual midi message to send.
    {

        if (midiMsg.size() <= 6)		// if the midi message is <= 3 words
        {
            sysxOutMsg = midiMsg;   // use the same sending routine as sysx messages.
            sendMidiMsg(sysxOutMsg, midiOutPort);
        }

        else
        {
            int z=0;
            sysxOutMsg = midiMsg.mid(z,2);
            sysxOutMsg.append(midiMsg.mid(z+2,2));  // skip every second byte
            sysxOutMsg.append(midiMsg.mid(z+4,2));  // skip every second byte
            sendMidiMsg(sysxOutMsg, midiOutPort);
            z=(z+6);								// advance to the next midi message in the string

            sysxOutMsg = midiMsg.mid(z,2);
            sysxOutMsg.append(midiMsg.mid(z+2,2));  // skip every second byte
            sysxOutMsg.append(midiMsg.mid(z+4,2));  // skip every second byte
            sendMidiMsg(sysxOutMsg, midiOutPort);
            z=(z+6);								// advance to the next midi message in the string

            sysxOutMsg = midiMsg.mid(z,2);
            sysxOutMsg.append(midiMsg.mid(z+2,2));  // skip every second byte
            sysxOutMsg.append(midiMsg.mid(z+4,2));  // skip every second byte
            sendMidiMsg(sysxOutMsg, midiOutPort);
        };
        emit setStatusSymbol(2);
        emit setStatusProgress(33); // time wasting sinusidal statusbar progress animation
        msleep(40);
        emit setStatusProgress(66);
        msleep(80);
        emit setStatusProgress(100);
        msleep(100);
        emit setStatusProgress(75);
        msleep(80);
        emit setStatusProgress(42);
        msleep(50);
        emit setStatusProgress(0);
        emit midiFinished(); // We are finished so we send a signal to free the device.
    }
    else   // if not a midi message, then it must be a sysx message
    {
    RECEIVE:
        this->dataReceive = false;
        this->sysxBuffer.clear();
        this->sysxInMsg.clear();
        /* Check if we are going to receive something on sending */
        bool receive;
        (this->sysxOutMsg.mid(sysxAddressOffset*2-2, 2) != "12")? receive = true: receive = false;

        if(receive==true)
        {
            emit setStatusSymbol(3);
            /* Get the size of data bytes returned to calculate the progress percentage */
            bool ok;
            QString sizeChunk = sysxOutMsg.mid(sysxDataOffset * 2, 4 * 2);
            int dataLenght = sizeChunk.toInt(&ok, 16);
            bytesTotal = (sysxDataOffset + 2) + dataLenght;
            if(dataLenght == 0) // Id request>
            {
                bytesTotal = 1;
            }
            else if(sizeChunk == patchRequestSize) // GT-3 Patch Request data size.
            {
                bytesTotal = patchSize;     // progressbar scaled to patch size
            };
            dataReceive = true;
            receiveMsg(midiInPort);
            Preferences *preferences = Preferences::Instance(); // Load the preferences.
            if((this->sysxBuffer.size()/2 != count) && (repeat<3) && preferences->getPreferences("Midi", "DBug", "bool")!="true")
            {
                emit setStatusdBugMessage(tr("re-trying data request"));
                repeat = repeat+1;
                goto RECEIVE;
            };
            emit midiFinished();
        }
        else
        {
            emit setStatusSymbol(2);
            sendSyxMsg(sysxOutMsg, midiOutPort);
            emit setStatusProgress(33);  // do the statusbar progress thing
            msleep(20);		// and wait predetermined time before being able to send more again.
            emit setStatusProgress(75);
            msleep(20);
            emit setStatusProgress(100);
            msleep(20);
            emit setStatusProgress(66);
            msleep(20);
            emit midiFinished(); // We are finished so we send a signal to free the device.
        };

        emit replyMsg(sysxInMsg);
        emit setStatusSymbol(1);
        emit setStatusProgress(0);
    };
}

/*********************** sendSysxMsg() ***********************************
 * Starts a new thread that handles the send and receive of sysex messages.
 *************************************************************************/
void midiIO::sendSysxMsg(QString sysxOutMsg, int midiOutPort, int midiInPort)
{	
    QString reBuild;
    QString sysxEOF;
    QString hex;
    int msgLength = sysxOutMsg.length()/2;
    for(int i=0;i<msgLength*2;++i)
    {
        hex.append(sysxOutMsg.mid(i*2, 2));
        sysxEOF = sysxOutMsg.mid((i*2)+4, 2);
        if (sysxEOF == "F7")

        {
            int dataSize = 0; bool ok;
            for(int h=checksumOffset;h<hex.size()-1;++h)
            { dataSize += hex.mid(h*2, 2).toInt(&ok, 16); };
            QString base = "80";                       // checksum calculate.
            unsigned int sum = dataSize % base.toInt(&ok, 16);
            if(sum!=0) { sum = base.toInt(&ok, 16) - sum; };
            QString checksum = QString::number(sum, 16).toUpper();
            if(checksum.length()<2) {checksum.prepend("0");};
            hex.append(checksum);
            hex.append("F7");
            if (!hex.contains("F00000001B12")) // remove txt portions of the message from address 00 16 00 00 onwards
            {reBuild.append(hex); };
            hex.clear();
            sysxEOF.clear();
            i=i+2;
        };
    };
    msgType = "";
    if (sysxOutMsg == idRequestString){reBuild = sysxOutMsg;  msgType = "identity"; emit setStatusdBugMessage(tr("identity request")); };  // identity request not require checksum

    this->sysxOutMsg = reBuild.simplified().toUpper();

    if((sysxOutMsg.size() == 32) && sysxOutMsg != idRequestString &&(sysxOutMsg.contains(patchRequestSize)) && (sysxOutMsg.mid((sysxAddressOffset*2-2), 2) == "11"))
    { msgType = "patch";  emit setStatusdBugMessage(tr("patch request")); };

    if (sysxOutMsg != idRequestString && sysxOutMsg.contains("F04100001B11000000000"))
    {msgType = "system";  emit setStatusdBugMessage(tr("system request")); };

    if (!sysxOutMsg.contains("F04100001B")){emit setStatusSymbol(1); emit replyMsg(""); return; };

    this->midiOutPort = midiOutPort;
    this->midiInPort = midiInPort;
    this->midi = false;
    Preferences *preferences = Preferences::Instance();// Load the preferences.
    QString midiOut = preferences->getPreferences("Midi", "MidiOut", "device");
    if(midiOut != "" && sysxOutMsg.contains("F04100001B")) {start();} else {
        emit setStatusSymbol(0);
        emit setStatusMessage(tr("no midi device set"));
        emit replyMsg("");};
}

/*********************** sendMidi() **********************************
 * Starts a new thread that handles the sending of the midi messages.
 **********************************************************************/
void midiIO::sendMidi(QString midiMsg, int midiOutPort)
{	
    this->midiOutPort = midiOutPort;
    this->midiMsg = midiMsg;
    this->midi = true;
    Preferences *preferences = Preferences::Instance();// Load the preferences.
    QString midiOut = preferences->getPreferences("Midi", "MidiOut", "device");
    if(midiOut!="" && !midiMsg.isEmpty()) {start();} else {
        emit setStatusSymbol(0);
        emit setStatusMessage(tr("no midi device set"));
    };
}

