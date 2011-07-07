CONFIG += debug_and_release
QT += gui

TARGET = qtpropertybrowser
TEMPLATE = lib

macx { CONFIG += lib_bundle }

VERSION = 2.5.1

SRC = qtpropertybrowser-2.5_1-opensource/src

SOURCES += $$SRC/qtpropertybrowser.cpp \
            $$SRC/qtpropertymanager.cpp \
            $$SRC/qteditorfactory.cpp \
            $$SRC/qtvariantproperty.cpp \
            $$SRC/qttreepropertybrowser.cpp \
            $$SRC/qtbuttonpropertybrowser.cpp \
            $$SRC/qtgroupboxpropertybrowser.cpp \
            $$SRC/qtpropertybrowserutils.cpp
HEADERS += $$SRC/qtpropertybrowser.h \
            $$SRC/qtpropertymanager.h \
            $$SRC/qteditorfactory.h \
            $$SRC/qtvariantproperty.h \
            $$SRC/qttreepropertybrowser.h \
            $$SRC/qtbuttonpropertybrowser.h \
            $$SRC/qtgroupboxpropertybrowser.h \
            $$SRC/qtpropertybrowserutils_p.h
RESOURCES += $$SRC/qtpropertybrowser.qrc

CONFIG(release, debug|release) { BUILDDIR=release }
CONFIG(debug, debug|release) {
    BUILDDIR=debug
    DEFINES += DEBUG
}

macx { DESTDIR = $$OUT_PWD/../../$$BUILDDIR/neurolab.app/Contents/Frameworks }
else { DESTDIR = $$OUT_PWD/../../$$BUILDDIR }
TEMPDIR = $$OUT_PWD/$$BUILDDIR

OBJECTS_DIR = $$TEMPDIR
MOC_DIR = $$TEMPDIR
UI_DIR = $$TEMPDIR
RCC_DIR = $$TEMPDIR
