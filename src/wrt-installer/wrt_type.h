/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/**
 * @file        wrt_type.h
 * @author      jihoon Chung (jihoon.Chung@samsung.com)
 * @version     1.0
 * @brief       This file contains declarations of wrt api
 */

/*
 * @defgroup wrt_engine_group WebRunTime engine Library
 * @ingroup internet_FW
 * Functions to APIs to access wrt-engine
 */

#ifndef WRT_TYPE_H_
#define WRT_TYPE_H_

#include <stdbool.h>
#include <stddef.h>

#define WRT_DEPRECATED __attribute__((deprecated))

typedef enum
{
    /* Generic success */
    WRT_SUCCESS = 0,                /*< Success*/

    /* pkgmgr error */
    WRT_INSTALLER_ERROR_PACKAGE_NOT_FOUND,      ///<
    WRT_INSTALLER_ERROR_PACKAGE_INVALID,        ///< invalid widget package
    WRT_INSTALLER_ERROR_PACKAGE_LOWER_VERSION,  ///< given version is lower than existing version
    WRT_INSTALLER_ERROR_PACKAGE_EXCUTABLE_NOT_FOUND,

    WRT_INSTALLER_ERROR_MANIFEST_NOT_FOUND = 11,///<
    WRT_INSTALLER_ERROR_MANIFEST_INVALID,       ///<
    WRT_INSTALLER_CONFIG_NOT_FOUND,             ///< couldn't find config.xml
                                                ///< in package.
    WRT_INSTALLER_ERROR_CONFIG_INVALID,         ///< invalid config.xml

    WRT_INSTALLER_ERROR_SIGNATURE_NOT_FOUND = 21,    ///< signature file not exist.
    WRT_INSTALLER_ERROR_SIGNATURE_INVALID,           ///< invalid signature file
    WRT_INSTALLER_ERROR_SIGNATURE_VERIFICATION_FAILED,  ///< failure in verificate signature
    WRT_INSTALLER_ERROR_ROOT_CERTIFICATE_NOT_FOUND = 31, ///< couldn't find root certificate.
    WRT_INSTALLER_ERROR_CERTIFICATION_INVAID,       ///< invalid certification
    WRT_INSTALLER_ERROR_CERTIFICATE_CHAIN_VERIFICATION_FAILED,    ///< failure in verificate certification chain.
    WRT_INSTALLER_ERROR_CERTIFICATE_EXPIRED,        ///< expire cerification.

    WRT_INSTALLER_ERROR_INVALID_PRIVILEGE = 41,     ///< invalid privilege.
    WRT_INSTALLER_ERROR_PRIVILEGE_LEVEL_VIOLATION,

    WRT_INSTALLER_ERROR_MENU_ICON_NOT_FOUND = 51,   ///<

    WRT_INSTALLER_ERROR_FATAL_ERROR = 61,           ///< failure in db operation or file opertion..
    WRT_INSTALLER_ERROR_OUT_OF_STORAGE,             ///< failure in shortage of memory
    WRT_INSTALLER_ERROR_OUT_OF_MEMORY,              ///< failure in shortage of RAM
    WRT_INSTALLER_ERROR_ARGUMENT_INVALID,

    /* wrt-installer error */
    /* 121-140 : reserved for Web installer */

    /* installation */
    WRT_INSTALLER_ERROR_PACKAGE_ALREADY_INSTALLED = 121,
    WRT_INSTALLER_ERROR_ACE_CHECK_FAILED,
    WRT_INSTALLER_ERROR_MANIFEST_CREATE_FAILED,     ///<
    WRT_INSTALLER_ERROR_ENCRYPTION_FAILED,          ///< Failure in reousrce encrypttion
    WRT_INSTALLER_ERROR_INSTALL_OSP_SERVCIE,        ///< Failure in installing osp service
    WRT_INSTALLER_ERROR_PLUGIN_INSTALLATION_FAILED,
    WRT_INSTALLER_ERROR_UNINSTALLATION_FAILED,

    WRT_INSTALLER_ERROR_UNKNOWN = 140,              ///< do not use this error code.

} WrtErrStatus;

#endif /* WRT_TYPE_H_ */
