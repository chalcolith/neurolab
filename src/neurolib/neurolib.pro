# -------------------------------------------------
# Project created by QtCreator 2009-10-10T12:15:38
# -------------------------------------------------
QT -= gui
QT += testlib
TEMPLATE = lib
TARGET = neurolib
include(../neurolab_version.txt)

DEFINES += NEUROLIB_LIBRARY

INCLUDEPATH += ../thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/src

SOURCES += neuronet.cpp \
    neurocell.cpp
HEADERS += neuronet.h \
    neurolib_global.h \
    neurocell.h

debug:BUILDDIR=debug
else:BUILDDIR=release

DESTDIR = ../$$BUILDDIR
OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR
LIBS = -L$$DESTDIR -lautomata
