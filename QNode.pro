#-------------------------------------------------
#
# Project created by QtCreator 2017-11-09T17:07:28
#
#-------------------------------------------------

QT       += core gui network webengine webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qnode
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
    browserwindow.cpp \
    nodethread.cpp \
    nodeapi.cc \
    scriptobjects.cpp

HEADERS += \
    browserwindow.h \
    nodethread.h \
    nodeapi.h \
    common.h \
    scriptobjects.h

FORMS += \
    browserwindow.ui


INCLUDEPATH += $$PWD \
    $$PWD/deps/node/deps/v8/include \
    $$PWD/deps/node/deps/v8 \
    $$PWD/deps/node/deps/uv/include \
    $$PWD/deps/node/src

#LIBS += -L"$$PWD/deps/node/out/Release" \
#    $$PWD/deps/node/out/Release/obj.target/libnode.a \
#    $$PWD/deps/node/out/Release/obj.target/deps/gtest/libgtest.a \
#    $$PWD/deps/node/out/Release/obj.target/deps/cares/libcares.a \
#    $$PWD/deps/node/out/Release/obj.target/deps/openssl/libopenssl.a \
#    $$PWD/deps/node/out/Release/obj.target/deps/zlib/libzlib.a \
#    $$PWD/deps/node/out/Release/obj.target/deps/http_parser/libhttp_parser.a \
#    $$PWD/deps/node/out/Release/obj.target/deps/uv/libuv.a \
#    $$PWD/deps/node/out/Release/obj.target/deps/v8/src/libv8_libplatform.a \
#    $$PWD/deps/node/out/Release/obj.target/deps/v8/src/libv8_base.a \
#    $$PWD/deps/node/out/Release/obj.target/deps/v8/src/libv8_libbase.a \
#    $$PWD/deps/node/out/Release/obj.target/deps/v8/src/libv8_nosnapshot.a \
#    $$PWD/deps/node/out/Release/obj.target/deps/v8/src/libv8_snapshot.a \
#    $$PWD/deps/node/out/Release/obj.target/deps/v8/src/libv8_libsampler.a \
#    $$PWD/deps/node/out/Release/obj.target/tools/icu/libicudata.a \
#    $$PWD/deps/node/out/Release/obj.target/tools/icu/libicuucx.a \
#    $$PWD/deps/node/out/Release/obj.target/tools/icu/libicustubdata.a \
#    $$PWD/deps/node/out/Release/obj.target/tools/icu/libicui18n.a

unix:macx {
    LIBS+= -L$$PWD/deps/node/out/Release -lnode.59
}else {
    LIBS+= -L$$PWD/deps/node/out/Release/lib.target -lnode
}

QMAKE_RPATHDIR+= $ORIGIN:$ORIGIN/lib



CONFIG(release, debug|release) {
    OUTPUT_DIR = "output/release"
}
else {
    TARGET = $${TARGET}d
    OUTPUT_DIR = "output/debug"
}


QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-variable
QMAKE_CXXFLAGS += -Wno-unused-function
QMAKE_CXXFLAGS += -Wno-reorder
QMAKE_CXXFLAGS += -fpermissive

RESOURCES += \
    lib.qrc


