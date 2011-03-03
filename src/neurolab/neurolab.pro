CONFIG += debug_and_release
QT += gui
QT += svg

TARGET = neurolab
TEMPLATE = app

macx { CONFIG += x86 }
macx { CONFIG -= x86_64 }

include(../version.txt)

INCLUDEPATH += ../thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/src

SOURCES += main.cpp

CONFIG(release, debug|release) { BUILDDIR=release }
CONFIG(debug, debug|release) { BUILDDIR=debug }

DESTDIR = $$OUT_PWD/../$$BUILDDIR
TEMPDIR = $$OUT_PWD/$$BUILDDIR

OBJECTS_DIR = $$TEMPDIR
MOC_DIR = $$TEMPDIR
UI_DIR = $$TEMPDIR
RCC_DIR = $$TEMPDIR

win32 {
    LIBS += -L$$DESTDIR \
        -lcommon1 \
        -lneurogui1 \
        -lneurolib1 \
        -lautomata1 \
        -lqtpropertybrowser2
} else:macx {
    QMAKE_LFLAGS += -F$$DESTDIR/neurolab.app/Contents/Frameworks
    LIBS += \
        -framework common \
        -framework neurogui \
        -framework neurolib \
        -framework automata \
        -framework qtpropertybrowser
} else {
    LIBS += -L$$DESTDIR \
        -lcommon \
        -lneurogui \
        -lneurolib \
        -lautomata \
        -lqtpropertybrowser
}
