# -------------------------------------------------
# Project created by QtCreator 2009-10-10T12:17:33
# -------------------------------------------------
QT += opengl \
    svg
TARGET = NeuroLab
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    labnetwork.cpp \
    labscene.cpp \
    labview.cpp \
    labtree.cpp \
    neuronodeitem.cpp \
    neurolinkitem.cpp
HEADERS += mainwindow.h \
    labnetwork.h \
    labscene.h \
    labview.h \
    labtree.h \
    neuronodeitem.h \
    neurolinkitem.h
FORMS += mainwindow.ui
debug:LIBS = -L../neurolib/debug \
    -lneurolib \
    -L../automata/debug \
    -lautomata
else:LIBS = -L../neurolib/release \
    -lneurolib \
    -L../automata/release \
    -lautomata
