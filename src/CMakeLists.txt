cmake_minimum_required(VERSION 3.16)

add_executable(nxmhandler)
mo2_configure_executable(nxmhandler
	WARNINGS OFF
	PRIVATE_DEPENDS uibase)
mo2_install_target(nxmhandler)

mo2_deploy_qt(BINARIES nxmhandler.exe NOPLUGINS)
