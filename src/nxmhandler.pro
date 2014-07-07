#-------------------------------------------------
#
# Project created by QtCreator 2013-06-16T16:14:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = nxmhandler
TEMPLATE = app


SOURCES += main.cpp \
    handlerwindow.cpp \
    addbinarydialog.cpp \
    handlerstorage.cpp

HEADERS  += \
    handlerwindow.h \
    addbinarydialog.h \
    handlerstorage.h

FORMS    += \
    handlerwindow.ui \
    addbinarydialog.ui

INCLUDEPATH += ../uibase "$(BOOSTPATH)"
LIBS += -luibase -lshell32

CONFIG(debug, debug|release) {
  OUTDIR = $$OUT_PWD/debug
  DSTDIR = $$PWD/../../outputd
  LIBS += -L$$OUT_PWD/../uibase/debug
} else {
  OUTDIR = $$OUT_PWD/release
  DSTDIR = $$PWD/../../output
  LIBS += -L$$OUT_PWD/../uibase/release
}

OUTDIR ~= s,/,$$QMAKE_DIR_SEP,g
DSTDIR ~= s,/,$$QMAKE_DIR_SEP,g

QMAKE_POST_LINK += xcopy /y /I $$quote($$OUTDIR\\nxmhandler*.exe) $$quote($$DSTDIR) $$escape_expand(\\n)
