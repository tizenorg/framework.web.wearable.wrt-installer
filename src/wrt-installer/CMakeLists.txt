# Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
#
# @file     CMakeLists.txt
# @author   Lukasz Wrzosek (l.wrzosek@samsung.com)
# @version     1.0
#

SET(WRT_INSTALLER_DIR
    ${INSTALLER_SRC_DIR}/wrt-installer
    )

SET(PKG_MANAGER_DIR
    ${INSTALLER_SRC_DIR}/pkg-manager
    )

SET(WRT_INSTALLER_SOURCES
    ${WRT_INSTALLER_DIR}/wrt-installer.cpp
    ${WRT_INSTALLER_DIR}/installer_callbacks_translate.cpp
    ${WRT_INSTALLER_DIR}/plugin_utils.cpp
    ${WRT_INSTALLER_DIR}/language_subtag_rst_tree.cpp
    ${WRT_INSTALLER_DIR}/installer_main_thread.cpp
    ${WRT_INSTALLER_DIR}/command_parser.cpp
    ${PKG_MANAGER_DIR}/pkgmgr_signal.cpp
)

PKG_CHECK_MODULES(WRT_INSTALLER_DEPS
	capi-appfw-application
    pkgmgr-installer
    libpcrecpp
    pkgmgr-info
    pkgmgr
    security-install
    capi-security-privilege-manager
    wrt-commons-i18n-dao-ro
    capi-system-device
    REQUIRED)

INCLUDE_DIRECTORIES(
    ${PKG_MANAGER_DIR}
    ${WRT_INSTALLER_DEP_INCLUDES}
    ${WRT_INSTALLER_INCLUDES}
    ${WRT_INSTALLER_DEPS_INCLUDE_DIRS}
)

ADD_EXECUTABLE(${TARGET_INSTALLER}
    ${TARGET_INSTALLER_STATIC_SRC}
    ${WRT_INSTALLER_SOURCES}
)

ADD_DEFINITIONS(${WRT_INSTALLER_DEPS_CFLAGS})

ADD_DEFINITIONS("-fPIE")

TARGET_LINK_LIBRARIES(${TARGET_INSTALLER}
    ${TARGET_INSTALLER_STATIC}
    ${WRT_INSTALLER_DEPS_LIBRARIES}
)


SET_TARGET_PROPERTIES(${TARGET_INSTALLER} PROPERTIES
    LINK_FLAGS "-Wl,--as-needed -pie -Wl,--hash-style=both"
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH_USE_LINK_PATH ON
)

INSTALL(TARGETS ${TARGET_INSTALLER} DESTINATION bin)
