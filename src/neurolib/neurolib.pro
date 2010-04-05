QT -= gui
QT += testlib

TEMPLATE = lib

TARGET = neurolib

include(../version.txt)

DEFINES += NEUROLIB_LIBRARY

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

win32:LIBS = -L$$DESTDIR -lautomata1
else:LIBS = -L$$DESTDIR -lautomata
