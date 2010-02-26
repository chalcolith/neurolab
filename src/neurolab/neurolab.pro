# -------------------------------------------------
# Project created by QtCreator 2009-10-10T12:17:33
# -------------------------------------------------
QT += svg
TEMPLATE = app
TARGET = neurolab
include(../neurolab_version.txt)

INCLUDEPATH += ../thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/src

SOURCES += main.cpp \
    mainwindow.cpp \
    labnetwork.cpp \
    labscene.cpp \
    labview.cpp \
    labtree.cpp \
    neuronodeitem.cpp \
    neurolinkitem.cpp \
    neuroitem.cpp \
    labeldialog.cpp
HEADERS += mainwindow.h \
    labnetwork.h \
    labscene.h \
    labview.h \
    labtree.h \
    neuronodeitem.h \
    neurolinkitem.h \
    neuroitem.h \
    labeldialog.h
FORMS += mainwindow.ui \
    labeldialog.ui

debug:BUILDDIR=debug
else:BUILDDIR=release

DESTDIR = ../$$BUILDDIR
OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR
LIBS = -L$$DESTDIR -lneurolib1 -lautomata1 -lqtpropertybrowser2
