QT += gui

TARGET = griditems
TEMPLATE = lib

macx:CONFIG += lib_bundle
macx:CONFIG += x86 x86_64

include(../version.txt)
INCLUDEPATH += ../thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/src

DEFINES += GRIDITEMS_LIBRARY

SOURCES += neurogriditem.cpp

HEADERS += neurogriditem.h\
        griditems_global.h

debug:BUILDDIR=debug
else:BUILDDIR=release

macx:DESTDIR = ../$$BUILDDIR/neurolab.app/Contents/Frameworks
else:DESTDIR = ../$$BUILDDIR/plugins

OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR

macx:QMAKE_LFLAGS += -F$$DESTDIR

win32:LIBS += -L$$DESTDIR/.. \
    -lneurogui1 \
    -lneurolib1 \
    -lautomata1 \
    -lqtpropertybrowser2
macx:LIBS += -framework neurogui \
    -framework neurolib \
    -framework automata \
    -framework qtpropertybrowser
else:LIBS += -L$$DESTDIR/.. \
    -lneurogui \
    -lneurolib \
    -lautomata \
    -lqtpropertybrowser
