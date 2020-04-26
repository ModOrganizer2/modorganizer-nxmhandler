cmake_minimum_required(VERSION 3.16)

project(nxmhandler)
set(project_type exe)
set(enable_warnings OFF)

include(../cmake_common/project.cmake)
add_subdirectory(src)
