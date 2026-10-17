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

#include "stompbox_wah.h"

stompbox_wah::stompbox_wah(QWidget *parent)
    : stompBox(parent)
{
	/* WAH */
	setImage(":/images/wah.png");
	setLSB("02", "00");
	setButton("02", "00", "00", QPoint(0, 109), ":/images/pedal.png");
	setComboBox("02", "00", "01");
	editDetails()->patchPos(140, 24, "02", "00");
	setEditPages();
};

void stompbox_wah::updateSignal()
{
	updateButton("02", "00", "00");
	updateComboBox("02", "00", "01");
};

void stompbox_wah::setEditPages()
{
 	editDetails()->page()->newGroupBox("Effect");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "02", "00", "00", "middle", Qt::AlignCenter);
	editDetails()->page()->newStackControl(0);
	editDetails()->page()->addComboBox(1, 0, 1, 1, "02", "00", "01");
	editDetails()->page()->addStackControl();
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	editDetails()->page()->insertStackField(0, 1, 0, 1, 1);
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Pedal Wah");
	editDetails()->page()->addKnob(0, 1, 1, 1, "02", "00", "02");           //pedal pos
	editDetails()->page()->addKnob(0, 3, 1, 1, "02", "00", "03");           //level
	editDetails()->page()->addGroupBox(1, 0, 1, 3);
	editDetails()->page()->addStackField();   // PAGE 1 ABOVE

	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Auto Wah");
	editDetails()->page()->addComboBox(0, 1, 1, 1, "02", "00", "04");       //mode
	editDetails()->page()->addComboBox(0, 2, 1, 1, "02", "00", "05");       //polarity
	editDetails()->page()->addKnob(0, 3, 1, 1, "02", "00", "06");           //sensitivity
	editDetails()->page()->addKnob(0, 4, 1, 1, "02", "00", "07", "turbo");  //freq
	editDetails()->page()->addKnob(0, 5, 1, 1, "02", "00", "08");           //peak
	editDetails()->page()->addKnob(0, 6, 1, 1, "02", "00", "09");           //rate
  editDetails()->page()->addKnob(0, 7, 1, 1, "02", "00", "0A");           //depth
  editDetails()->page()->addKnob(0, 8, 1, 1, "02", "00", "0B");           //level
	editDetails()->page()->addGroupBox(1, 0, 1, 1);
	editDetails()->page()->addStackField();   // PAGE 2 ABOVE

	editDetails()->addPage();	
};
