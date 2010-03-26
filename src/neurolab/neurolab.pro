# -------------------------------------------------
# Project created by QtCreator 2009-10-10T12:17:33
# -------------------------------------------------
QT += svg
TEMPLATE = app
TARGET = neurolab
include(../neurolab_version.txt)
INCLUDEPATH += ../thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/src
SOURCES += main.cpp 
debug:BUILDDIR = debug
else:BUILDDIR = release
DESTDIR = ../$$BUILDDIR
OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR
win32:LIBS = -L$$DESTDIR \
    -lneurogui1 \
    -lneurolib1 \
    -lautomata1 \
    -lqtpropertybrowser2
else:LIBS = -L$$DESTDIR \
    -lneurogui \
    -lneurolib \
    -lautomata \
    -lqtpropertybrowser
