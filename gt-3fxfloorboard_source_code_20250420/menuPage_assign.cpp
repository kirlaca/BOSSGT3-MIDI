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

#include "menuPage_assign.h"

menuPage_assign::menuPage_assign(QWidget *parent)
    : menuPage(parent)
{
  setImage(":/images/system_pushbutton.png");
  setLSB("0F", "00");
	setEditPages();
};

void menuPage_assign::updateSignal()
{

};

void menuPage_assign::setEditPages()
{
  editDetails()->page()->newGroupBox(tr("Assigns"));
	editDetails()->page()->newGroupBox(tr("Assign 1-8"));
	editDetails()->page()->newStackControl(0);
	editDetails()->page()->addComboBox(0, 0, 1, 1, "19", "00", "00");	
	editDetails()->page()->addStackControl();
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	editDetails()->page()->newGroupBox("Assign 1");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "0F", "00", "00");
	editDetails()->page()->addGroupBox(0, 1, 1, 1);
	editDetails()->page()->newGroupBox("Assign 2");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "10", "00", "00");
	editDetails()->page()->addGroupBox(0, 2, 1, 1);
	editDetails()->page()->newGroupBox("Assign 3");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "11", "00", "00");
	editDetails()->page()->addGroupBox(0, 3, 1, 1);
	editDetails()->page()->newGroupBox("Assign 4");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "12", "00", "00");
	editDetails()->page()->addGroupBox(0, 4, 1, 1);
	editDetails()->page()->newGroupBox("Assign 5");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "13", "00", "00");
	editDetails()->page()->addGroupBox(0, 5, 1, 1);
	editDetails()->page()->newGroupBox("Assign 6");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "14", "00", "00");
	editDetails()->page()->addGroupBox(0, 6, 1, 1);
	editDetails()->page()->newGroupBox("Assign 7");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "15", "00", "00");
	editDetails()->page()->addGroupBox(0, 7, 1, 1);
	editDetails()->page()->newGroupBox("Assign 8");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "16", "00", "00");
	editDetails()->page()->addGroupBox(0, 8, 1, 1);
	
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	editDetails()->page()->newGroupBox("");
	editDetails()->page()->addLabel(0, 0, 1, 1, "                             ");
	editDetails()->page()->insertStackField(0, 0, 1, 1, 1);
	editDetails()->page()->addGroupBox(1, 0, 1, 1);	
	editDetails()->addPage();

 
  editDetails()->page()->newStackField(0);
  editDetails()->page()->newGroupBox("Assign 1");
	editDetails()->page()->addTarget(0, 0, 1, 3, "0F", "00", "01", "target","03", "05"); // Target
	editDetails()->page()->newStackControl(1);
	editDetails()->page()->addComboBox(1, 0, 1, 1, "0F", "00", "07");                    // Source
	editDetails()->page()->addStackControl();
	editDetails()->page()->addLabel(1, 1, 1, 1, "          ");
  editDetails()->page()->addComboBox(1, 2, 1, 1, "0F", "00", "08");                    // Source Mode
  editDetails()->page()->addRange(2, 1, 1, 2, "0F", "00", "09", "Structure");         // min/max range widget
	editDetails()->page()->addGroupBox(1, 0, 1, 1);
  editDetails()->page()->insertStackField(1, 1, 2, 1, 1);
	editDetails()->page()->addStackField();
	
	editDetails()->page()->newStackField(1);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(1);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(1);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(1);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(1);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(1);
  editDetails()->page()->newGroupBox("Assign 1 Internal Pedal");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "0F", "00", "0B");                    // Internal Pedal Trigger
  editDetails()->page()->addKnob(2, 0, 1, 1, "0F", "00", "0C");                        // Internal Pedal Time
	editDetails()->page()->addComboBox(4, 0, 1, 1, "0F", "00", "0D");                    // Internal Pedal Curve
	editDetails()->page()->addGroupBox(0, 0, 3, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(1);
  editDetails()->page()->newGroupBox("Assign 1 Wave Pedal");
	editDetails()->page()->addKnob(0, 0, 1, 1, "0F", "00", "0E", "normal","right", 105);// Wave Form Rate
	editDetails()->page()->addComboBox(1, 0, 1, 1, "0F", "00", "0F");                    // Wave Form
	editDetails()->page()->addGroupBox(0, 0, 3, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(1);editDetails()->page()->addStackField();
	
	
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Assign 2");
	editDetails()->page()->addTarget(0, 0, 1, 3, "10", "00", "01", "target","03", "05"); // Target
	editDetails()->page()->newStackControl(2);
	editDetails()->page()->addComboBox(1, 0, 1, 1, "10", "00", "07");                    // Source
	editDetails()->page()->addStackControl();
	editDetails()->page()->addLabel(1, 1, 1, 1, "          ");
  editDetails()->page()->addComboBox(1, 2, 1, 1, "10", "00", "08");                    // Source Mode
  editDetails()->page()->addRange(2, 1, 1, 2, "10", "00", "09", "Structure");         // min/max range widget
	editDetails()->page()->addGroupBox(1, 0, 1, 1);
  editDetails()->page()->insertStackField(2, 1, 2, 1, 1);
	editDetails()->page()->addStackField();
	
	editDetails()->page()->newStackField(2);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(2);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(2);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(2);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(2);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(2);
  editDetails()->page()->newGroupBox("Assign 2 Internal Pedal");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "10", "00", "0B");                    // Internal Pedal Trigger
  editDetails()->page()->addKnob(1, 0, 1, 1, "10", "00", "0C");                        // Internal Pedal Time
	editDetails()->page()->addComboBox(2, 0, 1, 1, "10", "00", "0D");                    // Internal Pedal Curve
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(2);
  editDetails()->page()->newGroupBox("Assign 2 Wave Pedal");
	editDetails()->page()->addKnob(0, 0, 1, 1, "10", "00", "0E", "normal","right", 105);// Wave Form Rate
	editDetails()->page()->addComboBox(1, 0, 1, 1, "10", "00", "0F");                    // Wave Form
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(2);editDetails()->page()->addStackField();
	
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Assign 3");
	editDetails()->page()->addTarget(0, 0, 1, 3, "11", "00", "01", "target","03", "05"); // Target
	editDetails()->page()->newStackControl(3);
	editDetails()->page()->addComboBox(1, 0, 1, 1, "11", "00", "07");                    // Source
	editDetails()->page()->addStackControl();
	editDetails()->page()->addLabel(1, 1, 1, 1, "          ");
  editDetails()->page()->addComboBox(1, 2, 1, 1, "11", "00", "08");                    // Source Mode
  editDetails()->page()->addRange(2, 1, 1, 2, "11", "00", "09", "Structure");         // min/max range widget
	editDetails()->page()->addGroupBox(1, 0, 1, 1);
  editDetails()->page()->insertStackField(3, 1, 2, 1, 1);
	editDetails()->page()->addStackField();
	
	editDetails()->page()->newStackField(3);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(3);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(3);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(3);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(3);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(3);
  editDetails()->page()->newGroupBox("Assign 3 Internal Pedal");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "11", "00", "0B");                    // Internal Pedal Trigger
  editDetails()->page()->addKnob(1, 0, 1, 1, "11", "00", "0C");                        // Internal Pedal Time
	editDetails()->page()->addComboBox(2, 0, 1, 1, "11", "00", "0D");                    // Internal Pedal Curve
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(3);
  editDetails()->page()->newGroupBox("Assign 3 Wave Pedal");
	editDetails()->page()->addKnob(0, 0, 1, 1, "11", "00", "0E", "normal","right", 105);// Wave Form Rate
	editDetails()->page()->addComboBox(1, 0, 1, 1, "11", "00", "0F");                    // Wave Form
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(3);editDetails()->page()->addStackField();
	
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Assign 4");
	editDetails()->page()->addTarget(0, 0, 1, 3, "12", "00", "01", "target","03", "05"); // Target
	editDetails()->page()->newStackControl(4);
	editDetails()->page()->addComboBox(1, 0, 1, 1, "12", "00", "07");                    // Source
	editDetails()->page()->addStackControl();
	editDetails()->page()->addLabel(1, 1, 1, 1, "          ");
  editDetails()->page()->addComboBox(1, 2, 1, 1, "12", "00", "08");                    // Source Mode
  editDetails()->page()->addRange(2, 1, 1, 2, "12", "00", "09", "Structure");         // min/max range widget
	editDetails()->page()->addGroupBox(1, 0, 1, 1);
  editDetails()->page()->insertStackField(4, 1, 2, 1, 1);
	editDetails()->page()->addStackField();
	
	editDetails()->page()->newStackField(4);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(4);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(4);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(4);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(4);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(4);
  editDetails()->page()->newGroupBox("Assign 4 Internal Pedal");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "12", "00", "0B");                    // Internal Pedal Trigger
  editDetails()->page()->addKnob(1, 0, 1, 1, "12", "00", "0C");                        // Internal Pedal Time
	editDetails()->page()->addComboBox(2, 0, 1, 1, "12", "00", "0D");                    // Internal Pedal Curve
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(4);
  editDetails()->page()->newGroupBox("Assign 4 Wave Pedal");
	editDetails()->page()->addKnob(0, 0, 1, 1, "12", "00", "0E", "normal","right", 105);// Wave Form Rate
	editDetails()->page()->addComboBox(1, 0, 1, 1, "12", "00", "0F");                    // Wave Form
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
  editDetails()->page()->newStackField(4);editDetails()->page()->addStackField();
  
		
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Assign 5");
	editDetails()->page()->addTarget(0, 0, 1, 3, "13", "00", "01", "target","03", "05"); // Target
	editDetails()->page()->newStackControl(5);
	editDetails()->page()->addComboBox(1, 0, 1, 1, "13", "00", "07");                    // Source
	editDetails()->page()->addStackControl();
	editDetails()->page()->addLabel(1, 1, 1, 1, "          ");
  editDetails()->page()->addComboBox(1, 1, 1, 1, "13", "00", "08");                    // Source Mode
  editDetails()->page()->addRange(2, 1, 1, 2, "13", "00", "09", "Structure");         // min/max range widget
	editDetails()->page()->addGroupBox(1, 0, 1, 1);
  editDetails()->page()->insertStackField(5, 1, 2, 1, 1);
	editDetails()->page()->addStackField();
	
	editDetails()->page()->newStackField(5);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(5);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(5);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(5);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(5);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(5);
  editDetails()->page()->newGroupBox("Assign 5 Internal Pedal");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "13", "00", "0B");                    // Internal Pedal Trigger
  editDetails()->page()->addKnob(1, 0, 1, 1, "13", "00", "0C");                        // Internal Pedal Time
	editDetails()->page()->addComboBox(2, 0, 1, 1, "13", "00", "0D");                    // Internal Pedal Curve
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(5);
  editDetails()->page()->newGroupBox("Assign 5 Wave Pedal");
	editDetails()->page()->addKnob(0, 0, 1, 1, "13", "00", "0E", "normal","right", 105);// Wave Form Rate
	editDetails()->page()->addComboBox(1, 0, 1, 1, "13", "00", "0F");                    // Wave Form
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(5);editDetails()->page()->addStackField();
	
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Assign 6");
	editDetails()->page()->addTarget(0, 0, 1, 3, "14", "00", "01", "target","03", "05"); // Target
	editDetails()->page()->newStackControl(6);
	editDetails()->page()->addComboBox(1, 0, 1, 1, "14", "00", "07");                    // Source
	editDetails()->page()->addStackControl();
	editDetails()->page()->addLabel(1, 1, 1, 1, "          ");
  editDetails()->page()->addComboBox(1, 2, 1, 1, "14", "00", "08");                    // Source Mode
  editDetails()->page()->addRange(2, 1, 1, 2, "14", "00", "09", "Structure");         // min/max range widget
	editDetails()->page()->addGroupBox(1, 0, 1, 1);
  editDetails()->page()->insertStackField(6, 1, 2, 1, 1);
	editDetails()->page()->addStackField();
	
  editDetails()->page()->newStackField(6);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(6);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(6);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(6);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(6);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(6);
  editDetails()->page()->newGroupBox("Assign 6 Internal Pedal");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "14", "00", "0B");                    // Internal Pedal Trigger
  editDetails()->page()->addKnob(1, 0, 1, 1, "14", "00", "0C");                        // Internal Pedal Time
	editDetails()->page()->addComboBox(2, 0, 1, 1, "14", "00", "0D");                    // Internal Pedal Curve
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(6);
  editDetails()->page()->newGroupBox("Assign 6 Wave Pedal");
	editDetails()->page()->addKnob(0, 0, 1, 1, "14", "00", "0E", "normal","right", 105);// Wave Form Rate
	editDetails()->page()->addComboBox(1, 0, 1, 1, "14", "00", "0F");                    // Wave Form
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(6);editDetails()->page()->addStackField();
	  

	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Assign 7");
	editDetails()->page()->addTarget(0, 0, 1, 3, "15", "00", "01", "target","03", "05"); // Target
	editDetails()->page()->newStackControl(7);
	editDetails()->page()->addComboBox(1, 0, 1, 1, "15", "00", "07");                    // Source
	editDetails()->page()->addStackControl();
	editDetails()->page()->addLabel(1, 1, 1, 1, "          ");
  editDetails()->page()->addComboBox(1, 2, 1, 1, "15", "00", "08");                    // Source Mode
  editDetails()->page()->addRange(2, 1, 1, 2, "15", "00", "09", "Structure");         // min/max range widget
	editDetails()->page()->addGroupBox(1, 0, 1, 1);
  editDetails()->page()->insertStackField(7, 1, 2, 1, 1);
	editDetails()->page()->addStackField();
	
	editDetails()->page()->newStackField(7);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(7);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(7);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(7);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(7);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(7);
  editDetails()->page()->newGroupBox("Assign 7 Internal Pedal");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "15", "00", "0B");                    // Internal Pedal Trigger
  editDetails()->page()->addKnob(1, 0, 1, 1, "15", "00", "0C");                        // Internal Pedal Time
	editDetails()->page()->addComboBox(2, 0, 1, 1, "15", "00", "0D");                    // Internal Pedal Curve
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(7);
  editDetails()->page()->newGroupBox("Assign 7 Wave Pedal");
	editDetails()->page()->addKnob(0, 0, 1, 1, "15", "00", "0E", "normal","right", 105);// Wave Form Rate
	editDetails()->page()->addComboBox(1, 0, 1, 1, "15", "00", "0F");                    // Wave Form
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(7);editDetails()->page()->addStackField();
	
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Assign 8");
	editDetails()->page()->addTarget(0, 0, 1, 3, "16", "00", "01", "target","03", "05"); // Target
	editDetails()->page()->newStackControl(8);
	editDetails()->page()->addComboBox(1, 0, 1, 1, "16", "00", "07");                    // Source
	editDetails()->page()->addStackControl();
	editDetails()->page()->addLabel(1, 1, 1, 1, "          ");
  editDetails()->page()->addComboBox(1, 2, 1, 1, "16", "00", "08");                    // Source Mode
  editDetails()->page()->addRange(2, 1, 1, 2, "16", "00", "09", "Structure");         // min/max range widget
	editDetails()->page()->addGroupBox(1, 0, 1, 1);
  editDetails()->page()->insertStackField(8, 1, 2, 1, 1);
  editDetails()->page()->addStackField();
  
  editDetails()->page()->newStackField(8);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(8);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(8);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(8);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(8);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(8);
  editDetails()->page()->newGroupBox("Assign 8 Internal Pedal");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "16", "00", "0B");                    // Internal Pedal Trigger
  editDetails()->page()->addKnob(1, 0, 1, 1, "16", "00", "0C");                        // Internal Pedal Time
	editDetails()->page()->addComboBox(2, 0, 1, 1, "16", "00", "0D");                    // Internal Pedal Curve
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(8);
  editDetails()->page()->newGroupBox("Assign 8 Wave Pedal");
	editDetails()->page()->addKnob(0, 0, 1, 1, "16", "00", "0E", "normal","right", 105);// Wave Form Rate
	editDetails()->page()->addComboBox(1, 0, 1, 1, "16", "00", "0F");                    // Wave Form
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
  editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(8);editDetails()->page()->addStackField();
  
};
