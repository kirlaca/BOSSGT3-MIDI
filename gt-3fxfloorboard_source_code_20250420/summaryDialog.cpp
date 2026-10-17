/****************************************************************************
**
** Copyright (C) 2007~2025 Colin Willcocks.
** Copyright (C) 2005~2007 Uco Mesdag.
** All rights reserved.
** This file is part of "GT-10B Fx FloorBoard".
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
#include <QDateTime>
#include "summaryDialog.h"
#include "Preferences.h"
#include "SysxIO.h"
#include "MidiTable.h"
#include "globalVariables.h"

summaryDialog::summaryDialog(QWidget *parent)
    : QWidget(parent)
{

    this->textDialog = new QTextEdit(parent);
    textDialog->setReadOnly(true);
    //textDialog->setWordWrapMode(QTextOption::NoWrap);
    textDialog->setDocumentTitle("GT-3 Patch settings");
    textDialog->setMinimumHeight(600);

    SysxIO *sysxIO = SysxIO::Instance();
    QString sysxMsg;
    QList< QList<QString> > patchData = sysxIO->getFileSource().hex; // Get the loaded patch data.
    for(int i=0;i<patchData.size();++i)
    {
        QList<QString> data = patchData.at(i);
        for(int x=0;x<data.size();++x)
        {
            QString hex;
            hex = data.at(x);
            if (hex.length() < 2) hex.prepend("0");
            sysxMsg.append(hex);
            if(hex == "F7"){ sysxMsg.append("<br>"); };
        };
    };
    /******************************************************
  ******QString "sysxMsg" contains current patch data *****
  ******************************************************/
    MidiTable *midiTable = MidiTable::Instance();
    QString patchName = sysxIO->getCurrentPatchName();
    text = "<b>Patch name: " + patchName + "</b>";
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString();
    text.append("<br>" + dateTimeString);
    /*text.append("........<b>Output Select = </b>");
  int value = sysxIO->getSourceValue("Structure", "00", "00", "11");
  QString valueHex = QString::number(value, 16).toUpper();
  if(valueHex.length() < 2) {valueHex.prepend("0"); };
  text.append(midiTable->getValue("Structure", "00", "00", "11", valueHex) ); */



    QList<QString> fxChain = sysxIO->getFileSource("Structure", "0B", "00");

    QString chain = "<br><br><b>FX Chain =</b>";
    for(int i= sysxDataOffset;i< (sysxDataOffset + 12);i++ )
    {
        chain.append(" [");
        chain.append( midiTable->getMidiMap("Structure", "0B", "00", "00", fxChain.at(i)).name );
        chain.append("]");
    };

    text.append(chain);
    text.append("<br>");
    text.append("<br>");

    text.append("<b><u>**********Pre Amp***********</b></u><br>");
    address= "04";
    start = 0;
    finish = 13;
    makeList();

    text.append("<b><u>**********SFX***********</b></u><br>");
    address= "00";
    start = 0;
    finish = 26;
    makeList();

    text.append("<b><u>**********Compressor***********</b></u><br>");
    address= "01";
    start = 0;
    finish = 10;
    makeList();

    text.append("<b><u>**********WAH***********</b></u><br>");
    address= "02";
    start = 0;
    finish = 12;
    makeList();

    text.append("<b><u>**********Distortion***********</b></u><br>");
    address= "03";
    start = 0;
    finish = 6;
    makeList();

    text.append("<b><u>**********Equalizer***********</b></u><br>");
    address= "05";
    start = 0;
    finish = 10;
    makeList();

    text.append("<b><u>**********Modulation***********</b></u><br>");
    address= "06";
    start = 0;
    finish = 101;
    makeList();

    text.append("<b><u>**********Delay***********</b></u><br>");
    address= "07";
    start = 0;
    finish = 8;
    makeList();

    text.append("<b><u>**********Chorus***********</b></u><br>");
    address= "08";
    start = 0;
    finish = 7;
    makeList();

    text.append("<b><u>**********Reverb***********</b></u><br>");
    address= "09";
    start = 0;
    finish = 8;
    makeList();

    text.append("<b><u>**********Master***********</b></u><br>");
    address= "0A";
    start = 0;
    finish = 7;
    makeList();

    text.append("<b><u>**********EXP Pedal***********</b></u><br>");
    address= "0D";
    start = 0;
    finish = 6;
    makeList();

    text.append("<b><u>**********CTL Pedal***********</b></u><br>");
    address= "0E";
    start = 0;
    finish = 8;
    makeList();

    text.append("<b><u>**********Assign 1***********</b></u><br>");
    address= "0F";
    start = 0;
    finish = 16;
    makeList();

    text.append("<b><u>**********Assign 2***********</b></u><br>");
    address= "10";
    start = 0;
    finish = 16;
    makeList();

    text.append("<b><u>**********Assign 3***********</b></u><br>");
    address= "11";
    start = 0;
    finish = 16;
    makeList();

    text.append("<b><u>**********Assign 4***********</b></u><br>");
    address= "12";
    start = 0;
    finish = 16;
    makeList();

    text.append("<b><u>**********Assign 5***********</b></u><br>");
    address= "13";
    start = 0;
    finish = 16;
    makeList();

    text.append("<b><u>**********Assign 6***********</b></u><br>");
    address= "14";
    start = 0;
    finish = 16;
    makeList();

    text.append("<b><u>**********Assign 7***********</b></u><br>");
    address= "15";
    start = 0;
    finish = 16;
    makeList();

    text.append("<b><u>**********Assign 8***********</b></u><br>");
    address= "16";
    start = 0;
    finish = 16;
    makeList();


    //text.append("<b><u>**********Patch Data***********</b></u><br>");
    //text.append(sysxMsg);

    textDialog->setText(text);
    textDialog->show();


    QPushButton *cancelButton = new QPushButton(tr("Close"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));

    QPushButton *printButton = new QPushButton(tr("Print"));
    connect(printButton, SIGNAL(clicked()), this, SLOT(printFile()));

    QPushButton *saveAsButton = new QPushButton(tr("Save As"));
    connect(saveAsButton, SIGNAL(clicked()), this, SLOT(saveAs()));


    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(textDialog);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(printButton);
    buttonsLayout->addSpacing(12);
    buttonsLayout->addWidget(saveAsButton);
    buttonsLayout->addSpacing(12);
    buttonsLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    setWindowTitle(deviceType + tr(" Patch Summary"));
};

