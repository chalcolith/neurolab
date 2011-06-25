CONFIG += debug_and_release
QT += gui

TARGET = griditems
TEMPLATE = lib

macx { CONFIG += lib_bundle }
macx { CONFIG += x86 }
macx { CONFIG -= x86_64 }

include(../version.txt)

INCLUDEPATH += ../thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/src

DEFINES += GRIDITEMS_LIBRARY

SOURCES += neurogriditem.cpp \
           gridedgeitem.cpp \
           multigridioitem.cpp \
           multilink.cpp \
           multiitem.cpp

HEADERS += griditems_global.h \
           neurogriditem.h\
           gridedgeitem.h \
           multigridioitem.h \
           multilink.h \
           multiitem.h

CONFIG(release, debug|release) { BUILDDIR=release }
CONFIG(debug, debug|release) {
    BUILDDIR=debug
    DEFINES += DEBUG
}

macx { DESTDIR = $$OUT_PWD/../$$BUILDDIR/neurolab.app/Contents/Frameworks }
else { DESTDIR = $$OUT_PWD/../$$BUILDDIR/plugins }
TEMPDIR = $$OUT_PWD/$$BUILDDIR

OBJECTS_DIR = $$TEMPDIR
MOC_DIR = $$TEMPDIR
UI_DIR = $$TEMPDIR
RCC_DIR = $$TEMPDIR

win32 {
    LIBS += -L$$DESTDIR/.. \
        -lcommon1 \
        -lneurogui1 \
        -lneurolib1 \
        -lautomata1 \
        -lqtpropertybrowser2
} else:macx {
    QMAKE_LFLAGS += -F$$DESTDIR
    LIBS += -framework common \
        -framework neurogui \
        -framework neurolib \
        -framework automata \
        -framework qtpropertybrowser
} else {
    LIBS += -L$$DESTDIR/.. \
        -lcommon \
        -lneurogui \
        -lneurolib \
        -lautomata \
        -lqtpropertybrowser
}

RESOURCES += \
    griditems.qrc
