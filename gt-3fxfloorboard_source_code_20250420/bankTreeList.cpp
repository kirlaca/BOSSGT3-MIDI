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

#include <QLayout>
#include <QMessageBox>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include "bankTreeList.h"
//#include "Preferences.h"
//#include "MidiTable.h"
#include "SysxIO.h"
#include "globalVariables.h"

bankTreeList::bankTreeList(QWidget *parent)
    : QWidget(parent)
{
    QFont font;
    font.setStretch(85);

    this->treeList = newTreeList();
    this->treeList->setObjectName("banklist");
    QObject::connect(treeList, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(setOpenItems(QTreeWidgetItem*)));

    QObject::connect(treeList, SIGNAL(itemCollapsed(QTreeWidgetItem*)),	this, SLOT(setClosedItems(QTreeWidgetItem*)));

    QObject::connect(treeList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(setItemClicked(QTreeWidgetItem*, int)));

    QObject::connect(treeList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(setItemDoubleClicked(QTreeWidgetItem*, int)));

    QObject::connect(this, SIGNAL(updateSignal()), this->parent(), SIGNAL(updateSignal()));

    QVBoxLayout *treeListLayout = new QVBoxLayout;
    treeListLayout->addWidget(treeList);
    treeListLayout->setContentsMargins(0, 0, 0, 0);
    treeListLayout->setSpacing(0);
    setLayout(treeListLayout);

    SysxIO *sysxIO = SysxIO::Instance();
    QObject::connect(this, SIGNAL(setStatusSymbol(int)), sysxIO, SIGNAL(setStatusSymbol(int)));

    QObject::connect(this, SIGNAL(setStatusProgress(int)), sysxIO, SIGNAL(setStatusProgress(int)));

    QObject::connect(this, SIGNAL(setStatusMessage(QString)), sysxIO, SIGNAL(setStatusMessage(QString)));

    QObject::connect(this, SIGNAL(notConnectedSignal()), sysxIO, SIGNAL(notConnectedSignal()));
}

void bankTreeList::updateSize(QRect newrect)
{
    this->setGeometry(newrect);
}

void bankTreeList::setClosedItems(QTreeWidgetItem *item)
{
    if(item->childCount() == patchPerBank)
    {
        int index = openPatchTreeItems.indexOf(item);
        openPatchTreeItems.removeAt(index);
    }
    else if(item->childCount() == 5)
    {
        int index = openBankTreeItems.indexOf(item);
        openBankTreeItems.removeAt(index);
        closeChildren(item);
    }
    else
    {
        closeChildren(item);
    };
}

void bankTreeList::closeChildren(QTreeWidgetItem *item)
{
    for(int i=0;i<item->childCount();++i)
    {
        for(int n=0;n<item->child(i)->childCount();++n)
        {
            item->child(i)->child(n)->setExpanded(false);
        };
        item->child(i)->setExpanded(false);
    };
}

