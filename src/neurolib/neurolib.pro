QT -= gui

TARGET = neurolib
TEMPLATE = lib

macx { CONFIG += lib_bundle }
macx { CONFIG += x86 x86_64 }

include(../version.txt)

DEFINES += NEUROLIB_LIBRARY

SOURCES += neuronet.cpp \
    neurocell.cpp
HEADERS += neuronet.h \
    neurolib_global.h \
    neurocell.h

build_pass:release { BUILDDIR=$$OUT_PWD/release }
build_pass:debug { BUILDDIR=$$OUT_PWD/debug }

macx { DESTDIR = $$BUILDDIR/neurolab.app/Contents/Frameworks }
else { DESTDIR = $$BUILDDIR }

OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR

macx { QMAKE_LFLAGS += -F$$DESTDIR }

win32 { LIBS += -L$$DESTDIR -lautomata1 }
else:macx { LIBS += -framework automata }
else { LIBS += -L$$DESTDIR -lautomata }