void summaryDialog::makeList()
{
    SysxIO *sysxIO = SysxIO::Instance();
    MidiTable *midiTable = MidiTable::Instance();
    for(int i=start;i<finish;i++ )
    {
        QString pos = QString::number(i, 16).toUpper();
        if(pos.size()<2){ pos.prepend("0"); };
        QString txt =  midiTable->getMidiMap("Structure", address, "00", pos).desc;
        if(!txt.isEmpty() && txt != "")
        {
            int value = sysxIO->getSourceValue("Structure", address, "00", pos);
            QString valueHex = QString::number(value, 16).toUpper();
            if(valueHex.length() < 2) {valueHex.prepend("0"); };

            text.append("[");
            text.append(txt);
            text.append("] = ");
            text.append(midiTable->getValue("Structure", address, "00", pos, valueHex) );
            text.append("<br>");
        };
    };
    text.append("<br>");
};

void summaryDialog::valueChanged(int value)
{
    Q_UNUSED(value);
};

void summaryDialog::cancel()
{
    this->close();
};

void summaryDialog::printFile()
{
#ifndef  Q_PROCESSOR_ARM
#elif QT_NO_PRINTER
    QPrinter printer;
    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    //QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
    dialog->setWindowTitle(tr("Print Document"));
    if (dialog->exec() != QDialog::Accepted) { return; }
    else { textDialog->print(&printer); };
#endif
};

void summaryDialog::saveAs()
{

    Preferences *preferences = Preferences::Instance();
    QString dir = preferences->getPreferences("General", "Files", "dir");

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save As"),
        dir,
        tr("Text Document (*.txt)"));
    if (!fileName.isEmpty())
    {
        if(!fileName.contains(".txt"))
        {
            fileName.append(".txt");
        };
        QFile file(fileName);

        QByteArray out;
        text.remove("<b>");
        text.remove("</b>");
        text.remove("<u>");
        text.remove("</u>");
        QString newLine;
        newLine.append((char)13);
        newLine.append((char)10);
        text.replace("<br>", newLine);
        text.prepend("Boss GT-3 Patch  " + newLine);
        int size = text.size();

        for (int x=0; x<size; x++)
        {
            QString str(text.at(x));
            out.append(str.toLatin1());
        };

        if (file.open(QIODevice::WriteOnly))
        {

            file.write(out);
        };

    };
};
