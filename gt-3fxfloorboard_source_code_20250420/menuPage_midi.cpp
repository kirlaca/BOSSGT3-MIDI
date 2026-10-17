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

#include "menuPage_midi.h"

menuPage_midi::menuPage_midi(QWidget *parent)
    : menuPage(parent)
{
  setImage(":/images/midi_pushbutton.png");
  //setLSB("0B", "00");
	setEditPages();
};

void menuPage_midi::updateSignal()
{

};

void menuPage_midi::setEditPages()
{ /*
  editDetails()->page()->addLabel(0, 0, 1, 1, "***CAUTION*** Settings changes are automatically written to GT-6");
  editDetails()->page()->newGroupBox("Custom Preamp 1");
  editDetails()->page()->addComboBox(0, 0, 1, 1, "06", "00", "00", "System02"); // preamp type
  editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "01", "System02"); // bottom
  editDetails()->page()->addKnob(0, 2, 1, 1, "06", "00", "02", "System02"); // edge
  editDetails()->page()->addKnob(0, 3, 1, 1, "06", "00", "03", "System02"); // bass freq
  editDetails()->page()->addKnob(0, 4, 1, 1, "06", "00", "04", "System02"); // treb freq
  editDetails()->page()->addKnob(0, 5, 1, 1, "06", "00", "05", "System02"); // pre low
  editDetails()->page()->addKnob(0, 6, 1, 1, "06", "00", "06", "System02"); // pre high
  editDetails()->page()->addKnob(0, 7, 1, 1, "06", "00", "07", "System02"); // spkr low
  editDetails()->page()->addKnob(0, 8, 1, 1, "06", "00", "08", "System02"); // spkr high 
	editDetails()->page()->addGroupBox(1, 0, 1, 2);
	
	editDetails()->page()->newGroupBox("Custom Preamp 2");
  editDetails()->page()->addComboBox(0, 0, 1, 1, "06", "01", "00", "System02"); // preamp type
  editDetails()->page()->addKnob(0, 1, 1, 1, "06", "01", "01", "System02"); // bottom
  editDetails()->page()->addKnob(0, 2, 1, 1, "06", "01", "02", "System02"); // edge
  editDetails()->page()->addKnob(0, 3, 1, 1, "06", "01", "03", "System02"); // bass freq
  editDetails()->page()->addKnob(0, 4, 1, 1, "06", "01", "04", "System02"); // treb freq
  editDetails()->page()->addKnob(0, 5, 1, 1, "06", "01", "05", "System02"); // pre low
  editDetails()->page()->addKnob(0, 6, 1, 1, "06", "01", "06", "System02"); // pre high
  editDetails()->page()->addKnob(0, 7, 1, 1, "06", "01", "07", "System02"); // spkr low
  editDetails()->page()->addKnob(0, 8, 1, 1, "06", "01", "08", "System02"); // spkr high 
	editDetails()->page()->addGroupBox(2, 0, 1, 2);
	
	editDetails()->page()->newGroupBox("Custom Preamp 3");
  editDetails()->page()->addComboBox(0, 0, 1, 1, "06", "02", "00", "System02"); // preamp type
  editDetails()->page()->addKnob(0, 1, 1, 1, "06", "02", "01", "System02"); // bottom
  editDetails()->page()->addKnob(0, 2, 1, 1, "06", "02", "02", "System02"); // edge
  editDetails()->page()->addKnob(0, 3, 1, 1, "06", "02", "03", "System02"); // bass freq
  editDetails()->page()->addKnob(0, 4, 1, 1, "06", "02", "04", "System02"); // treb freq
  editDetails()->page()->addKnob(0, 5, 1, 1, "06", "02", "05", "System02"); // pre low
  editDetails()->page()->addKnob(0, 6, 1, 1, "06", "02", "06", "System02"); // pre high
  editDetails()->page()->addKnob(0, 7, 1, 1, "06", "02", "07", "System02"); // spkr low
  editDetails()->page()->addKnob(0, 8, 1, 1, "06", "02", "08", "System02"); // spkr high 
	editDetails()->page()->addGroupBox(3, 0, 1, 2);

  editDetails()->addPage("19", "00", "02", "00");


	editDetails()->page()->addLabel(0, 0, 1, 1, "***CAUTION*** Settings changes are automatically written to GT-6");
	editDetails()->page()->newGroupBox("Custom Distortion 1");
  editDetails()->page()->addComboBox(0, 0, 1, 1, "07", "00", "00", "System02"); // dist type
  editDetails()->page()->addKnob(0, 1, 1, 1, "07", "00", "01", "System02"); // bottom
  editDetails()->page()->addKnob(0, 2, 1, 1, "07", "00", "02", "System02"); // top
  editDetails()->page()->addKnob(0, 3, 1, 1, "07", "00", "03", "System02"); // low
  editDetails()->page()->addKnob(0, 4, 1, 1, "07", "00", "04", "System02"); // high
  editDetails()->page()->addGroupBox(1, 0, 1, 2);
  
  editDetails()->page()->newGroupBox("Custom Distortion 2");
  editDetails()->page()->addComboBox(0, 0, 1, 1, "07", "01", "00", "System02"); // dist type
  editDetails()->page()->addKnob(0, 1, 1, 1, "07", "01", "01", "System02"); // bottom
  editDetails()->page()->addKnob(0, 2, 1, 1, "07", "01", "02", "System02"); // top
  editDetails()->page()->addKnob(0, 3, 1, 1, "07", "01", "03", "System02"); // low
  editDetails()->page()->addKnob(0, 4, 1, 1, "07", "01", "04", "System02"); // high
  editDetails()->page()->addGroupBox(2, 0, 1, 2);
  
  editDetails()->addPage("19", "00", "02", "01");
  
  editDetails()->page()->addLabel(0, 0, 1, 1, "***CAUTION*** Settings changes are automatically written to GT-6");
	editDetails()->page()->newGroupBox("Custom Wah 1");
  editDetails()->page()->addComboBox(0, 0, 1, 1, "08", "00", "00", "System02"); // wah type
  editDetails()->page()->addKnob(0, 1, 1, 1, "08", "00", "01", "System02"); // Q
  editDetails()->page()->addKnob(0, 2, 1, 1, "08", "00", "02", "System02"); // range low
  editDetails()->page()->addKnob(0, 3, 1, 1, "08", "00", "03", "System02"); // range high
  editDetails()->page()->addKnob(0, 4, 1, 1, "08", "00", "04", "System02"); // presence
  editDetails()->page()->addGroupBox(1, 0, 1, 2);
  
  editDetails()->page()->newGroupBox("Custom Wah 2");
  editDetails()->page()->addComboBox(0, 0, 1, 1, "08", "01", "00", "System02"); // wah type
  editDetails()->page()->addKnob(0, 1, 1, 1, "08", "01", "01", "System02"); // Q
  editDetails()->page()->addKnob(0, 2, 1, 1, "08", "01", "02", "System02"); // range low
  editDetails()->page()->addKnob(0, 3, 1, 1, "08", "01", "03", "System02"); // range high
  editDetails()->page()->addKnob(0, 4, 1, 1, "08", "01", "04", "System02"); // presence
  editDetails()->page()->addGroupBox(2, 0, 1, 2);
  
  editDetails()->page()->newGroupBox("Custom Wah 3");
  editDetails()->page()->addComboBox(0, 0, 1, 1, "08", "02", "00", "System02"); // wah type
  editDetails()->page()->addKnob(0, 1, 1, 1, "08", "02", "01", "System02"); // Q
  editDetails()->page()->addKnob(0, 2, 1, 1, "08", "02", "02", "System02"); // range low
  editDetails()->page()->addKnob(0, 3, 1, 1, "08", "02", "03", "System02"); // range high
  editDetails()->page()->addKnob(0, 4, 1, 1, "08", "02", "04", "System02"); // presence
  editDetails()->page()->addGroupBox(3, 0, 1, 2);
  
  editDetails()->addPage("19", "00", "02", "02");
  */
	editDetails()->addPage();
	

};
