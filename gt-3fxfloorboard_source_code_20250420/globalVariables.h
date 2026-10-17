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

#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

/****************************************************************************
* These are general settings keep in mind that number of fx and fx setting 
* are per item setting to change them you will need to modify the following 
* files:
*     -> floorBoard.cpp
*     -> stompBox.cpp
*     -> stompbox_{fx abbrv.}.cpp
*
****************************************************************************/

/* General Parameters */
const QString deviceType = "GT-3";
const QString idRequestString = "F04100001B1100000000000000017FF7";		// Indentity Request (GT-3).
const QString idReplyPatern = "F04100001B12";			// Returned device id message must contain/match this (QRegularExpression or a string without spaces and all caps).
const int buttonBlinkInterval = 250;				// The interval (ms) the led on buttons blink.

/* Sysex Message Parameters */
const int sysxAddressOffset = 6;	// Offset (starts at 0) where the address information starts in a sysx message.
const int sysxDataOffset = 10;		// Offset (starts at 0) where the data starts in a sysx message.
const int sysxNameOffset = 374;		// Offset (starts at 0) where the data starts in a sysx message.
const int nameLength = 11;        // length of name string
const int checksumOffset = 6;		// Offset (starts at 0) where we start calculating the checksum (Normally this is the address offset).

/* Patches and Banks */
const int bankTotalUser = 35;		// Number of user (editable) banks.
const int bankTotalAll = 85;		// Number of total banks.
const int patchPerBank = 4;			// Number of patches in a bank.
const int bankSize = 100;       // Number of patches in a midi bank before bank change message is required.
const int bankUserStart = 7;    // Start address of user patch range.

/* Midi Send & Receive */
const int patchPackets = 23;		// number of sysx packets "F0....F7" which make up a patch.
const int patchSize = 650;          // size of bytes in a patch.
const QString patchRequestSize = "00000001";
const int maxRetry = 2;			// Maximum times we retry to load a patch in case of a transfer error .
const QString systemRequest = "F04100001B1100000000020000017DF7";

/* Patch Sellection (Used when copying patches) */
const int sellectionBlinks = 5;				// Times we blink to indicate we have sellected a patch before returning.
const int sellectionBlinkInterval = 500;	// Interval (ms) the item blinks.

/* Patch addresses */
const QString chainAddress = "0B";
const QString nameAddress = "0C";
const QString tempBulkWrite = "0C";
const QString tempDataWrite = "0C";

#endif // GLOBALVARIABLES_H
