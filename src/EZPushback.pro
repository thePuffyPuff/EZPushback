#-------------------------------------------------
#
# Project created by QtCreator 2012-02-19T15:55:39
#
#-------------------------------------------------

TEMPLATE = lib
QT -= core gui

CONFIG += warn_on plugin release
CONFIG -= exceptions qt rtti debug

VERSION =

INCLUDEPATH += ../../XPSDK212/SDK/CHeaders/XPLM
INCLUDEPATH += ../../XPSDK212/SDK/CHeaders/Wrappers
INCLUDEPATH += ../../XPSDK212/SDK/CHeaders/Widgets

# Defined to use X-Plane SDK 2.0 capabilities - no backward compatibility before 9.0
DEFINES += XPLM200 XPLM210

win32 {
    # using MSVC 10.0 & SDK 7.1 profile
    QMAKE_CFLAGS += -TP
    DEFINES += APL=0 IBM=1 LIN=0
    DEFINES += _CRT_SECURE_NO_WARNINGS=1
    INCLUDEPATH += "C:/Program Files/Microsoft SDKs/Windows/v7.1/Include"
    LIBS += -L../../XPSDK212/SDK/Libraries/Win
    TARGET = win
    TARGET_EXT = .xpl
    QMAKE_POST_LINK += del $(DESTDIR)win.exp $$escape_expand(\n\t)
    QMAKE_POST_LINK += del $(DESTDIR)win.lib $$escape_expand(\n\t)
}

IBM_X86 {
    message("IBM_X86")
    DEFINES += IBM_X86=1
    DESTDIR=../plugin/EZPushback/32
    OBJECTS_DIR=release/32
    LIBS += -L"C:/Program Files/Microsoft SDKs/Windows/v7.1/Lib"
    LIBS += -lWS2_32
    LIBS += -lXPLM -lXPWidgets
}

IBM_X64 {
    message("IBM_X64")
    DEFINES += IBM_X64=1
    DESTDIR=../plugin/EZPushback/64
    OBJECTS_DIR=release/64
    LIBS += -L"C:/Program Files/Microsoft SDKs/Windows/v7.1/Lib/x64"
    LIBS += -lWS2_32
    LIBS += -lXPLM_64 -lXPWidgets_64
}

macx {
    message("macx")
    # using gcc 4 profile
    DEFINES += APL=1 IBM=0 LIN=0
    DEFINES += MACX=1
    DESTDIR=../plugin/EZPushback
    QMAKE_LFLAGS += -F"../../XPSDK212/SDK/Libraries/Mac"
    QMAKE_CFLAGS += -fvisibility=hidden
    CONFIG += x86 x86_64
    LIBS += -framework XPLM
    LIBS += -framework XPWidgets
    LIBS += -framework CoreFoundation
    TARGET = mac
    QMAKE_POST_LINK += mv $(DESTDIR)libmac.dylib $(DESTDIR)mac.xpl &
    QMAKE_POST_LINK += nm $(DESTDIR)mac.xpl | grep \"T \"
}

unix:!macx {
    message("linux")
    DEFINES += APL=0 IBM=0 LIN=1
    TARGET = lin.xpl
    QMAKE_CXXFLAGS += -fvisibility=hidden
    DEFINES += XPLANE_PLUGIN_DIR=../../../xp10/Resource/plugins
    QMAKE_POST_LINK += mv $(DESTDIR)liblin.xpl.so $(DESTDIR)lin.xpl &
    QMAKE_POST_LINK += ldd $(DESTDIR)lin.xpl &
}

LINUX_X64 {
    message("LINUX_X64")
    DESTDIR=../plugin/EZPushback/64
}

LINUX_X86 {
    message("LINUX_X86")
    DESTDIR=../plugin/EZPushback/32
}


SOURCES += \
    ezpushback.c \
    datarefs.c \
    debug.c \
    ui.c \
    prefs.c

HEADERS += \
    ezpushback.h
