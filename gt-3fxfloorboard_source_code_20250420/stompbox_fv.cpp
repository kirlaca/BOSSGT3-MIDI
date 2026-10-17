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

#include "stompbox_fv.h"

stompbox_fv::stompbox_fv(QWidget *parent)
    : stompBox(parent)
{
	/* VOLUME */
	setImage(":/images/fv.png");
	setLSB("0D", "00");
//	setButton("0C", "00", "00", QPoint(0, 109), ":/images/pedal.png");
//editDetails()->patchPos(794, 54, "0D", "00");
	setEditPages();
};

void stompbox_fv::updateSignal()
{
//	updateButton("0C", "00", "00");
};

void stompbox_fv::setEditPages()
{
  editDetails()->page()->newGroupBox("Foot Volume");
	editDetails()->page()->addKnob(0, 0, 1, 1, "0A", "00", "06");         //foot volume level
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	
	editDetails()->page()->newGroupBox("Master");
	editDetails()->page()->addKnob(0, 0, 1, 1, "0A", "00", "00");         //Master level
	editDetails()->page()->addKnob(0, 1, 1, 1, "0A", "00", "01");         //Master bpm
	editDetails()->page()->addGroupBox(0, 2, 1, 2);
	
	editDetails()->page()->newGroupBox("Expression Pedal");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "0D", "00", "00");    //on/off
	editDetails()->page()->addTarget(0, 1, 1, 1, "0D", "00", "01", "target", "03", "05");//target
	editDetails()->page()->addGroupBox(1, 0, 1, 4);
	
	editDetails()->page()->newGroupBox("CTL Pedal");	
	editDetails()->page()->addSwitch(0, 0, 1, 1, "0E", "00", "00");    //on/off
	editDetails()->page()->addTarget(0, 1, 1, 1, "0E", "00", "01", "target", "03", "05");//target
	editDetails()->page()->addComboBox(0, 4, 1, 1, "0E", "00", "07");         // source mode
	editDetails()->page()->addGroupBox(2, 0, 1, 4);
	editDetails()->addPage();  // PAGE ABOVE   
	
};
