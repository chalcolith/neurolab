CONFIG += debug_and_release
QT -= gui

TARGET = automata
TEMPLATE = lib

macx { CONFIG += lib_bundle }
macx { CONFIG += x86 }
macx { CONFIG -= x86_64 }

include(../version.txt)

DEFINES += AUTOMATA_LIBRARY

HEADERS += automaton.h \
    automata_global.h \
    graph.h \
    asyncstate.h \
    pool.h

SOURCES += automata.cpp

CONFIG(release, debug|release) { BUILDDIR=release }
CONFIG(debug, debug|release) {
    BUILDDIR=debug
    DEFINES += DEBUG
}

macx { DESTDIR = $$OUT_PWD/../$$BUILDDIR/neurolab.app/Contents/Frameworks }
else { DESTDIR = $$OUT_PWD/../$$BUILDDIR }
TEMPDIR = $$OUT_PWD/$$BUILDDIR

OBJECTS_DIR = $$TEMPDIR
MOC_DIR = $$TEMPDIR
UI_DIR = $$TEMPDIR
RCC_DIR = $$TEMPDIR

win32 {
    LIBS += -L$$DESTDIR \
        -lcommon1
} else:macx {
    QMAKE_LFLAGS += -F$$DESTDIR
    LIBS += -framework common
} else {
    LIBS += -L$$DESTDIR \
        -lcommon
}
