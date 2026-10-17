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

#include "stompbox_eq.h"

stompbox_eq::stompbox_eq(QWidget *parent)
    : stompBox(parent)
{
	/* EQ */
	setImage(":/images/eq.png");
  setLSB("05", "00");
	setSlider1("05", "00", "01");  //low
	setSlider2("05", "00", "04");  //low mid
	setSlider3("05", "00", "07");  // hi mid
	setSlider4("05", "00", "08");   // high
	setSlider5("05", "00", "09");   // level
	setButton("05", "00", "00"); 
	editDetails()->patchPos(274, 20, "05", "00");
	setEditPages();
};

void stompbox_eq::updateSignal()
{
	updateSlider1("05", "00", "01");
	updateSlider2("05", "00", "04");
	updateSlider3("05", "00", "07");
	updateSlider4("05", "00", "08");
	updateSlider5("05", "00", "09");
	updateButton("05", "00", "00");
};

void stompbox_eq::setEditPages()
{
    editDetails()->page()->newGroupBox("Effect");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "05", "00", "00", "middle", Qt::AlignCenter);        //on/off
	editDetails()->page()->addGroupBox(1, 0, 1, 1);
	
	editDetails()->page()->newGroupBox("Low EQ");
	editDetails()->page()->addKnob(2, 2, 1, 1, "05", "00", "01", "turbo"); //level
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	
	editDetails()->page()->newGroupBox("Low Mid EQ");
	editDetails()->page()->addKnob(2, 0, 1, 1, "05", "00", "02", "turbo"); //Q
	editDetails()->page()->addKnob(2, 1, 1, 1, "05", "00", "03");          //freq
	editDetails()->page()->addKnob(2, 2, 1, 1, "05", "00", "04", "turbo"); //level
	editDetails()->page()->addGroupBox(0, 1, 1, 1);
	
	editDetails()->page()->newGroupBox("High Mid EQ");
	editDetails()->page()->addKnob(1, 0, 1, 1, "05", "00", "05", "turbo"); //Q
	editDetails()->page()->addKnob(1, 1, 1, 1, "05", "00", "06");          //freq
	editDetails()->page()->addKnob(1, 2, 1, 1, "05", "00", "07", "turbo"); //level
	editDetails()->page()->addGroupBox(0, 2, 1, 1);

	editDetails()->page()->newGroupBox("High EQ");
	editDetails()->page()->addKnob(0, 2, 1, 1, "05", "00", "08", "turbo"); //level
	editDetails()->page()->addGroupBox(0, 3, 1, 1);

	editDetails()->page()->newGroupBox("Output");
	editDetails()->page()->addKnob(2, 0, 1, 1, "05", "00", "09");          //level
	editDetails()->page()->addGroupBox(1, 1, 1, 3);
	editDetails()->addPage();  // PAGE 1 ABOVE 

};
