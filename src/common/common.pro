CONFIG += debug_and_release
QT -= gui

TARGET = common
TEMPLATE = lib

macx { CONFIG += lib_bundle }
macx { CONFIG += x86 }
macx { CONFIG -= x86_64 }

include(../version.txt)

DEFINES += COMMON_LIBRARY

SOURCES += exception.cpp

HEADERS += common.h \
    exception.h \

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
