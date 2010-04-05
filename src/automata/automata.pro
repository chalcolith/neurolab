QT -= gui
QT += testlib

TEMPLATE = lib

TARGET = automata

include(../version.txt)
DEFINES += AUTOMATA_LIBRARY

SOURCES += exception.cpp

HEADERS += automaton.h \
    automata_global.h \
    graph.h \
    exception.h \
    asyncstate.h \
    pool.h

debug:BUILDDIR=debug
else:BUILDDIR=release

DESTDIR = ../$$BUILDDIR
OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR
