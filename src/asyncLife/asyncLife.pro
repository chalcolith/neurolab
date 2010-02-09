# -------------------------------------------------
# Project created by QtCreator 2009-07-30T14:19:23
# -------------------------------------------------
TARGET = asyncLife
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    lifecell.cpp \
    lifeboard.cpp \
    lifewidget.cpp
HEADERS += \
    mainwindow.h \
    lifecell.h \
    lifeboard.h \
    lifewidget.h
FORMS += mainwindow.ui

debug:BUILDDIR=debug
else:BUILDDIR=release

DESTDIR = ../$$BUILDDIR
OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR
LIBS = -L$$DESTDIR -lautomata
