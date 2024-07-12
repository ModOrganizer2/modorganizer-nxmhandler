cmake_minimum_required(VERSION 3.16)

find_package(mo2-cmake CONFIG REQUIRED)
find_package(mo2-uibase CONFIG REQUIRED)
find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Widgets)

add_executable(nxmhandler)
mo2_configure_target(nxmhandler)
target_link_libraries(nxmhandler PRIVATE Qt6::Core Qt6::Widgets mo2::uibase)

install(FILES $<TARGET_FILE:nxmhandler> DESTINATION bin)
install(FILES $<TARGET_PDB_FILE:nxmhandler> DESTINATION pdb)

mo2_deploy_qt(BINARIES nxmhandler.exe NOPLUGINS)
