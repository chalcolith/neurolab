# -------------------------------------------------
# Project created by QtCreator 2009-10-10T12:15:38
# -------------------------------------------------
QT -= gui
QT += testlib
TARGET = neurolib
TEMPLATE = lib
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
LIBS = -L$$DESTDIR -lautomata
