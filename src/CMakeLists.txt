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

SET(TARGET_INSTALLER "wrt-installer")

OPTION(LB_SUPPORT "lb support" OFF)

SET(INSTALLER_SRC_DIR
    ${PROJECT_SOURCE_DIR}/src
    )

SET(INSTALLER_CONFIG_PARSER
    ${INSTALLER_SRC_DIR}/configuration_parser
    )

SET(INSTALLER_JOBS
    ${INSTALLER_SRC_DIR}/jobs
    )

SET(INSTALLER_INCLUDES
    ${INSTALLER_SRC_DIR}
    ${INSTALLER_SRC_DIR}/logic
    ${INSTALLER_SRC_DIR}/jobs
    ${INSTALLER_SRC_DIR}/jobs/plugin_install
    ${INSTALLER_SRC_DIR}/jobs/widget_install
    ${INSTALLER_SRC_DIR}/jobs/widget_uninstall
    ${INSTALLER_SRC_DIR}/misc
    ${INSTALLER_SRC_DIR}/configuration_parser
    ${INSTALLER_SRC_DIR}/wrt-installer
    ${INSTALLER_SRC_DIR}/commons
    ${INSTALLER_SRC_DIR}/pkg-manager
)

SET(INSTALLER_SOURCES
    ${INSTALLER_CONFIG_PARSER}/widget_parser.cpp
    ${INSTALLER_CONFIG_PARSER}/parser_runner.cpp
    ${INSTALLER_CONFIG_PARSER}/ignoring_parser.cpp
    ${INSTALLER_CONFIG_PARSER}/deny_all_parser.cpp
    ${INSTALLER_CONFIG_PARSER}/libiriwrapper.cpp
    ${INSTALLER_JOBS}/job.cpp
    ${INSTALLER_JOBS}/plugin_install/job_plugin_install.cpp
    ${INSTALLER_JOBS}/plugin_install/plugin_install_task.cpp
    ${INSTALLER_JOBS}/plugin_install/plugin_objects.cpp
    ${INSTALLER_JOBS}/plugin_install/plugin_metafile_reader.cpp
    ${INSTALLER_JOBS}/widget_install/job_widget_install.cpp
    ${INSTALLER_JOBS}/widget_install/manifest.cpp
    ${INSTALLER_JOBS}/widget_install/task_commons.cpp
    ${INSTALLER_JOBS}/widget_install/task_process_config.cpp
    ${INSTALLER_JOBS}/widget_install/task_database.cpp
    ${INSTALLER_JOBS}/widget_install/task_configuration.cpp
    ${INSTALLER_JOBS}/widget_install/ace_registration.cpp
    ${INSTALLER_JOBS}/widget_install/task_file_manipulation.cpp
    ${INSTALLER_JOBS}/widget_install/task_user_data_manipulation.cpp
    ${INSTALLER_JOBS}/widget_install/task_smack.cpp
    ${INSTALLER_JOBS}/widget_install/task_ace_check.cpp
    ${INSTALLER_JOBS}/widget_install/task_manifest_file.cpp
    ${INSTALLER_JOBS}/widget_install/task_certify.cpp
    ${INSTALLER_JOBS}/widget_install/task_certify_level.cpp
    ${INSTALLER_JOBS}/widget_install/task_prepare_files.cpp
    ${INSTALLER_JOBS}/widget_install/task_recovery.cpp
    ${INSTALLER_JOBS}/widget_install/task_install_ospsvc.cpp
    ${INSTALLER_JOBS}/widget_install/task_update_files.cpp
    ${INSTALLER_JOBS}/widget_install/task_remove_backup.cpp
    ${INSTALLER_JOBS}/widget_install/task_encrypt_resource.cpp
    ${INSTALLER_JOBS}/widget_install/task_prepare_reinstall.cpp
    ${INSTALLER_JOBS}/widget_install/task_pkg_info_update.cpp
    ${INSTALLER_JOBS}/widget_install/task_status_check.cpp
    ${INSTALLER_JOBS}/widget_install/task_recovery.cpp
    ${INSTALLER_JOBS}/widget_install/widget_security.cpp
    ${INSTALLER_JOBS}/widget_install/widget_update_info.cpp
    ${INSTALLER_JOBS}/widget_install/directory_api.cpp
    ${INSTALLER_JOBS}/widget_install/widget_unzip.cpp
    ${INSTALLER_JOBS}/widget_uninstall/job_widget_uninstall.cpp
    ${INSTALLER_JOBS}/widget_uninstall/task_check.cpp
    ${INSTALLER_JOBS}/widget_uninstall/task_remove_files.cpp
    ${INSTALLER_JOBS}/widget_uninstall/task_remove_custom_handlers.cpp
    ${INSTALLER_JOBS}/widget_uninstall/task_db_update.cpp
    ${INSTALLER_JOBS}/widget_uninstall/task_smack.cpp
    ${INSTALLER_JOBS}/widget_uninstall/task_uninstall_ospsvc.cpp
    ${INSTALLER_JOBS}/widget_uninstall/task_delete_pkginfo.cpp
    ${INSTALLER_SRC_DIR}/logic/installer_logic.cpp
    ${INSTALLER_SRC_DIR}/logic/installer_controller.cpp
    ${INSTALLER_SRC_DIR}/misc/wac_widget_id.cpp
    ${INSTALLER_SRC_DIR}/misc/feature_logic.cpp
    ${INSTALLER_SRC_DIR}/misc/libxml_utils.cpp
    ${INSTALLER_SRC_DIR}/misc/widget_location.cpp
    ${INSTALLER_SRC_DIR}/misc/widget_install_to_external.cpp
    ${INSTALLER_SRC_DIR}/misc/plugin_path.cpp
    ${INSTALLER_SRC_DIR}/pkg-manager/pkgmgr_signal.cpp
    )

