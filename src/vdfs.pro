TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    buffer.cpp \
    disk.cpp \
    shell.cpp \
    vdfs.cpp \
    utils.cpp \
    filesystem.cpp \
    inode.cpp \
    file.cpp

HEADERS += \
    buffer.h \
    disk.h \
    shell.h \
    vdfs.h \
    utils.h \
    filesystem.h \
    inode.h \
    file.h

STATECHARTS +=

DISTFILES +=
