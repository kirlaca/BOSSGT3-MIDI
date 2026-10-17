#############################################################################
##
## Copyright (C) 2007~2025 Colin Willcocks.
## Copyright (C) 2005~2007 Uco Mesdag.
## All rights reserved.
##
## This file is part of "GT-3 Fx FloorBoard".
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
##
#############################################################################

#Header files
HEADERS += ./editPage.h \
    ./editWindow.h \
    ./globalVariables.h \
    ./mainWindow.h \
    ./bankTreeList.h \
    ./bulkLoadDialog.h \
    ./bulkSaveDialog.h \
    ./customButton.h \
    ./customComboBox.h \
    ./customControlKnob.h \
    ./customControlLabel.h \
    ./customControlListMenu.h \
    ./customControlRange.h \
    ./customControlSwitch.h \
    ./customControlTarget.h \
    ./customDial.h \
    ./customDisplay.h \
    ./customKnob.h \
    ./customKnobRange.h \
    ./customKnobTarget.h \
    ./customLabelDisplay.h \
    ./customLed.h \
    ./customRangeDial.h \
    ./customRenameDialog.h \
    ./customRenameWidget.h \
    ./customSlider.h \
    ./customSplashScreen.h \
    ./customSwitch.h \
    ./customTargetDial.h \
    ./customTargetListMenu.h \
    ./dragBar.h \
    ./fileDialog.h \
    ./floorBoard.h \
    ./floorBoardDisplay.h \
    ./floorPanelBar.h \
    ./floorPanelBarButton.h \
    ./initPatchListMenu.h \
    ./menuPage.h \
    ./menuPage_assign.h \
    ./menuPage_midi.h \
    ./menuPage_system.h \
    ./midiIO.h \
    ./midiIODestroyer.h \
    ./MidiTable.h \
    ./MidiTableDestroyer.h \
    ./Preferences.h \
    ./PreferencesDestroyer.h \
    ./preferencesDialog.h \
    ./preferencesPages.h \
    ./renameDialog.h \
    ./renameWidget.h \
    ./RtMidi.h \
    ./statusBarSymbol.h \
    ./statusBarWidget.h \
    ./stompBox.h \
    ./stompbox_ce.h \
    ./stompbox_cs.h \
    ./stompbox_dd.h \
    ./stompbox_eq.h \
    ./stompbox_fv.h \
    ./stompbox_fx1.h \
    ./stompbox_fx2.h \
    ./stompbox_ns.h \
    ./stompbox_od.h \
    ./stompbox_pre.h \
    ./stompbox_rv.h \
    ./stompbox_wah.h \
    ./summaryDialog.h \  
    ./SysxIO.h \
    ./SysxIODestroyer.h \
    ./sysxWriter.h \
    ./xmlwriter/xmlwriter.h \
    ./xmlwriter/xmlwriter_p.h 
  
#Source files
SOURCES += ./editPage.cpp \
    ./editWindow.cpp \
    ./main.cpp \
    ./mainWindow.cpp \
    ./bankTreeList.cpp \
    ./bulkLoadDialog.cpp \
    ./bulkSaveDialog.cpp \
    ./customButton.cpp \
    ./customComboBox.cpp \
    ./customControlKnob.cpp \
    ./customControlLabel.cpp \
    ./customControlListMenu.cpp \
    ./customControlRange.cpp \
    ./customControlSwitch.cpp \
    ./customControlTarget.cpp \
    ./customDial.cpp \
    ./customDisplay.cpp \
    ./customKnob.cpp \
    ./customKnobRange.cpp \
    ./customKnobTarget.cpp \
    ./customLabelDisplay.cpp \
    ./customLed.cpp \
    ./customRangeDial.cpp \
    ./customRenameDialog.cpp \
    ./customRenameWidget.cpp \
    ./customSlider.cpp \
    ./customSplashScreen.cpp \
    ./customSwitch.cpp \
    ./customTargetDial.cpp \
    ./customTargetListMenu.cpp \
    ./dragBar.cpp \
    ./fileDialog.cpp \
    ./floorBoard.cpp \
    ./floorBoardDisplay.cpp \
    ./floorPanelBar.cpp \
    ./floorPanelBarButton.cpp \
    ./initPatchListMenu.cpp \
    ./menuPage.cpp \
    ./menuPage_assign.cpp \
    ./menuPage_midi.cpp \
    ./menuPage_system.cpp \
    ./midiIO.cpp \
    ./midiIODestroyer.cpp \
    ./MidiTable.cpp \
    ./MidiTableDestroyer.cpp \
    ./Preferences.cpp \
    ./PreferencesDestroyer.cpp \
    ./preferencesDialog.cpp \
    ./preferencesPages.cpp \
    ./renameDialog.cpp \
    ./renameWidget.cpp \
    ./RtMidi.cpp \
    ./statusBarSymbol.cpp \
    ./statusBarWidget.cpp \
    ./stompBox.cpp \
    ./stompbox_ce.cpp \
    ./stompbox_cs.cpp \
    ./stompbox_dd.cpp \
    ./stompbox_eq.cpp \
    ./stompbox_fv.cpp \
    ./stompbox_fx1.cpp \
    ./stompbox_fx2.cpp \
    ./stompbox_ns.cpp \
    ./stompbox_od.cpp \
    ./stompbox_pre.cpp \
    ./stompbox_rv.cpp \
    ./stompbox_wah.cpp \
    ./summaryDialog.cpp \
    ./SysxIO.cpp \
    ./SysxIODestroyer.cpp \
    ./sysxWriter.cpp \
    ./xmlwriter/XMLWriter.cpp 

#Resource file(s)
RESOURCES += GT-3FxFloorBoard.qrc
