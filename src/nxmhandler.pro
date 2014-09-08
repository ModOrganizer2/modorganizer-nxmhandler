#-------------------------------------------------
#
# Project created by QtCreator 2013-06-16T16:14:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = nxmhandler
TEMPLATE = app

!include(../LocalPaths.pri) {
  message("paths to required libraries need to be set up in LocalPaths.pri")
}


SOURCES += main.cpp \
    handlerwindow.cpp \
    addbinarydialog.cpp \
    handlerstorage.cpp \
    ../uibase/json.cpp

HEADERS  += \
    handlerwindow.h \
    addbinarydialog.h \
    handlerstorage.h

FORMS    += \
    handlerwindow.ui \
    addbinarydialog.ui

INCLUDEPATH += ../uibase "$${BOOSTPATH}"
LIBS += -luibase -lshell32

debug:  LIBS += -L$$OUT_PWD/../uibase/debug
release:LIBS += -L$$OUT_PWD/../uibase/release

QMAKE_POST_LINK += xcopy /y /I $$quote($$SRCDIR\\nxmhandler*.exe) $$quote($$DSTDIR) $$escape_expand(\\n)
