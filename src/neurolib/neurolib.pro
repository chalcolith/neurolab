# -------------------------------------------------
# Project created by QtCreator 2009-10-10T12:15:38
# -------------------------------------------------
QT -= gui
QT += testlib
TARGET = neurolib
TEMPLATE = lib
DEFINES += NEUROLIB_LIBRARY
SOURCES += neuronet.cpp \
    neurocell.cpp
HEADERS += neuronet.h \
    neurolib_global.h \
    neurocell.h
debug:LIBS = -L../automata/debug \
    -lautomata
else:LIBS = -L../automata/release \
    -lautomata
debug:DLLDESTDIR = ../neurolab/debug
else:DLLDESTDIR = ../neurolab/release
