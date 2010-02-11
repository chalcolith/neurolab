QT -= gui
TEMPLATE = lib
TARGET = qtpropertybrowser
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

debug:BUILDDIR=debug
else:BUILDDIR=release

DESTDIR = ../../$$BUILDDIR
OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR
RCC_DIR = $$BUILDDIR
