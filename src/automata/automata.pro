QT -= gui

TARGET = automata
TEMPLATE = lib

macx { CONFIG += lib_bundle }
macx { CONFIG += x86 }
macx { CONFIG -= x86_64 }
else { QMAKE_CXXFLAGS += -Wno-type-limits }

include(../version.txt)

DEFINES += AUTOMATA_LIBRARY

SOURCES += exception.cpp

HEADERS += automaton.h \
    automata_global.h \
    graph.h \
    exception.h \
    asyncstate.h \
    pool.h

release { BUILDDIR=release }
debug { BUILDDIR=debug }

macx { DESTDIR = $$OUT_PWD/../$$BUILDDIR/neurolab.app/Contents/Frameworks }
else { DESTDIR = $$OUT_PWD/../$$BUILDDIR }
TEMPDIR = $$OUT_PWD/$$BUILDDIR

OBJECTS_DIR = $$TEMPDIR
MOC_DIR = $$TEMPDIR
UI_DIR = $$TEMPDIR
RCC_DIR = $$TEMPDIR
