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

#include "stompbox_fx2.h"

stompbox_fx2::stompbox_fx2(QWidget *parent)
    : stompBox(parent)
{
	/* FX2 */
	setImage(":/images/fx2.png");
	setLSB("06", "00");
	setComboBox("06", "00", "01", QRect(8, 31, 79, 13));
	setButton("06", "00", "00");
	editDetails()->patchPos(318, 202, "06", "00");
	setEditPages();
};

void stompbox_fx2::updateSignal()
{
	updateComboBox("06", "00", "01");
	updateButton("06", "00", "00");
};

void stompbox_fx2::setEditPages()
{

  editDetails()->page()->newGroupBox("Effect");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "06", "00", "00", "middle", Qt::AlignCenter);
	editDetails()->page()->newStackControl(0);
	editDetails()->page()->addComboBox(0, 1, 1, 1, "06", "00", "01");     
	editDetails()->page()->addStackControl();
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	editDetails()->page()->insertStackField(0, 1, 0, 1, 1);
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Harmony 1");
	editDetails()->page()->newStackControl(1);
	editDetails()->page()->addComboBox(0, 0, 1, 1, "06", "00", "02");     //voice
	editDetails()->page()->addStackControl();
	editDetails()->page()->addComboBox(0, 1, 1, 1, "06", "00", "03");     //mode
	editDetails()->page()->addKnob(0, 2, 1, 1, "06", "00", "04");         //hr1 pitch
	editDetails()->page()->addKnob(0, 3, 1, 1, "06", "00", "05");         //hr1 pitch fine
	editDetails()->page()->addComboBox(0, 4, 2, 1, "06", "00", "06");     //hr1 harmony
	editDetails()->page()->addKnob(0, 5, 1, 1, "06", "00", "07", "normal","right", 90);//hr1 pre delay
	editDetails()->page()->addKnob(0, 6, 1, 1, "06", "00", "09", "turbo");//hr1 feedback
	editDetails()->page()->addKnob(0, 7, 1, 1, "06", "00", "0A");         //hr1 level
	editDetails()->page()->addGroupBox(1, 0, 1, 3);

	editDetails()->page()->insertStackField(1, 2, 0, 1, 2);
	
	editDetails()->page()->newGroupBox("Output");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "06", "00", "12");     //key
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "13");         //direct level
	editDetails()->page()->addGroupBox(2, 2, 1, 1);
	editDetails()->page()->addStackField(); 

	editDetails()->page()->newStackField(1);
	editDetails()->page()->addStackField();
	
	editDetails()->page()->newStackField(1);
	editDetails()->page()->newGroupBox("Harmony 2");
		editDetails()->page()->addComboBox(0, 0, 1, 1, "06", "00", "0B");     //mode
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "0C");         //hr2 pitch
	editDetails()->page()->addKnob(0, 2, 1, 1, "06", "00", "0D");         //hr2 pitch fine
	editDetails()->page()->addComboBox(0, 3, 1, 1, "06", "00", "0E");     //hr2 harmony
	editDetails()->page()->addKnob(0, 4, 1, 1, "06", "00", "0F" ,"normal","right", 90);         //hr2 pre delay
	editDetails()->page()->addKnob(0, 5, 1, 1, "06", "00", "11", "turbo");//hr2 level
	editDetails()->page()->addGroupBox(2, 0, 1, 1);
	editDetails()->page()->addStackField();
	
		editDetails()->page()->newStackField(1);
	editDetails()->page()->newGroupBox("Harmony 2");
		editDetails()->page()->addComboBox(0, 0, 1, 1, "06", "00", "0B");     //mode
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "0C");         //hr2 pitch
	editDetails()->page()->addKnob(0, 2, 1, 1, "06", "00", "0D");         //hr2 pitch fine
	editDetails()->page()->addComboBox(0, 3, 1, 1, "06", "00", "0E");     //hr2 harmony
	editDetails()->page()->addKnob(0, 4, 1, 1, "06", "00", "0F" ,"normal","right", 90);         //hr2 pre delay
	editDetails()->page()->addKnob(0, 5, 1, 1, "06", "00", "11", "turbo");//hr2 level
	editDetails()->page()->addGroupBox(2, 0, 1, 1);
	editDetails()->page()->addStackField();
	  // PAGE 1 ABOVE
	
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Flanger");
	editDetails()->page()->addKnob(0, 0, 1, 1, "06", "00", "14", "normal","right", 90);//rate
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "15");         //depth
	editDetails()->page()->addKnob(0, 2, 1, 1, "06", "00", "16", "turbo");//manual
	editDetails()->page()->addKnob(0, 3, 1, 1, "06", "00", "17");         //resonance
	editDetails()->page()->addKnob(0, 4, 1, 1, "06", "00", "18", "turbo");//separation
	editDetails()->page()->addKnob(0, 5, 1, 1, "06", "00", "19", "turbo");//level
	editDetails()->page()->addGroupBox(1, 0, 1, 7);
  editDetails()->page()->addStackField();   // PAGE 2 ABOVE
	
	
   editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Phaser");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "06", "00", "1A");     //type
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "1B" ,"normal", "right", 90);         //rate
	editDetails()->page()->addKnob(0, 2, 1, 1, "06", "00", "1C", "turbo");//depth
	editDetails()->page()->addKnob(0, 3, 1, 1, "06", "00", "1D");         //manual
	editDetails()->page()->addKnob(0, 4, 1, 1, "06", "00", "1E", "turbo");//resonance
	editDetails()->page()->addComboBox(0, 5, 1, 1, "06", "00", "1F");     //step
	editDetails()->page()->addKnob(0, 6, 1, 1, "06", "00", "20", "normal", "right", 90);//step rate
	editDetails()->page()->addKnob(0, 7, 1, 1, "06", "00", "21");         //level
	editDetails()->page()->addGroupBox(1, 0, 1, 8);
	editDetails()->page()->addStackField();   // PAGE 3 ABOVE
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Auto Riff");
	editDetails()->page()->addComboBox(0, 1, 1, 1, "06", "00", "22");     //phrase
	editDetails()->page()->addComboBox(0, 2, 1, 1, "06", "00", "23");     //loop
	editDetails()->page()->addKnob(0, 3, 1, 1, "06", "00", "24", "normal", "right", 90);//tempo
	editDetails()->page()->addKnob(0, 4, 1, 1, "06", "00", "25");         //sensitivity
	editDetails()->page()->addComboBox(0, 5, 1, 1, "06", "00", "26");     //key
	editDetails()->page()->addKnob(0, 6, 1, 1, "06", "00", "27");         //attack
	editDetails()->page()->addComboBox(0, 7, 1, 1, "06", "00", "28");     //hold
	editDetails()->page()->addGroupBox(1, 0, 1, 4);
	editDetails()->page()->newGroupBox("Output");
	editDetails()->page()->addKnob(0, 0, 1, 1, "06", "00", "29", "turbo");//effect level
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "2A");         //direct level
	editDetails()->page()->addGroupBox(2, 0, 1, 5);
	editDetails()->page()->addStackField();   // PAGE 4 ABOVE


	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Low EQ");
	editDetails()->page()->addKnob(2, 2, 1, 1, "06", "00", "2B", "turbo"); //level
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	
	editDetails()->page()->newGroupBox("Low Mid EQ");
	editDetails()->page()->addKnob(2, 0, 1, 1, "06", "00", "2C", "turbo"); //Q
	editDetails()->page()->addKnob(2, 1, 1, 1, "06", "00", "2D");          //freq
	editDetails()->page()->addKnob(2, 2, 1, 1, "06", "00", "2E", "turbo"); //level
	editDetails()->page()->addGroupBox(0, 1, 1, 1);
	
	editDetails()->page()->newGroupBox("High Mid EQ");
	editDetails()->page()->addKnob(1, 0, 1, 1, "06", "00", "2F", "turbo"); //Q
	editDetails()->page()->addKnob(1, 1, 1, 1, "06", "00", "30");          //freq
	editDetails()->page()->addKnob(1, 2, 1, 1, "06", "00", "31", "turbo"); //level
	editDetails()->page()->addGroupBox(0, 2, 1, 1);

	editDetails()->page()->newGroupBox("High EQ");
	editDetails()->page()->addKnob(0, 2, 1, 1, "06", "00", "32", "turbo"); //level
	editDetails()->page()->addGroupBox(0, 3, 1, 1);

	editDetails()->page()->newGroupBox("Output");
	editDetails()->page()->addKnob(2, 0, 1, 1, "06", "00", "33");          //level
	editDetails()->page()->addGroupBox(1, 0, 1, 4);
	editDetails()->page()->addStackField();                        // PAGE 5 ABOVE 
	
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("X-over freq");
	editDetails()->page()->addKnob(0, 0, 1, 1, "06", "00", "34", "turbo");//x-over freq
	editDetails()->page()->addGroupBox(1, 5, 1, 1);
	editDetails()->page()->newGroupBox("Low Band");
	editDetails()->page()->addKnob(0, 0, 1, 1, "06", "00", "35", "normal","right", 90);//low rate
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "36");         //low depth
	editDetails()->page()->addKnob(0, 2, 1, 1, "06", "00", "37", "normal","right", 90);//low pre-delay
	editDetails()->page()->addKnob(0, 3, 1, 1, "06", "00", "38");         //low level
	editDetails()->page()->addGroupBox(2, 0, 1, 6);
	editDetails()->page()->newGroupBox("High Band");
	editDetails()->page()->addKnob(0, 0, 1, 1, "06", "00", "39", "normal","right", 90);//high rate
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "3A");         //high depth
	editDetails()->page()->addKnob(0, 2, 1, 1, "06", "00", "3B", "normal","right", 90);//high pre delay
	editDetails()->page()->addKnob(0, 3, 1, 1, "06", "00", "3C");         //high level
	editDetails()->page()->addGroupBox(1, 0, 1, 4);
	editDetails()->page()->addStackField();     // PAGE 6 ABOVE
	
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Short Digital Delay");
	editDetails()->page()->addKnob(0, 0, 1, 1, "06", "00", "3D", "normal", "right", 90);//delay time
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "3F");         //feedback
	editDetails()->page()->addGroupBox(1, 0, 1, 3);
	editDetails()->page()->newGroupBox("Output");
	editDetails()->page()->addKnob(0, 0, 1, 1, "06", "00", "40");         //effect level
	editDetails()->page()->addGroupBox(1, 4, 1, 2);
	editDetails()->page()->addStackField();   // PAGE 7 ABOVE	
	
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Humanizer");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "06", "00", "41");     //mode
	editDetails()->page()->addComboBox(0, 1, 1, 1, "06", "00", "42");     //vowel 1
	editDetails()->page()->addComboBox(0, 2, 1, 1, "06", "00", "43");     //vowel 2
	editDetails()->page()->addKnob(0, 3, 1, 1, "06", "00", "44", "turbo");//sensitivity
	editDetails()->page()->addKnob(0, 5, 1, 1, "06", "00", "45" ,"normal","right", 90);         //rate
	editDetails()->page()->addKnob(0, 6, 1, 1, "06", "00", "46", "turbo");//depth
	editDetails()->page()->addKnob(0, 7, 1, 1, "06", "00", "47");         //manual
	editDetails()->page()->addKnob(0, 8, 1, 1, "06", "00", "48", "turbo");//level
	editDetails()->page()->addGroupBox(1, 0, 1, 9);
	editDetails()->page()->addStackField();      // PAGE 8 ABOVE
	
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Vibrato");
	editDetails()->page()->addKnob(0, 0, 1, 1, "06", "00", "49", "normal","right", 90);//rate
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "4A");         //depth
	editDetails()->page()->addComboBox(0, 2, 1, 1, "06", "00", "4B");     //trigger
	editDetails()->page()->addGroupBox(1, 0, 1, 3);
	editDetails()->page()->newGroupBox("Time");
	editDetails()->page()->addKnob(0, 0, 1, 1, "06", "00", "4C");         //effect level
	editDetails()->page()->addGroupBox(1, 4, 1, 2);
	editDetails()->page()->addStackField();     // PAGE 9 ABOVE	
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Effect");
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "4D", "turbo");//sens
	editDetails()->page()->addComboBox(0, 3, 1, 1, "06", "00", "4E");//, "middle", Qt::AlignCenter);     //wave
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	editDetails()->page()->newGroupBox("Wave");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "06", "00", "4F");     //chromatic
	editDetails()->page()->addComboBox(0, 1, 1, 1, "06", "00", "50");     //octave
	editDetails()->page()->addKnob(0, 2, 1, 1, "06", "00", "51", "turbo");//pwm rate
	editDetails()->page()->addKnob(0, 3, 1, 1, "06", "00", "52");         //pwm depth
  editDetails()->page()->addGroupBox(0, 1, 1, 1);
  editDetails()->page()->newGroupBox("Synth");
	editDetails()->page()->addKnob(0, 0, 1, 1, "06", "00", "58", "turbo");//attack
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "59");         //release
	editDetails()->page()->addKnob(0, 2, 1, 1, "06", "00", "5A", "turbo");//velocity
	editDetails()->page()->addComboBox(0, 4, 1, 1, "06", "00", "5B");     //hold
	editDetails()->page()->addGroupBox(0, 2, 1, 1); 
  editDetails()->page()->newGroupBox("Filter");
	editDetails()->page()->addKnob(0, 0, 1, 1, "06", "00", "53", "turbo");//cutoff freq
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "54");         //resonance
	editDetails()->page()->addKnob(0, 2, 1, 1, "06", "00", "55");         //flt sens
	editDetails()->page()->addKnob(0, 3, 1, 1, "06", "00", "56");         //flt decay
	editDetails()->page()->addKnob(0, 4, 1, 1, "06", "00", "57", "turbo");//flt depth
	editDetails()->page()->addGroupBox(1, 0, 1, 2);   
	editDetails()->page()->newGroupBox("Output");
	editDetails()->page()->addKnob(0, 0, 1, 1, "06", "00", "5C");         //synth level
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "5D", "turbo");//direct level
	editDetails()->page()->addGroupBox(1, 2, 1, 1);
	editDetails()->page()->addStackField();    // PAGE 10 ABOVE 
	
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Ring Modulator");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "06", "00", "5E");     //mode
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "5F", "turbo");//freq
	editDetails()->page()->addGroupBox(1, 0, 1, 2);
	editDetails()->page()->newGroupBox("Output");
	editDetails()->page()->addKnob(0, 0, 1, 1, "06", "00", "60", "turbo");//effect
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "61");         //direct
	editDetails()->page()->addGroupBox(1, 4, 1, 1);
	editDetails()->page()->addStackField();       // PAGE 11 ABOVE
	
	
	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Slicer");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "06", "00", "62");     //pattern
	editDetails()->page()->addKnob(0, 1, 1, 1, "06", "00", "63", "normal","right", 90);//rate
	editDetails()->page()->addKnob(0, 2, 1, 1, "06", "00", "64");         //trigger sens
	editDetails()->page()->addGroupBox(1, 0, 1, 4);
	editDetails()->page()->addStackField();    // PAGE 12 ABOVE
	
	editDetails()->addPage();		
};
