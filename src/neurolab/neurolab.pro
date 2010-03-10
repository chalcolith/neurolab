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
    labeldialog.cpp \
    propertyobj.cpp \
    filedirtydialog.cpp
HEADERS += mainwindow.h \
    labnetwork.h \
    labscene.h \
    labview.h \
    labtree.h \
    neuronodeitem.h \
    neurolinkitem.h \
    neuroitem.h \
    labeldialog.h \
    propertyobj.h \
    filedirtydialog.h
FORMS += mainwindow.ui \
    labeldialog.ui \
    filedirtydialog.ui
debug:BUILDDIR = debug
else:BUILDDIR = release
DESTDIR = ../$$BUILDDIR
OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR
win32:LIBS = -L$$DESTDIR \
    -lneurolib1 \
    -lautomata1 \
    -lqtpropertybrowser2
else:LIBS = -L$$DESTDIR \
    -lneurolib \
    -lautomata \
    -lqtpropertybrowser