void bankTreeList::setOpenItems(QTreeWidgetItem *item)
{
    QString type = "root";
    if(item->childCount() == patchPerBank)
    {
        openPatchTreeItems.append(item);
        updateTree(item);
        type = "patch";
    }
    else if(item->childCount() == 5)
    {
        openBankTreeItems.append(item);
        type = "bank";
    };

    int c = openPatchTreeItems.size();
    int b = openBankTreeItems.size();
    int a = 0;
    if(treeList->isExpanded(treeList->model()->index(0, 0)) &&
            treeList->isExpanded(treeList->model()->index(1, 0)))
    {
        a = 2;
    }
    else if(treeList->isExpanded(treeList->model()->index(0, 0)) ||
            treeList->isExpanded(treeList->model()->index(1, 0)))
    {
        a = 1;
    };

   /* int userBankCount = 0;
    for(int i=0;i<openBankTreeItems.size();++i)
    {
        if(openBankTreeItems.at(i)->parent()->text(0).contains("User"))
        {
            userBankCount++;
        };
    };*/


    if(type == "root")
    {
        if(a > 1)
        {
            int maxExpandedItems = 1;
            while(c > 3)
            {
                openPatchTreeItems.first()->setExpanded(false);
                c = openPatchTreeItems.size();
            };
            switch(c)
            {
            case 0: maxExpandedItems = 3; break;
            case 1: maxExpandedItems = 2; break;
            case 2: maxExpandedItems = 1; break;
            case 3: maxExpandedItems = 1; break;
            };

            while(b > maxExpandedItems)
            {
                openBankTreeItems.first()->setExpanded(false);;
                b = openBankTreeItems.size();
            };
        };
    }
    else if(type == "bank")
    {
        int maxExpandedItems = 1;
        if(a > 1)
        {
            switch(c)
            {
            case 0: maxExpandedItems = 3; break;
            case 1: maxExpandedItems = 2; break;
            case 2: maxExpandedItems = 1; break;
            case 3: maxExpandedItems = 1; break;
            };
        }
        else
        {
            if(treeList->isExpanded(treeList->model()->index(0, 0)))
            {
                switch(c)
                {
                case 0: maxExpandedItems = 5; break;
                case 1: maxExpandedItems = 4; break;
                case 2: maxExpandedItems = 3; break;
                case 3: maxExpandedItems = 3; break;
                case 4: maxExpandedItems = 2; break;
                case 5: maxExpandedItems = 1; break;
                };
            }
            else
            {
                switch(c)
                {
                case 0: maxExpandedItems = 4; break;
                case 1: maxExpandedItems = 4; break;
                case 2: maxExpandedItems = 3; break;
                case 3: maxExpandedItems = 3; break;
                case 4: maxExpandedItems = 1; break;
                case 5: maxExpandedItems = 1; break;
                };
            };
        };

        if(maxExpandedItems == 1)
        {
            openPatchTreeItems.first()->setExpanded(false);
            if(treeList->isExpanded(treeList->model()->index(1, 0)))
            {
                openPatchTreeItems.first()->setExpanded(false);
            }
            maxExpandedItems = 2;
        };
        if(b > maxExpandedItems)
        {
            openBankTreeItems.first()->setExpanded(false);;
        };
    }
    else if(type == "patch")
    {
        for(int i=0;i<openBankTreeItems.size();++i)
        {
            bool allCollapsed = true;
            for(int n=0;n<openBankTreeItems.at(i)->childCount();++n)
            {
                if(openBankTreeItems.at(i)->child(n)->isExpanded())
                {
                    allCollapsed = false;
                };
            };
            if(allCollapsed)
            {
                openBankTreeItems.at(i)->setExpanded(false);
                b = openBankTreeItems.size();
                break;
            };
        };

        int maxExpandedItems = 1;
        if(a > 1)
        {
            switch(b)
            {
            case 1: maxExpandedItems = 3; break;
            case 2: maxExpandedItems = 1; break;
            case 3: maxExpandedItems = 0; break;
            };
        }
        else
        {
            if(treeList->isExpanded(treeList->model()->index(0, 0)))
            {
                switch(b)
                {
                case 1: maxExpandedItems = 5; break;
                case 2: maxExpandedItems = 4; break;
                case 3: maxExpandedItems = 3; break;
                case 4: maxExpandedItems = 1; break;
                };
            }
            else
            {
                switch(b)
                {
                case 1: maxExpandedItems = 4; break;
                case 2: maxExpandedItems = 3; break;
                case 3: maxExpandedItems = 3; break;
                case 4: maxExpandedItems = 1; break;
                };
            };
        };

        if(maxExpandedItems == 0)
        {
            openBankTreeItems.first()->setExpanded(false);
            maxExpandedItems = 1;
        };
        if(c > maxExpandedItems)
        {
            openPatchTreeItems.first()->setExpanded(false);
        };
    };
}

