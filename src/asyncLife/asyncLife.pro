QT += gui

TARGET = asyncLife
TEMPLATE = app

macx { CONFIG += x86 }
macx { CONFIG -= x86_64 }

SOURCES += main.cpp \
    mainwindow.cpp \
    lifecell.cpp \
    lifeboard.cpp \
    lifewidget.cpp
HEADERS += mainwindow.h \
    lifecell.h \
    lifeboard.h \
    lifewidget.h
FORMS += mainwindow.ui

release { BUILDDIR=release }
debug { BUILDDIR=debug }

DESTDIR = $$OUT_PWD/../$$BUILDDIR
TEMPDIR = $$OUT_PWD/$$BUILDDIR

OBJECTS_DIR = $$TEMPDIR
MOC_DIR = $$TEMPDIR
UI_DIR = $$TEMPDIR
RCC_DIR = $$TEMPDIR

win32 {
    LIBS += -L$$DESTDIR -lautomata1
} else:macx {
    QMAKE_LFLAGS += -F$$DESTDIR/asyncLife.app/Contents/Frameworks
    LIBS += -framework automata
} else {
    LIBS += -L$$DESTDIR -lautomata
}
