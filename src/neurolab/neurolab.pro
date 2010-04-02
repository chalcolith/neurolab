QT += svg

TEMPLATE = app
TARGET = neurolab

include(../version.txt)
INCLUDEPATH += ../thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/src

SOURCES += main.cpp

debug:BUILDDIR = debug
else:BUILDDIR = release

DESTDIR = ../$$BUILDDIR
OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR

win32:LIBS = -L$$DESTDIR \
    -lneurogui0 \
    -lneurolib0 \
    -lautomata0 \
    -lqtpropertybrowser2
else:LIBS = -L$$DESTDIR \
    -lneurogui \
    -lneurolib \
    -lautomata \
    -lqtpropertybrowser
