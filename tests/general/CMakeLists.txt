# Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
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
# @file        CMakeLists.txt
# @author      Karol Pawlowski (k.pawlowski@samsung.com)
#

PKG_CHECK_MODULES(COMMON_LIB_PKGS
    dbus-1
    libpcrecpp
    dpl-efl
    dpl-test-efl
    dpl-utils-efl
    dpl-wrt-dao-ro
    dpl-event-efl
    cert-svc-vcore
    xmlsec1
    libiri
    REQUIRED
    )

pkg_search_module(dpl REQUIRED dpl-efl)
pkg_search_module(dpl-test REQUIRED dpl-test-efl)

SET(WRT_TEST_LIBRARY "wrt-tests-libs")

include_directories(
  ${dpl_INCLUDE_DIRS}
  ${dpl-test_INCLUDE_DIRS}
  ${ace-client_INCLUDE_DIRS}
)

SET(COMMON_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}")

SET_PROPERTY(GLOBAL APPEND PROPERTY COMMON_TESTS_LIBRARY ${WRT_TEST_LIBRARY})

SET_PROPERTY(GLOBAL APPEND PROPERTY TESTS_INCLUDE_DIRS ${COMMON_LIB_PKGS_INCLUDE_DIRS})

SET_PROPERTY(GLOBAL APPEND PROPERTY TESTS_LIBRARY_DIRS ${COMMON_LIB_PKGS_LIBRARY_DIRS})

SET_PROPERTY(GLOBAL APPEND PROPERTY TESTS_LIBRARIES ${COMMON_LIB_PKGS_LIBRARIES})

SET(WRT_DETAIL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/InstallerWrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ManifestFile.cpp
)

INCLUDE_DIRECTORIES(${COMMON_INCLUDES})
INCLUDE_DIRECTORIES(${COMMON_LIB_PKGS_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(SYSTEM ${SYS_INSTALLER_STATIC_DEP_INCLUDE_DIRS})

ADD_LIBRARY(${WRT_TEST_LIBRARY} STATIC ${WRT_DETAIL_SOURCES})


SET(INSTALLER_TESTS_TARGET "wrt-installer-tests-general")

PKG_CHECK_MODULES(INSTALLER_TESTS_DEPS
    wrt-commons-i18n-dao-ro
    REQUIRED)

SET(INSTALLER_TESTS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/TestInit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ManifestTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BackgroundPageTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/NPluginsInstallTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParsingAccountTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParsingAllowNavigationTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParsingAppWidgetTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParsingCategoryTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParsingContentTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParsingCspTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParsingMetadataTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParsingTizenAppcontrolTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParsingTizenPrivilegeTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParsingSplashImgTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WidgetUpdateTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PluginsInstallation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LanguageSubtagRstTreeTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WidgetInstallManifestTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WidgetLocationTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AceRegistrationTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParserRunnerTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TaskConfigurationTests.cpp
    ${CMAKE_SOURCE_DIR}/src/wrt-installer/language_subtag_rst_tree.cpp
    ${CMAKE_SOURCE_DIR}/src/wrt-installer/installer_callbacks_translate.cpp
)

SET(INSTALLER_TESTS_INCLUDE_DIRS
    ${INSTALLER_TESTS_DEPS_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/wrt-installer
    ${CMAKE_SOURCE_DIR}/src/configuration_parser
    ${CMAKE_SOURCE_DIR}/src/commons
    ${CMAKE_SOURCE_DIR}/src/jobs
    ${CMAKE_SOURCE_DIR}/src/jobs/widget_install
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/misc
    ${CMAKE_SOURCE_DIR}/src/pkg-manager
)

# Functions used to build test targets (proper sources, includes, libs are
# added automatically)
FUNCTION(WRT_TEST_BUILD TARGET_NAME)
    # get include dirs global property
    GET_PROPERTY(INCLUDE_DIRS GLOBAL PROPERTY TESTS_INCLUDE_DIRS)
    GET_PROPERTY(TEST_INCLUDE_DIRS GLOBAL PROPERTY ${TARGET_NAME}_INCLUDE_DIRS)
    INCLUDE_DIRECTORIES(
        ${INCLUDE_DIRS}
        ${TEST_INCLUDE_DIRS}
    )

    # get library dirs global property
    GET_PROPERTY(LIBRARY_DIRS GLOBAL PROPERTY TESTS_LIBRARY_DIRS)
    GET_PROPERTY(TEST_LIBRARY_DIRS GLOBAL PROPERTY ${TARGET_NAME}_LIBRARY_DIRS)
    LINK_DIRECTORIES(
        ${LIBRARY_DIRS}
        ${TEST_LIBRARY_DIRS}
    )

    SET(SOURCES "${ARGN}")
    ADD_EXECUTABLE("${TARGET_NAME}" ${SOURCES})

    # get link libraries global property
    GET_PROPERTY(LINK_LIBRARIES GLOBAL PROPERTY TESTS_LIBRARIES)
    GET_PROPERTY(TEST_LIBRARIES GLOBAL PROPERTY ${TARGET_NAME}_LIBRARIES)
    TARGET_LINK_LIBRARIES("${TARGET_NAME}"
        ${LINK_LIBRARIES}
        ${TEST_LIBRARIES}
    )
ENDFUNCTION(WRT_TEST_BUILD)

FUNCTION(WRT_TEST_INSTALL)
    SET_TARGET_PROPERTIES(${ARGV} PROPERTIES
        BUILD_WITH_INSTALL_RPATH ON
        INSTALL_RPATH_USE_LINK_PATH ON
    )
    INSTALL(TARGETS ${ARGV}
        DESTINATION bin
        PERMISSIONS OWNER_READ
                    OWNER_WRITE
                    OWNER_EXECUTE
                    GROUP_READ
                    GROUP_EXECUTE
                    WORLD_READ
                    WORLD_EXECUTE
    )
ENDFUNCTION(WRT_TEST_INSTALL)

FUNCTION(WRT_TEST_INCLUDE_DIRS TARGET_NAME)
    SET_PROPERTY(GLOBAL APPEND PROPERTY ${TARGET_NAME}_INCLUDE_DIRS ${ARGN})
ENDFUNCTION(WRT_TEST_INCLUDE_DIRS)

WRT_TEST_INCLUDE_DIRS(${INSTALLER_TESTS_TARGET} ${INSTALLER_TESTS_INCLUDE_DIRS})
WRT_TEST_BUILD(${INSTALLER_TESTS_TARGET} ${INSTALLER_TESTS_SOURCES})
WRT_TEST_INSTALL(${INSTALLER_TESTS_TARGET})
target_link_libraries(${INSTALLER_TESTS_TARGET}
  ${dpl_LIBRARIES}
  ${dpl-test_LIBRARIES}
  ${WRT_TEST_LIBRARY}
  ${TARGET_CORE_MODULE_LIB}
  ${COMMON_LIB_PKGS_LIBRARIES}
  ${INSTALLER_TESTS_DEPS_LIBRARIES}
  ${TARGET_INSTALLER_STATIC}
)

#widgets
FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/widgets/*.wgt")
INSTALL(FILES ${files} "${CMAKE_CURRENT_SOURCE_DIR}/widgets/widgetInDir.tar" DESTINATION /opt/share/widget/tests/installer/widgets/)

FILE(GLOB files_conf "${CMAKE_CURRENT_SOURCE_DIR}/configs/*.xml")
INSTALL(FILES ${files_conf} DESTINATION /opt/share/widget/tests/installer/configs/)
