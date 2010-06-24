QT += gui
QT += svg

TARGET = neurolab
TEMPLATE = app

macx { CONFIG += x86 x86_64 }

include(../version.txt)

INCLUDEPATH += ../thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/src

SOURCES += main.cpp

build_pass:release { BUILDDIR=$$OUT_PWD/release }
build_pass:debug { BUILDDIR=$$OUT_PWD/debug }

DESTDIR = $$BUILDDIR
OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR

macx { QMAKE_LFLAGS += -F$$DESTDIR/neurolab.app/Contents/Frameworks }

win32 {
    LIBS += -L../$$BUILDDIR \
        -lneurogui1 \
        -lneurolib1 \
        -lautomata1 \
        -lqtpropertybrowser2
} else:macx {
    LIBS += -framework neurogui \
        -framework neurolib \
        -framework automata \
        -framework qtpropertybrowser
} else {
    LIBS += -L../$$BUILDDIR \
        -lneurogui \
        -lneurolib \
        -lautomata \
        -lqtpropertybrowser
}
