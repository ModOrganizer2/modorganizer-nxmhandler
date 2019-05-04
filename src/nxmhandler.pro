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
    logger.cpp

HEADERS  += \
    handlerwindow.h \
    addbinarydialog.h \
    handlerstorage.h \
    logger.h

FORMS    += \
    handlerwindow.ui \
    addbinarydialog.ui

INCLUDEPATH += ../uibase "$${BOOSTPATH}"
LIBS += -luibase -lshell32

debug:  LIBS += -L$$OUT_PWD/../uibase/debug
release:LIBS += -L$$OUT_PWD/../uibase/release

release:QMAKE_CXXFLAGS += /Zi /GL
release:QMAKE_LFLAGS += /DEBUG /LTCG /OPT:REF /OPT:ICF

QMAKE_POST_LINK += xcopy /y /I $$quote($$SRCDIR\\nxmhandler*.exe) $$quote($$DSTDIR) $$escape_expand(\\n)

OTHER_FILES += \
    SConscript\
    app_icon.rc

APP_ICON +=\
    app_icon.rc

RC_FILE +=\
    app_icon.rc
