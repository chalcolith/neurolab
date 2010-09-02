CONFIG += debug_and_release
QT += gui
QT += svg

TARGET = neurogui
TEMPLATE = lib

CONFIG += resources

macx { CONFIG += lib_bundle }
macx { CONFIG += x86 }
macx { CONFIG -= x86_64 }

include(../version.txt)

DEFINES += NEUROGUI_LIBRARY

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
    neurotextitem.cpp \
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
    neurotextitem.h \
    labdatafile.h \
    aboutdialog.h

FORMS += mainwindow.ui \
    labeldialog.ui \
    filedirtydialog.ui \
    aboutdialog.ui

RESOURCES += neurogui.qrc

CONFIG(release, debug|release) { BUILDDIR=release }
CONFIG(debug, debug|release) { BUILDDIR=debug }

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

HEADERS += \
    subnetworkitem.h

SOURCES += \
    subnetworkitem.cpp

HEADERS += \
    subconnectionitem.h

SOURCES += \
    subconnectionitem.cpp

HEADERS += \
    labexception.h

SOURCES += \
    labexception.cpp

HEADERS += \
    mixinarrow.h

SOURCES += \
    mixinarrow.cpp