QTreeWidget* bankTreeList::newTreeList()
{
    QTreeWidget *newTreeList = new QTreeWidget();
    newTreeList->setColumnCount(1);
    newTreeList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    newTreeList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QStringList headers;
    headers << tr("Double-click tree item to load patch");
    newTreeList->setHeaderLabels(headers);
    
    QTreeWidgetItem *temp = new QTreeWidgetItem(newTreeList);
    temp->setText(0, "Temp");


    QTreeWidgetItem *user = new QTreeWidgetItem(newTreeList);
    user->setText(0, "User");
    user->setWhatsThis(0, "User Banks");
    //user->setIcon(...);

    QList<QTreeWidgetItem *> userBankRanges;
    for (int a=1; a<=bankTotalUser; a++)
    {
        QTreeWidgetItem* bankRange = new QTreeWidgetItem; // don't pass a parent here!
        if (deviceType == "GT-6B")
        {
            if (a<=5)
            {bankRange->setText(0, QString("Bank U1-U5"));}
            else if (a<=10)
            {bankRange->setText(0, QString("Bank U6-U0"));}
            else if (a<=15)
            {bankRange->setText(0, QString("Bank u1-u5"));}
            else
            {bankRange->setText(0, QString("Bank u6-u0"));};
        } else {bankRange->setText(0, QString("Bank U").append(QString::number(a, 10)).append(" - U").append(QString::number(a+4, 10)) );
        };
        bankRange->setWhatsThis(0, "what the ?");

        for (int b=a; b<=(a+4); b++)
        {
            QTreeWidgetItem* bank = new QTreeWidgetItem(bankRange);
            bank->setText(0, QString("Bank ").append(QString::number(b, 10)));
            bank->setWhatsThis(0, "");

            for (int c=1; c<=4; c++)
            {
                QTreeWidgetItem* patch = new QTreeWidgetItem(bank);
                patch->setText(0, QString("Patch ").append(QString::number(c, 10)));
                patch->setWhatsThis(0, "");
            };
        };
        userBankRanges << bankRange;
        a += 4;
    };
    user->addChildren(userBankRanges);


    QTreeWidgetItem *preset = new QTreeWidgetItem(newTreeList);
    preset->setText(0, "Preset");
    preset->setWhatsThis(0, "Preset Banks");

    QList<QTreeWidgetItem *> presetBankRanges;
    for (int a=(bankTotalUser+1); a<=bankTotalAll; a++)
    {
        QTreeWidgetItem* bankRange = new QTreeWidgetItem; // don't pass a parent here!
        bankRange->setText(0, QString("Bank P").append(QString::number(a, 10)).append("-P").append(QString::number(a+4, 10)) );
        bankRange->setWhatsThis(0, "");

        for (int b=a; b<=(a+4); b++)
        {
            QTreeWidgetItem* bank = new QTreeWidgetItem(bankRange);
            bank->setText(0, QString("Bank ").append(QString::number(b, 10)));
            bank->setWhatsThis(0, "");

            for (int c=1; c<=4; c++)
            {
                QTreeWidgetItem* patch = new QTreeWidgetItem(bank);
                patch->setText(0, QString("Patch ").append(QString::number(c, 10)));
                patch->setWhatsThis(0, "");
            };
        };
        presetBankRanges << bankRange;
        a += 4;
    };
    preset->addChildren(presetBankRanges);

    newTreeList->setExpanded(newTreeList->model()->index(1, 0), true);
    newTreeList->setExpanded(newTreeList->model()->index(2, 0), true);
    return newTreeList;
}

/*********************** setItemClicked() ***********************************
 * Expands and colapses on a single click and sets patch sellection.
 ****************************************************************************/
void bankTreeList::setItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if(item->childCount() != 0 && item->text(0) != "Temp")
    {
        if(item->isExpanded())
        {
            item->setExpanded(false);
        }
        else
        {
            item->setExpanded(true);
        };
    }
    else if (item->childCount() == 0 && item->text(0) != "Temp")
    {
        SysxIO *sysxIO = SysxIO::Instance();
        if(/*sysxIO->isConnected() && */sysxIO->deviceReady())
        {
            bool ok;
            int bank = item->parent()->text(0).section(" ", 1, 1).trimmed().toInt(&ok, 10);
            int patch = item->parent()->indexOfChild(item) + 1;
            emit patchSelectSignal(bank, patch);
            sysxIO->requestPatchChange(bank, patch); // extra to try patch change
            //sysxIO->setRequestName(item->text(0));	// Set the name of the patch we have sellected in case we load it.
        };
    }
    else
    {
        emit patchSelectSignal(0, 0);
    };
}

/*********************** setItemDoubleClicked() *****************************
 * Handels when a patch is double clicked in the tree list. Patch will be
 * loaded into the temp buffer and will tell to request the data afterwards.
 ****************************************************************************/
