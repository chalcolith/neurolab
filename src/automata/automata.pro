# -------------------------------------------------
# Project created by QtCreator 2009-07-30T14:22:13
# -------------------------------------------------
QT -= gui
TARGET = automata
TEMPLATE = lib
DEFINES += AUTOMATA_LIBRARY
SOURCES += exception.cpp
HEADERS += automaton.h \
    automata_global.h \
    graph.h \
    exception.h \
    asyncstate.h
debug:DLLDESTDIR = ../neurolab/debug
else:DLLDESTDIR = ../neurolab/release