IF(LB_SUPPORT)
    SET(INSTALLER_SOURCES
        ${INSTALLER_SOURCES}
        )
    MESSAGE(STATUS "adding definition -DLB_SUPPORT")
    ADD_DEFINITIONS("-DLB_SUPPORT")
ENDIF(LB_SUPPORT)

MESSAGE(STATUS "add -DSEP_INSTALLER")
ADD_DEFINITIONS("-DSEP_INSTALLER")


PKG_CHECK_MODULES(INSTALLER_STATIC_DEP
    dpl-efl
    dpl-event-efl
    dpl-utils-efl
    dpl-wrt-dao-ro
    dpl-wrt-dao-rw
    wrt-commons-custom-handler-dao-rw
    wrt-commons-security-origin-dao
    wrt-commons-widget-interface-dao
    wrt-plugins-types
    pkgmgr-installer
    pkgmgr-parser
    pkgmgr-info
    web-provider
    libsmack
    REQUIRED
)

PKG_CHECK_MODULES(SYS_INSTALLER_STATIC_DEP
    appsvc
    libxml-2.0
    openssl
    cert-svc-vcore
    security-install
    ecore-x
    xmlsec1
    libidn
    libiri
    libpcrecpp
    ail
    elementary
    shortcut
    capi-appfw-app-manager
    app2sd
    libprivilege-control
    storage
    REQUIRED
)

INCLUDE_DIRECTORIES( SYSTEM ${SYS_INSTALLER_STATIC_DEP_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(
    ${INSTALLER_DEP_INCLUDES}
    ${INSTALLER_INCLUDES}
    ${INSTALLER_STATIC_DEP_INCLUDE_DIRS}
    ${OSP_APPFW_INCLUDES}
    )

ADD_LIBRARY(${TARGET_INSTALLER_STATIC} STATIC
    ${INSTALLER_SOURCES}
    )

ADD_DEFINITIONS(${INSTALLER_STATIC_DEP_CFLAGS})
ADD_DEFINITIONS(${INSTALLER_STATIC_DEP_CFLAGS_OTHERS})
ADD_DEFINITIONS(${SYS_INSTALLER_STATIC_DEP_CFLAGS})
ADD_DEFINITIONS(${SYS_INSTALLER_STATIC_DEP_CFLAGS_OTHERS})

TARGET_LINK_LIBRARIES(${TARGET_INSTALLER_STATIC}
    ${INSTALLER_STATIC_DEP_LIBRARIES} "-ldl"
    ${SYS_INSTALLER_STATIC_DEP_LIBRARIES} "-ldl"
    )

#for encryption
TARGET_LINK_LIBRARIES(${TARGET_INSTALLER_STATIC} "-lss-client" )

ADD_SUBDIRECTORY(pkg-manager)
ADD_SUBDIRECTORY(wrt-installer)