void bankTreeList::setItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    SysxIO *sysxIO = SysxIO::Instance();
    if(item->childCount() == 0  && item->text(0) != "Temp" && sysxIO->deviceReady() && sysxIO->isConnected())
        // Make sure it's a patch (Patches are the last in line so no children).
    {
        emit setStatusSymbol(2);
        emit setStatusMessage(tr("Patch request"));

        sysxIO->setDeviceReady(false);
        //sysxIO->setRequestName(item->text(0));	// Set the name of the patch we are going to load, so we can check if we have loaded the correct patch at the end.

        bool ok;
        int bank = item->parent()->text(0).section(" ", 1, 1).trimmed().toInt(&ok, 10); // Get the bank
        int patch = item->parent()->indexOfChild(item) + 1;								// and the patch number.

        requestPatch(bank, patch);
    }
    if(item->text(0) == "Temp")
    {  requestPatch(); };
}
/*********************** requestPatch() *******************************
 * Does the actual requesting of the patch data and hands the
 * reception over to updatePatch function.
 **********************************************************************/
void bankTreeList::requestPatch() 
{
    SysxIO *sysxIO = SysxIO::Instance();

    QObject::disconnect(sysxIO, SIGNAL(isChanged()),
                        this, SLOT(requestPatch()));

    if(sysxIO->isConnected())
    {
        QObject::connect(sysxIO, SIGNAL(sysxReply(QString)),	// Connect the result of the request
                         this, SLOT(updatePatch(QString)));					// to updatePatch function.

        emit setStatusSymbol(3);
        emit setStatusMessage(tr("Receiving Patch"));
        sysxIO->requestPatch(0, 0);
    };
}

void bankTreeList::requestPatch(int bank, int patch) 
{
    SysxIO *sysxIO = SysxIO::Instance();
    if(sysxIO->isConnected())
    {
        QObject::connect(sysxIO, SIGNAL(sysxReply(QString)),	// Connect the result of the request
                         this, SLOT(updatePatch(QString)));					// to updatePatch function.

        emit setStatusSymbol(3);
        emit setStatusMessage(tr("Receiving Patch"));

        sysxIO->requestPatch(bank, patch);
    };
}

/*********************** updatePatch() *******************************
 * Updates the source of the currently handeled patch and set the
 * attributes accordingly.
 *********************************************************************/
void bankTreeList::updatePatch(QString replyMsg)
{
    SysxIO *sysxIO = SysxIO::Instance();

    sysxIO->setDeviceReady(true); // Free the device after finishing interaction.

    QObject::disconnect(sysxIO, SIGNAL(sysxReply(QString)),
                        this, SLOT(updatePatch(QString)));

    replyMsg = replyMsg.remove(" ").toUpper();
    if(replyMsg.size()/2 == patchSize) // cjw
    {
        QByteArray data;
        QFile file(":default.syx");   // Read the default GT-3 sysx file so we don't start empty handed.
        if (file.open(QIODevice::ReadOnly))
        {	data = file.readAll(); };
        QByteArray temp;
        temp = data.mid(650, 228);           // copy patch description from default.syx  address 00 16 00 00

        QString sysxBuffer;
        for(int i=0;i<temp.size();i++)
        {
            unsigned char byte = (char)temp[i];
            unsigned int n = (int)byte;
            QString hex = QString::number(n, 16).toUpper();     // convert QByteArray to QString
            if (hex.length() < 2) hex.prepend("0");
            sysxBuffer.append(hex);
        };
        replyMsg.append(sysxBuffer);

        QString area = "Structure";
        sysxIO->setFileSource(area, replyMsg);		// Set the source to the data received.
        sysxIO->setFileName(tr("Patch from ") + deviceType);	// Set the file name to GT-3 patch for the display.
        sysxIO->setDevice(true);				// Patch received from the device so this is set to true.
        sysxIO->setSyncStatus(true);			// We can't be more in sync than right now! :)
        sysxIO->setLoadedBank(sysxIO->getBank());
        sysxIO->setLoadedPatch(sysxIO->getPatch());
        emit updateSignal();
        emit setStatusProgress(0);
    }
    else if(!replyMsg.isEmpty() && replyMsg.size()/2 != patchSize) // cjw
    {
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setWindowTitle(QObject::tr("Warning - Patch data received is incomplete!"));
        msgBox->setIcon(QMessageBox::Warning);
        msgBox->setTextFormat(Qt::RichText);
        QString msgText;
        msgText.append("<font size='+1'><b>");
        msgText.append(QObject::tr("Patch data transfer wrong size"));
        msgText.append("<b></font><br>");
        msgText.append(QObject::tr("Please make sure the ") + deviceType + tr(" is connected correctly and re-try."));
        msgBox->setText(msgText);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->exec();
        /* END WARNING */
    }
    else if(replyMsg.isEmpty()) // cjw
    {
        /* NO-REPLY WARNING */
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setWindowTitle(QObject::tr("Warning - No patch data received!"));
        msgBox->setIcon(QMessageBox::Warning);
        msgBox->setTextFormat(Qt::RichText);
        QString msgText;
        msgText.append("<font size='+1'><b>");
        msgText.append(QObject::tr("Patch data transfer failed, are the correct midi ports selected?"));
        msgText.append("<b></font><br>");

        msgText.append(QObject::tr("Please make sure the ") + deviceType + tr(" is connected correctly and re-try."));
        msgBox->setText(msgText);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->exec();
        /* END WARNING */
    };
}

