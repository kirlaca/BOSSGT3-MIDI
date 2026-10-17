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

#include "stompbox_dd.h"
#include "MidiTable.h"
#include "SysxIO.h"

#include "customButton.h"
#include "customLed.h"
#include "customDial.h"

stompbox_dd::stompbox_dd(QWidget *parent)
    : stompBox(parent)
{
	/* DELAY */
	setImage(":/images/dd.png");
	setLSB("07", "00");
	setKnob1("07", "00", "05");
	setKnob2("07", "00", "07");
	setComboBox("07", "00", "01");
	setButton("07", "00", "00");
	editDetails()->patchPos(544, 16, "07", "00");
	setEditPages();
};

void stompbox_dd::updateSignal()
{
	updateKnob1("07", "00", "05");
	updateKnob2("07", "00", "07");
	updateComboBox("07", "00", "01");
	updateButton("07", "00", "00");
};

void stompbox_dd::setEditPages()
{
	
	editDetails()->page()->newGroupBox("Effect");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "07", "00", "00", "middle", Qt::AlignCenter);
	editDetails()->page()->addGroupBox(1, 0, 1, 1);
	editDetails()->page()->newGroupBox("Delay");
	editDetails()->page()->newStackControl(0);
	editDetails()->page()->addComboBox(0, 0, 1, 1, "07", "00", "01");     //delay type
	editDetails()->page()->addStackControl();
	editDetails()->page()->addGroupBox(1, 1, 1, 1);

	
	editDetails()->page()->newGroupBox("Time");
	editDetails()->page()->addKnob(0, 1, 1, 1, "07", "00", "02","normal","right", 60);//delay time
	editDetails()->page()->addKnob(0, 2, 1, 1, "07", "00", "03");         //time fine
	editDetails()->page()->insertStackField(0, 0, 3, 1, 1);
	
	editDetails()->page()->addKnob(0, 4, 1, 1, "07", "00", "05", "turbo");//feedback
    editDetails()->page()->addGroupBox(1, 2, 1, 1);
	

	editDetails()->page()->newGroupBox("Filter");
	editDetails()->page()->addKnob(0, 5, 1, 1, "07", "00", "06");         //high cut
	editDetails()->page()->addGroupBox(1, 3, 1, 1);

	editDetails()->page()->newGroupBox("Output");
	editDetails()->page()->addKnob(0, 6, 1, 1, "07", "00", "07", "turbo");//level
	editDetails()->page()->addGroupBox(1, 4, 1, 1);

	editDetails()->page()->newStackField(0);
	editDetails()->page()->addStackField();
    editDetails()->page()->newStackField(0);
	editDetails()->page()->addKnob(0, 3, 1, 1, "07", "00", "04");         //tap time
	editDetails()->page()->addStackField();
	
	editDetails()->addPage();// PAGE 2 ABOVE


	};
