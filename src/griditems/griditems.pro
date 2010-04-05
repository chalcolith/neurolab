QT += gui
QT += testlib

TARGET = griditems
TEMPLATE = lib

include(../version.txt)
INCLUDEPATH += ../thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/src

DEFINES += GRIDITEMS_LIBRARY

SOURCES += neurogriditem.cpp

HEADERS += neurogriditem.h\
        griditems_global.h

debug:BUILDDIR=debug
else:BUILDDIR=release

DESTDIR = ../$$BUILDDIR/plugins
OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR

win32:LIBS = -L$$DESTDIR/.. \
    -lneurogui1 \
    -lneurolib1 \
    -lautomata1 \
    -lqtpropertybrowser2
else:LIBS = -L$$DESTDIR/.. \
    -lneurogui \
    -lneurolib \
    -lautomata \
    -lqtpropertybrowser
