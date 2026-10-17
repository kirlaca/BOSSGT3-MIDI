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

#include "menuPage_system.h"

menuPage_system::menuPage_system(QWidget *parent)
    : menuPage(parent)
{ 
  setImage(":/images/system_pushbutton.png");
  //setLSB("0B", "00");
	setEditPages();
};

void menuPage_system::updateSignal()
{

};

void menuPage_system::setEditPages()
{
  editDetails()->page()->addLabel(0, 0, 1, 1, tr("***CAUTION*** Settings changes are automatically written to GT-3"));
  editDetails()->page()->newGroupBox("settings");
  editDetails()->page()->addKnob(0, 0, 1, 1, "02", "00", "00", "System01"); // lcd contrast
  editDetails()->page()->addLabel(0, 1, 1, 1, "     ");
	editDetails()->page()->addComboBox(0, 2, 1, 1, "02", "00", "01", "System01"); // dial function 
	editDetails()->page()->addLabel(0, 3, 1, 1, "     ");
	editDetails()->page()->addComboBox(0, 4, 1, 1, "02", "00", "06", "System01"); // assign hold
	editDetails()->page()->addLabel(1, 0, 1, 1, "     ");
	editDetails()->page()->addComboBox(1, 1, 1, 1, "02", "00", "02", "System01"); // sub ctl1
	editDetails()->page()->addLabel(1, 2, 1, 1, "     ");
	editDetails()->page()->addComboBox(1, 3, 1, 1, "02", "00", "03", "System01"); // sub ctl2
	editDetails()->page()->addGroupBox(1, 0, 1, 2);
	
	editDetails()->page()->newGroupBox("Tuner");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "00", "00", "00", "System00"); // tuner pitch
	editDetails()->page()->addLabel(0, 1, 1, 1, "               ");
	editDetails()->page()->addComboBox(0, 2, 1, 1, "00", "00", "01", "System00"); //tuner bypass
	editDetails()->page()->addGroupBox(2, 1, 1, 1);
	
	editDetails()->page()->newGroupBox("Patch Change");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "02", "00", "04", "System01"); // patch change mode
	editDetails()->page()->addKnob(0, 3, 1, 1, "02", "00", "05", "System01"); // bank extent 
	editDetails()->page()->addGroupBox(3, 1, 1, 1);
	
	editDetails()->page()->newGroupBox("Output select");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "00", "00", "00", "System01"); // output select
	editDetails()->page()->addGroupBox(2, 0, 1, 1);
	
	editDetails()->page()->newGroupBox("Global E.Q and NS/Reverb");
	editDetails()->page()->addKnob(0, 1, 1, 1, "00", "00", "01", "System01"); // eq low
	editDetails()->page()->addKnob(0, 2, 1, 1, "00", "00", "02", "System01"); // eq high
	editDetails()->page()->addKnob(0, 3, 1, 1, "00", "00", "03", "System01"); // NS threshold
  editDetails()->page()->addKnob(0, 4, 1, 1, "00", "00", "04", "System01"); // revberb level
	editDetails()->page()->addGroupBox(3, 0, 1, 1);
	
  editDetails()->addPage("19", "00", "01", "00");


	
	editDetails()->page()->newGroupBox(tr("System Midi  ***CAUTION*** Settings changes are automatically written to GT-3"));
	
	editDetails()->page()->addComboBox(0, 0, 1, 1, "03", "00", "00", "System01"); // rx channel
	editDetails()->page()->addComboBox(1, 0, 1, 1, "03", "00", "01", "System01"); // omni mode
	editDetails()->page()->addComboBox(2, 0, 1, 1, "03", "00", "02", "System01"); // tx channel
	editDetails()->page()->addComboBox(3, 0, 1, 1, "03", "00", "03", "System01"); // pc out
	editDetails()->page()->addComboBox(4, 0, 1, 1, "03", "00", "08", "System01"); // map select
		
	editDetails()->page()->addComboBox(0, 1, 1, 1, "03", "00", "04", "System01"); // exp out
	editDetails()->page()->addComboBox(2, 1, 1, 1, "03", "00", "05", "System01"); // ctl out
	editDetails()->page()->addComboBox(3, 1, 1, 1, "03", "00", "06", "System01"); // sub ctl1 out
	editDetails()->page()->addComboBox(4, 1, 1, 1, "03", "00", "07", "System01"); // sub ctl2 out
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->addPage("19", "00", "01", "01");
  
  editDetails()->page()->newGroupBox(tr("MANUAL settings  ***CAUTION*** Settings changes are automatically written to GT-3"));
  editDetails()->page()->addComboBox(1, 0, 1, 1, "00", "00", "00", "System02"); // manual pedal 1
	editDetails()->page()->addComboBox(1, 1, 1, 1, "00", "00", "01", "System02"); // manual pedal 2
	editDetails()->page()->addComboBox(1, 2, 1, 1, "00", "00", "02", "System02"); // manual pedal 3
	editDetails()->page()->addComboBox(1, 3, 1, 1, "00", "00", "03", "System02"); // manual pedal 4
	editDetails()->page()->addComboBox(0, 2, 1, 1, "00", "00", "04", "System02"); // manual bank down
	editDetails()->page()->addComboBox(0, 3, 1, 1, "00", "00", "05", "System02"); // manual bank up
  editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->addPage("19", "00", "01", "02");
  

  
	editDetails()->addPage();
	
};
