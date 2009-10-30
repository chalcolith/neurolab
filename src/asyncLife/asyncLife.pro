# -------------------------------------------------
# Project created by QtCreator 2009-07-30T14:19:23
# -------------------------------------------------
TARGET = asyncLife
TEMPLATE = app
debug:LIBS = -L../automata/debug \
    -lautomata
else:LIBS = -L../automata/release \
    -lautomata
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