/********************************** connectedSignal() ****************************
* This slot reloads all patch names of expanded items, if any, on (re)connection to a
* device.
*********************************************************************************/
void bankTreeList::connectedSignal()
{
    //requestPatch();
}

/********************************** updateTree() ********************************
* This handles whether we add the newly expanded item to the current job or 
* start a new one.
*********************************************************************************/
void bankTreeList::updateTree(QTreeWidgetItem *item)
{
    Q_UNUSED(item);
    SysxIO *sysxIO = SysxIO::Instance();
    if(sysxIO->deviceReady() && sysxIO->isConnected())
    {
        sysxIO->setDeviceReady(false);

        this->listIndex = 0;
        this->itemIndex = 0;

        QObject::disconnect(sysxIO, SIGNAL(patchName(QString)),
                            this, SLOT(updatePatchNames(QString)));
        QObject::connect(sysxIO, SIGNAL(patchName(QString)),
                         this, SLOT(updatePatchNames(QString)));

        //this->currentPatchTreeItems.append(item);  //3 lines of mods added below
        this->currentPatchTreeItems.clear();
        this->currentPatchTreeItems = this->openPatchTreeItems;
        //qSort(this->currentPatchTreeItems);
        this->updatePatchNames("");
    };
}

/***************************** updatePatchNames() ********************************
* This updates the patch names in the treeList of all expanded items. this is 
* done when a bank is expanded or when we (re)connect to a device.
*********************************************************************************/
void bankTreeList::updatePatchNames(QString name)
{		SysxIO *sysxIO = SysxIO::Instance();
        if(!name.isEmpty() && sysxIO->isConnected()) //  If not empty we can assume that we did receive a patch name.
        {
            this->currentPatchTreeItems.at(listIndex)->child(itemIndex)->setText(0, name); // Set the patch name of the item in the tree list.
            if(itemIndex >= patchPerBank - 1) // If we reach the last patch in this bank we need to increment the bank and restart at patch 1.
            {
                this->listIndex++;
                this->itemIndex = 0;
            }
            else
            {
                this->itemIndex++;
            };
        };

            if(listIndex < currentPatchTreeItems.size()) // As long as we have items in the list we continue, duh! :)
            {
                bool ok;
                int bank = this->currentPatchTreeItems.at(listIndex)->text(0).section(" ", 1, 1).trimmed().toInt(&ok, 10);
                int patch = itemIndex + 1 ;
                sysxIO->requestPatchName(bank, patch); // The patch name request.

                if(sysxIO->isConnected())
                {
                    emit setStatusSymbol(3);
                    emit setStatusMessage(tr("Receiving names"));
                }
                else
                {
                    sysxIO->setDeviceReady(true);

                    this->currentPatchTreeItems.clear(); // We are done so we can safely reset items that need to be named.
                    this->listIndex = 0;
                    this->itemIndex = 0;

                    emit setStatusSymbol(1);
                    emit setStatusMessage(tr("Ready"));
                    emit setStatusProgress(0);
                };
            }
            else {SysxIO *sysxIO = SysxIO::Instance();
                sysxIO->setDeviceReady(true);
                emit setStatusSymbol(1);
                emit setStatusMessage(tr("Ready"));
            };
}
