QT += gui

TARGET = neurogui
TEMPLATE = lib

macx { CONFIG += lib_bundle }
macx { CONFIG += x86 x86_64 }

DEFINES += NEUROGUI_LIBRARY

include(../version.txt)

INCLUDEPATH += ../thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/src

SOURCES += mainwindow.cpp \
    labnetwork.cpp \
    labscene.cpp \
    labview.cpp \
    labtree.cpp \
    neuronodeitem.cpp \
    neurolinkitem.cpp \
    neuroitem.cpp \
    labeldialog.cpp \
    propertyobj.cpp \
    filedirtydialog.cpp \
    neuronarrowitem.cpp \
    labdatafile.cpp \
    aboutdialog.cpp

HEADERS += neurogui_global.h \
    mainwindow.h \
    labnetwork.h \
    labscene.h \
    labview.h \
    labtree.h \
    neuronodeitem.h \
    neurolinkitem.h \
    neuroitem.h \
    labeldialog.h \
    propertyobj.h \
    filedirtydialog.h \
    neuronarrowitem.h \
    labdatafile.h \
    aboutdialog.h

FORMS += mainwindow.ui \
    labeldialog.ui \
    filedirtydialog.ui \
    aboutdialog.ui

release { BUILDDIR=release }
debug { BUILDDIR=debug }

macx { DESTDIR = $$OUT_PWD/../$$BUILDDIR/neurolab.app/Contents/Frameworks }
else { DESTDIR = $$OUT_PWD/../$$BUILDDIR }
TEMPDIR = $$OUT_PWD/$$BUILDDIR

OBJECTS_DIR = $$TEMPDIR
MOC_DIR = $$TEMPDIR
UI_DIR = $$TEMPDIR
RCC_DIR = $$TEMPDIR

win32 {
    LIBS += -L$$DESTDIR \
        -lneurolib1 \
        -lautomata1 \
        -lqtpropertybrowser2
} else:macx {
    QMAKE_LFLAGS += -F$$DESTDIR
    LIBS += -framework neurolib \
        -framework automata \
        -framework qtpropertybrowser
} else {
    LIBS += -L$$DESTDIR \
        -lneurolib \
        -lautomata \
        -lqtpropertybrowser
}
