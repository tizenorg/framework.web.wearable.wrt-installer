#
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

SET(BACKLIB_SRCS
    backendlib.cpp
    ${PROJECT_SOURCE_DIR}/src/configuration_parser/widget_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/configuration_parser/parser_runner.cpp
    ${PROJECT_SOURCE_DIR}/src/configuration_parser/ignoring_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/configuration_parser/deny_all_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/configuration_parser/libiriwrapper.cpp
    ${PROJECT_SOURCE_DIR}/src/wrt-installer/language_subtag_rst_tree.cpp
)

PKG_CHECK_MODULES(WRT_BACKLIB_PKGS
    dpl-efl
    dpl-wrt-dao-ro
    dpl-wrt-dao-rw
    dpl-utils-efl
    pkgmgr-installer
    pkgmgr-types
    pkgmgr
    dlog
    libpcrecpp
    wrt-commons-i18n-dao-ro
    REQUIRED)

INCLUDE_DIRECTORIES(
    ${WRT_BACKLIB_PKGS_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/configuration_parser
)

ADD_LIBRARY(${TARGET_BACKEND_LIB} SHARED
    ${BACKLIB_SRCS}
)

TARGET_LINK_LIBRARIES(${TARGET_BACKEND_LIB}
    ${WRT_BACKLIB_PKGS_LIBRARIES}
)

SET_TARGET_PROPERTIES(${TARGET_BACKEND_LIB} PROPERTIES
    LINK_FLAGS "-Wl,--as-needed -Wl,--hash-style=both"
)

INSTALL(TARGETS ${TARGET_BACKEND_LIB}
    DESTINATION etc/package-manager/backendlib
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

#SYMLINK
set(SYMLINK_USE OFF)
set(SYMLINK_DEST "${CMAKE_INSTALL_PREFIX}/etc/package-manager")

IF(SYMLINK_USE)
    ADD_CUSTOM_COMMAND(OUTPUT ${SYMLINK_DEST}/backend/wgt
        COMMAND mkdir
        ARGS -p ${SYMLINK_DEST}/backend
        COMMAND ln
        ARGS -sf ${CMAKE_INSTALL_PREFIX}/bin/${BACKEND} ${SYMLINK_DEST}/backend/wgt
        DEPENDS ${BACKEND}
        )
    ADD_CUSTOM_TARGET(test_symlinks ALL
        DEPENDS ${SYMLINK_DEST}/backend/wgt
        )
ENDIF(SYMLINK_USE)
