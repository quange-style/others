#-------------------------------------------------
#
# Project created by QtCreator 2022-01-14T16:35:11
#
#-------------------------------------------------

QT       += widgets  network

QT       -= gui

TARGET = whiteboard
TEMPLATE = lib

DEFINES += WHITEBOARD_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        painterscene.cpp \
        painterview.cpp \
        shape.cpp \
        wbconnect.cpp

HEADERS += \
        whiteboard_global.h \
        painterscene.h \
        painterview.h \
        shape.h \
        wbconnect.h

unix {
    target.path = /opt/app/lib3rd/lib
    INSTALLS += target
}

OBJECTS_DIR = ./debug/obj
MOC_DIR = ./debug/moc
RCC_DIR = ./debug/rcc
UI_DIR=./debug/ui