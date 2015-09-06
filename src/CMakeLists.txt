CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT(nxmhandler)

CMAKE_POLICY(SET CMP0020 NEW)
CMAKE_POLICY(SET CMP0043 NEW)


SET(nxmhandler_SRCS
    main.cpp
    handlerwindow.cpp
    addbinarydialog.cpp
    handlerstorage.cpp
  )

SET(nxmhandler_HDRS
    handlerwindow.h
    addbinarydialog.h
    handlerstorage.h
  )

SET(nxmhandler_FORMS
    handlerwindow.ui
    addbinarydialog.ui
  )

SET(nxmhandler_RCS
    version.rc)


# Qt5
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
FIND_PACKAGE(Qt5Widgets REQUIRED)
QT5_WRAP_UI(nxmhandler_UIHDRS ${nxmhandler_FORMS})

# Boost
SET(Boost_USE_STATIC_LIBS        ON)
SET(Boost_USE_MULTITHREADED      ON)
SET(Boost_USE_STATIC_RUNTIME    OFF)
FIND_PACKAGE(Boost REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

ADD_DEFINITIONS(-D_UNICODE -DUNICODE -DNOMINMAX)

SET(project_path "${CMAKE_SOURCE_DIR}/../.." CACHE PATH "path to the other mo projects")


INCLUDE_DIRECTORIES(${project_path}/modorganizer-uibase/src)

LINK_DIRECTORIES(${project_path}/modorganizer-uibase/build)

ADD_EXECUTABLE(nxmhandler WIN32 ${nxmhandler_HDRS} ${nxmhandler_SRCS} ${nxmhandler_UIHDRS} ${nxmhandler_RCS})
TARGET_LINK_LIBRARIES(nxmhandler
                      Qt5::Widgets
                      ${Boost_LIBRARIES}
                      uibase)



SET_TARGET_PROPERTIES(nxmhandler PROPERTIES COMPILE_FLAGS /GL)
SET_TARGET_PROPERTIES(nxmhandler PROPERTIES LINK_FLAGS_RELWITHDEBINFO
                      "/LTCG /INCREMENTAL:NO /LARGEADDRESSAWARE /OPT:REF /OPT:ICF")

QT5_USE_MODULES(nxmhandler Widgets)

###############
## Installation

SET(qt5bin ${Qt5Core_DIR}/../../../bin)

FIND_PROGRAM(WINDEPLOYQT_COMMAND windeployqt PATHS ${qt5bin} NO_DEFAULT_PATH)

INSTALL(TARGETS nxmhandler
        RUNTIME DESTINATION bin)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/nxmhandler.pdb
        DESTINATION pdb)

# use windeployqt.exe to install all required libraries
SET(windeploy_parameters --no-translations --no-plugins --libdir dlls --release-with-debug-info --no-compiler-runtime)
INSTALL(
    CODE
    "EXECUTE_PROCESS(
        COMMAND
        ${qt5bin}/windeployqt.exe nxmhandler.exe ${windeploy_parameters}
        WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin
    )"
)
