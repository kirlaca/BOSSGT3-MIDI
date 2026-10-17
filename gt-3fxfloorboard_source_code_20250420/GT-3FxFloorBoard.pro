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

TEMPLATE = app
CONFIG += embed_manifest_exe
CONFIG += release
TARGET = "GT-3FxFloorBoard"
DESTDIR = ./packager
MOC_DIR += ./generatedfiles/release
OBJECTS_DIR += release
UI_DIR += ./generatedfiles
INCLUDEPATH += ./generatedfiles \
        ./generatedfiles/release \
        .

TRANSLATIONS = language_fr.ts \
               language_ge.ts \
               language_ch.ts

CODECFORTR = UTF-8


DEPENDPATH += .
QT += xml
QT += widgets
QT += printsupport


#Platform dependent file(s)
win32-g++:contains(QMAKE_HOST.arch, x86):{
       message("x86 build")
       ## Windows x86 (32bit) specific build here
       exists("C:\Qt\gt-3\windows\WinMM.Lib")
               {	# <-- Change the path to WinMM.Lib here!
                   LIBS += C:\Qt\gt-3\windows\WinMM.Lib	# <-- Change the path here also!
                   QT += printsupport
                   win32:RC_FILE = GT-3FxFloorBoard.rc
               }
               INCLUDEPATH += C:/Qt/Tools/OpenSSL/Win_x86/include
               INCLUDEPATH += C:/Qt/Tools/OpenSSL/Win_x86/bin
               LIBS += -LC:/Qt/Tools/OpenSSL/Win_x86/bin -llibcrypto-1_1 -llibssl-1_1
               CONFIG += embed_manifest_exe
       message(Including Windows 32 specific headers and sources...)
}
win32:contains(QMAKE_HOST.arch, x86_64):{
       message("x86_64 build")
       ## Windows x86_64 (64bit) specific build here
       exists("C:\Qt\gt-3\win64\WinMM.Lib")
               {	# <-- Change the path to WinMM.Lib here!
                   LIBS += C:\Qt\gt-3\win64\WinMM.Lib	# <-- Change the path here also!
                   QT += printsupport
                   win32:RC_FILE = GT-3FxFloorBoard.rc
               }
               INCLUDEPATH += C:/Qt/Tools/OpenSSL/Win_x64/include
               INCLUDEPATH += C:/Qt/Tools/OpenSSL/Win_x64/bin
             #  LIBS += -LC:/Qt/Tools/OpenSSL/Win_x64/bin -llibcrypto-1_1-x64 -llibssl-1_1-x64
               CONFIG += embed_manifest_exe
       message(Including Windows 64 specific headers and sources...)
}
linux-g++{
        LIBS += -lasound -lbluetooth -Bluez
        QT += printsupport
        message("ALSA LIBRARIES SHOULD BE INSTALLED or ERROR will Occur")
        message("Please install the ALSA Audio System packages if not present")
        message(Including Linux 32 specific headers and sources...)
}
linux-g++-64{
        LIBS += -lasound -lbluetooth -Bluez
        QT += printsupport
        message("ALSA LIBRARIES SHOULD BE INSTALLED or ERROR will Occur")
        message("Please install the ALSA Audio System packages if not present")
        message(Including Linux 64 specific headers and sources...)
}
macx{
        LIBS += -framework CoreMidi -framework CoreAudio -framework CoreFoundation #-framework CoreBluetooth
        QT += printsupport
        message("X-Code LIBRARIES SHOULD BE INSTALLED or ERROR will Occur")
        message("Please install the X-Code Audio System packages if not present")
        ICON = GT-3FxFloorBoard.icns
        message(Including Mac OS X specific headers and sources...)
}
ios{
        LIBS += -framework CoreMidi -framework CoreAudio -framework CoreFoundation #-framework CoreBluetooth
        message("X-Code LIBRARIES SHOULD BE INSTALLED or ERROR will Occur")
        message("Please install the X-Code Audio System packages if not present")
        ICON = GT-3FxFloorBoard.icns
        message(Including Apple IOS specific headers and sources...)
}
android{
        message(Including Android specific headers and sources...)
        QT += androidextras
        ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
        INCLUDEPATH += C:/Qt/Tools/OpenSSL/Win_x64/include
        INCLUDEPATH += C:/Qt/Tools/OpenSSL/Win_x64/bin
        DISTFILES += \
        android/AndroidManifest.xml \
        android/build.gradle \
        android/gradle.properties \
        android/gradle/wrapper/gradle-wrapper.jar \
        android/gradle/wrapper/gradle-wrapper.properties \
        android/gradlew \
        android/gradlew.bat \
        android/res/values/libs.xml
}

#Include file(s)
include(GT-3FxFloorBoard.pri)

#Windows resource file
win32:RC_FILE = GT-3FxFloorBoard.rc
