QT -= gui

TARGET = automata
TEMPLATE = lib

macx { CONFIG += lib_bundle }
macx { CONFIG += x86 x86_64 }

include(../version.txt)
DEFINES += AUTOMATA_LIBRARY

SOURCES += exception.cpp

HEADERS += automaton.h \
    automata_global.h \
    graph.h \
    exception.h \
    asyncstate.h \
    pool.h

build_pass:release { BUILDDIR=$$OUT_PWD/release }
build_pass:debug { BUILDDIR=$$OUT_PWD/debug }

macx { DESTDIR = $$BUILDDIR/neurolab.app/Contents/Frameworks }
else { DESTDIR = $$BUILDDIR }

OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR
