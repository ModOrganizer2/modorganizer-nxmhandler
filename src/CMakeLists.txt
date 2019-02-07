CMAKE_MINIMUM_REQUIRED (VERSION 2.8.11)

CMAKE_POLICY(SET CMP0020 NEW)
CMAKE_POLICY(SET CMP0043 NEW)


SET(nxmhandler_SRCS
    main.cpp
    handlerwindow.cpp
    addbinarydialog.cpp
    handlerstorage.cpp
    logger.cpp
  )

SET(nxmhandler_HDRS
    handlerwindow.h
    addbinarydialog.h
    handlerstorage.h
    logger.h
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

SET(default_project_path "${CMAKE_SOURCE_DIR}/..")
GET_FILENAME_COMPONENT(${default_project_path} ${default_project_path} REALPATH)

SET(project_path "${default_project_path}" CACHE PATH "path to the other mo projects")
SET(lib_path "${project_path}/../../install/libs")

MESSAGE(STATUS "${default_project_path} - ${project_path}")

INCLUDE_DIRECTORIES(${project_path}/uibase/src)

LINK_DIRECTORIES(${lib_path})

ADD_EXECUTABLE(nxmhandler WIN32 ${nxmhandler_HDRS} ${nxmhandler_SRCS} ${nxmhandler_UIHDRS} ${nxmhandler_RCS})
TARGET_LINK_LIBRARIES(nxmhandler
                      Qt5::Widgets
                      ${Boost_LIBRARIES}
                      uibase)

IF (MSVC)
  SET_TARGET_PROPERTIES(nxmhandler PROPERTIES COMPILE_FLAGS "/std:c++latest")
ENDIF()
IF (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  # 32 bits
  SET_TARGET_PROPERTIES(nxmhandler PROPERTIES LINK_FLAGS "/LARGEADDRESSAWARE")
ENDIF()

IF (NOT "${OPTIMIZE_COMPILE_FLAGS}" STREQUAL "")
  SET_TARGET_PROPERTIES(${PROJ_NAME} PROPERTIES COMPILE_FLAGS_RELWITHDEBINFO ${OPTIMIZE_COMPILE_FLAGS})
ENDIF()
IF (NOT "${OPTIMIZE_LINK_FLAGS}" STREQUAL "")
  SET_TARGET_PROPERTIES(${PROJ_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO ${OPTIMIZE_LINK_FLAGS})
ENDIF()

###############
## Installation

SET(qt5bin ${Qt5Core_DIR}/../../../bin)

FIND_PROGRAM(WINDEPLOYQT_COMMAND windeployqt PATHS ${qt5bin} NO_DEFAULT_PATH)

INSTALL(TARGETS nxmhandler
        RUNTIME DESTINATION bin)
INSTALL(FILES $<TARGET_PDB_FILE:nxmhandler>
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
