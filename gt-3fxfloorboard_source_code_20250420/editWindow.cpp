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

#include "editWindow.h"
#include "MidiTable.h"
#include "SysxIO.h"
#include "globalVariables.h"

editWindow::editWindow(QWidget *parent)
    : QWidget(parent)
{
    this->image = QPixmap(":images/editwindow.png");
    this->setFixedSize(image.width(), image.height());
    /*this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->setWindowFlags(Qt::WindowStaysOnTopHint
        | Qt::WindowTitleHint
        | Qt::WindowMinimizeButtonHint
        | Qt::MSWindowsFixedSizeDialogHint);*/

    this->title = new QLabel;
    this->title->setObjectName("title");

    this->comboBoxLabel = new QLabel(tr("Select"));
    this->comboBoxLabel->setObjectName("selectlabel");
    this->comboBoxLabel->setVisible(false);

    this->pageComboBox = new QComboBox;
    this->pageComboBox->setObjectName("smallcombo");
    this->pageComboBox->setEditable(false);
    this->pageComboBox->setFrame(false);
    this->pageComboBox->setVisible(false);

    this->closeButton = new customControlLabel;
    this->closeButton->setButton(true);
    this->closeButton->setImage(":/images/closebutton.png");

    this->temp1_Button = new customControlLabel;
    this->temp1_Button->setButton(true);
    this->temp1_Button->setImage(":/images/pushbutton_dark.png");
    this->temp1_Button->setText(tr("Temp 1 Partial"));
    this->temp1_Button->setAlignment(Qt::AlignCenter);

    this->temp2_Button = new customControlLabel;
    this->temp2_Button->setButton(true);
    this->temp2_Button->setImage(":/images/pushbutton_dark.png");
    this->temp2_Button->setText(tr("Temp 2 Partial"));
    this->temp2_Button->setAlignment(Qt::AlignCenter);

    this->temp3_Button = new customControlLabel;
    this->temp3_Button->setButton(true);
    this->temp3_Button->setImage(":/images/pushbutton_dark.png");
    this->temp3_Button->setText(tr("Temp 3 Partial"));
    this->temp3_Button->setAlignment(Qt::AlignCenter);

    this->temp4_Button = new customControlLabel;
    this->temp4_Button->setButton(true);
    this->temp4_Button->setImage(":/images/pushbutton_dark.png");
    this->temp4_Button->setText(tr("Temp 4 Partial"));
    this->temp4_Button->setAlignment(Qt::AlignCenter);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(this->temp1_Button);
    buttonLayout->addWidget(this->temp2_Button);
    buttonLayout->addWidget(this->temp3_Button);
    buttonLayout->addWidget(this->temp4_Button);

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->addWidget(this->title);
    headerLayout->addStretch();
    headerLayout->addWidget(this->comboBoxLabel);
    headerLayout->addWidget(this->pageComboBox);
    headerLayout->addStretch();
    headerLayout->addLayout(buttonLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(this->closeButton);

    this->pagesWidget = new QStackedWidget;

    QHBoxLayout *pagesLayout = new QHBoxLayout;
    pagesLayout->addStretch();
    pagesLayout->addWidget(this->pagesWidget);
    pagesLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(headerLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(pagesLayout);
    mainLayout->addStretch();
    mainLayout->addSpacing(16);
    setLayout(mainLayout);

    this->tempPage = new editPage;

    QObject::connect(this->pageComboBox, SIGNAL(activated(int)), this->pagesWidget, SLOT(setCurrentIndex(int)));

    QObject::connect(this->closeButton, SIGNAL(mouseReleased()), this, SLOT(hide()));
    QObject::connect(this->temp1_Button, SIGNAL(mouseReleased()), this, SLOT(temp1()));
    QObject::connect(this->temp2_Button, SIGNAL(mouseReleased()), this, SLOT(temp2()));
    QObject::connect(this->temp3_Button, SIGNAL(mouseReleased()), this, SLOT(temp3()));
    QObject::connect(this->temp4_Button, SIGNAL(mouseReleased()), this, SLOT(temp4()));

    QObject::connect(this, SIGNAL( closeWindow() ), this, SLOT(hide()));

    QObject::connect(this, SIGNAL( dialogUpdateSignal() ), this, SLOT( pageUpdateSignal() ));

    QObject::connect(this->pageComboBox, SIGNAL(activated(int)), this, SLOT(valueChanged(int)));

}

void editWindow::paintEvent(QPaintEvent *)
{
    QRectF target(0.0, 0.0, image.width(), image.height());
    QRectF source(0.0, 0.0, image.width(), image.height());

    QPainter painter(this);
    painter.drawPixmap(target, image, source);
}

editWindow::~editWindow()
{

}

void editWindow::setLSB(QString hex1, QString hex2)
{
    Q_UNUSED(hex1);
    Q_UNUSED(hex2);
}

void editWindow::setWindow(QString title)
{
    this->title->setText(title);
    this->pagesWidget->setCurrentIndex(0);
}

QString editWindow::getTitle()
{
    return this->title->text();
}

void editWindow::addPage(QString hex1, QString hex2, QString hex3, QString hex4, QString area)
{
    this->hex1 = hex1;
    this->hex2 = hex2;
    this->hex3 = hex3;
    if (area == "normal" || area == "turbo" || area.isEmpty())
    {this->area = "Structure";} else {this->area = area; };
    this->tempPage->setGridLayout();
    this->editPages.append(this->tempPage);
    this->pagesWidget->addWidget(editPages.last());
    this->pages = this->pagesWidget->count();

    QObject::connect(this, SIGNAL( dialogUpdateSignal() ), editPages.last(), SIGNAL( dialogUpdateSignal() ));

    QObject::connect(editPages.last(), SIGNAL( updateSignal() ), this, SIGNAL( updateSignal() ));

    if (this->area != "Structure" || this->temp_hex1.isEmpty() || this->temp_hex1.contains("void"))
    {
        this->temp1_Button->hide();
        this->temp2_Button->hide();
        this->temp3_Button->hide();
        this->temp4_Button->hide();
    };

    if(hex1 != "void" && hex2 != "void" && hex3 != "void")
    {
        MidiTable *midiTable = MidiTable::Instance();
        QString hex0 = area;
        Midi items;
        if(!area.contains("System")) {
            items = midiTable->getMidiMap(this->area, hex1, hex2, hex3);
        } else {
            hex0.remove("System");
            items = midiTable->getMidiMap("System", hex0, hex1, hex2, hex3);
        };
        int itemsCount;
        if(hex4 == "void")
        {
            itemsCount = this->pagesWidget->count() - 1;
        }
        else
        {
            bool ok;
            itemsCount = hex4.toInt(&ok, 16);
        };

        QString item;
        QString desc = items.level.at(itemsCount).desc;
        QString customdesc = items.level.at(itemsCount).customdesc;
        if(!customdesc.isEmpty())
        {
            item = customdesc;
        }
        else
        {
            item = desc;
        };

        this->pageComboBox->addItem(item);
        this->tempPage = new editPage;
        this->pageComboBox->setMaxVisibleItems(this->pages);
        if(this->pages > 1)
        {
            this->pageComboBox->setVisible(true);
            this->comboBoxLabel->setVisible(true);
        };
    };
}

void editWindow::valueChanged(int index)
{
    if(hex1 != "void" && hex2 != "void")
    {
        QString valueHex = QString::number(index, 16).toUpper();
        if(valueHex.length() < 2) valueHex.prepend("0");

        SysxIO *sysxIO = SysxIO::Instance();
        sysxIO->setFileSource(this->area, this->hex1, this->hex2, this->hex3, valueHex);
        emit updateDisplay(valueHex);
        emit updateSignal();
    };
}

void editWindow::pageUpdateSignal()
{ 
    if(this->pages > 1 && this->hex1 != "void" && this->hex2 != "void" && !this->hex1.isEmpty() && !this->hex2.isEmpty())
    {
        SysxIO *sysxIO = SysxIO::Instance();
        int index = sysxIO->getSourceValue(this->area, this->hex1, this->hex2, this->hex3);
        this->pageComboBox->setCurrentIndex(index);
        this->pagesWidget->setCurrentIndex(index);
        this->valueChanged(index);
    };
}

editPage* editWindow::page()
{
    return this->tempPage;
}

void editWindow::closeEvent(QCloseEvent* ce)
{
    ce->accept();
}

void editWindow::temp1()     // paste partial patch effect only data
{

    SysxIO *sysxIO = SysxIO::Instance();
    if (!sysxIO->temp1_sysxMsg.isEmpty() && !temp_hex1.isEmpty() && !temp_hex1.contains("void") )   //check we are sending from a proper address
    {
        QList<QString> temp = sysxIO->temp1_sysxMsg.mid(this->position/2, this->length/2);   // copy the effct only section from the temp 1 saved patch.
        sysxIO->setFileSource("Structure", this->temp_hex1, "00", this->temp_hex3, temp);

        QList< QList<QString> > patchData = sysxIO->getFileSource().hex; // Get the loaded patch data.
        QString addr1 = tempBulkWrite;  // temp address
        QString addr2 = QString::number(0, 16).toUpper();
        QString sysxMsg;
        for(int i=0;i<patchData.size();++i)
        {
            QList<QString> data = patchData.at(i);
            for(int x=0;x<data.size();++x)
            {
                QString hex;
                if(x == sysxAddressOffset)
                { hex = addr1; }
                else if(x == sysxAddressOffset + 1)
                {	hex = addr2; }
                else
                {	hex = data.at(x);	};
                if (hex.length() < 2) hex.prepend("0");
                sysxMsg.append(hex);
            };
        };
        QString msg;
        int l=temp.size();
        for (int x=0; x<l; x++)
        {
            QString hex;
            hex = temp.at(x);
            if (hex.length() < 2) hex.prepend("0");
            msg.append(hex);
        };
        sysxMsg = sysxMsg.replace(this->position, this->length, msg);
        sysxIO->setFileSource("Structure", sysxMsg );
        emit dialogUpdateSignal();
    }
    else
    {
        QApplication::beep();
        sysxIO->emitStatusdBugMessage(tr("patch must be copied to clipboard first"));
    };
}

void editWindow::temp2()
{

    SysxIO *sysxIO = SysxIO::Instance();
    if (!sysxIO->temp2_sysxMsg.isEmpty() && !temp_hex1.isEmpty() && !temp_hex1.contains("void")  )
    {
        QList<QString> temp = sysxIO->temp2_sysxMsg.mid(this->position/2, this->length/2);
        sysxIO->setFileSource("Structure", this->temp_hex1, "00", this->temp_hex3, temp);
        QList< QList<QString> > patchData = sysxIO->getFileSource().hex; // Get the loaded patch data.
        QString addr1 = tempBulkWrite;  // temp address
        QString addr2 = QString::number(0, 16).toUpper();
        QString sysxMsg;
        for(int i=0;i<patchData.size();++i)
        {
            QList<QString> data = patchData.at(i);
            for(int x=0;x<data.size();++x)
            {
                QString hex;
                if(x == sysxAddressOffset)
                { hex = addr1; }
                else if(x == sysxAddressOffset + 1)
                {	hex = addr2; }
                else
                {	hex = data.at(x);	};
                if (hex.length() < 2) hex.prepend("0");
                sysxMsg.append(hex);
            };
        };
        QString msg;
        int l=temp.size();
        for (int x=0; x<l; x++)
        {
            QString hex;
            hex = temp.at(x);
            if (hex.length() < 2) hex.prepend("0");
            msg.append(hex);
        };
        sysxMsg = sysxMsg.replace(this->position, this->length, msg);
        sysxIO->setFileSource("Structure", sysxMsg );
        emit dialogUpdateSignal();
    }
    else
    {
        QApplication::beep();
        sysxIO->emitStatusdBugMessage(tr("patch must be copied to clipboard first"));
    };
}

void editWindow::temp3()
{

    SysxIO *sysxIO = SysxIO::Instance();
    if (!sysxIO->temp3_sysxMsg.isEmpty() && !temp_hex1.isEmpty() && !temp_hex1.contains("void")  )
    {
        QList<QString> temp = sysxIO->temp3_sysxMsg.mid(this->position/2, this->length/2);
        sysxIO->setFileSource("Structure", this->temp_hex1, "00", this->temp_hex3, temp);
        QList< QList<QString> > patchData = sysxIO->getFileSource().hex; // Get the loaded patch data.
        QString addr1 = tempBulkWrite;  // temp address
        QString addr2 = QString::number(0, 16).toUpper();
        QString sysxMsg;
        for(int i=0;i<patchData.size();++i)
        {
            QList<QString> data = patchData.at(i);
            for(int x=0;x<data.size();++x)
            {
                QString hex;
                if(x == sysxAddressOffset)
                { hex = addr1; }
                else if(x == sysxAddressOffset + 1)
                {	hex = addr2; }
                else
                {	hex = data.at(x);	};
                if (hex.length() < 2) hex.prepend("0");
                sysxMsg.append(hex);
            };
        };
        QString msg;
        int l=temp.size();
        for (int x=0; x<l; x++)
        {
            QString hex;
            hex = temp.at(x);
            if (hex.length() < 2) hex.prepend("0");
            msg.append(hex);
        };
        sysxMsg = sysxMsg.replace(this->position, this->length, msg);
        sysxIO->setFileSource("Structure", sysxMsg );
        emit dialogUpdateSignal();
    }
    else
    {
        QApplication::beep();
        sysxIO->emitStatusdBugMessage(tr("patch must be copied to clipboard first"));
    };
}

void editWindow::temp4()
{

    SysxIO *sysxIO = SysxIO::Instance();
    if (!sysxIO->temp4_sysxMsg.isEmpty() && !temp_hex1.isEmpty() && !temp_hex1.contains("void")  )
    {
        QList<QString> temp = sysxIO->temp4_sysxMsg.mid(this->position/2, this->length/2);
        sysxIO->setFileSource("Structure", this->temp_hex1, "00", this->temp_hex3, temp);
        QList< QList<QString> > patchData = sysxIO->getFileSource().hex; // Get the loaded patch data.
        QString addr1 = tempBulkWrite;  // temp address
        QString addr2 = QString::number(0, 16).toUpper();
        QString sysxMsg;
        for(int i=0;i<patchData.size();++i)
        {
            QList<QString> data = patchData.at(i);
            for(int x=0;x<data.size();++x)
            {
                QString hex;
                if(x == sysxAddressOffset)
                { hex = addr1; }
                else if(x == sysxAddressOffset + 1)
                {	hex = addr2; }
                else
                {	hex = data.at(x);	};
                if (hex.length() < 2) hex.prepend("0");
                sysxMsg.append(hex);
            };
        };
        QString msg;
        int l=temp.size();
        for (int x=0; x<l; x++)
        {
            QString hex;
            hex = temp.at(x);
            if (hex.length() < 2) hex.prepend("0");
            msg.append(hex);
        };
        sysxMsg = sysxMsg.replace(this->position, this->length, msg);
        sysxIO->setFileSource("Structure", sysxMsg );
        emit dialogUpdateSignal();
    }
    else
    {
        QApplication::beep();
        sysxIO->emitStatusdBugMessage(tr("patch must be copied to clipboard first"));
    };
}

void editWindow::patchPos(int pos, int len, QString t_hex1, QString t_hex3)
{
    this->position = pos;
    this->length = len;
    this->temp_hex1 = t_hex1;
    this->temp_hex3 = t_hex3;
}

