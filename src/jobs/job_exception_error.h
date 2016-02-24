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
 * @file        job_exception_error.h
 * @author      Soyoung Kim (sy037.kim@samsung.com)
 * @version     1.0
 * @brief       This file contains declarations of wrt api
 */

/*
 * @defgroup wrt_engine_group WebRunTime engine Library
 * @ingroup internet_FW
 * Functions to APIs to access wrt-engine
 */

#ifndef JOB_EXCEPTION_ERROR_H
#define JOB_EXCEPTION_ERROR_H

#include <stdbool.h>
#include <stddef.h>

namespace Jobs {
namespace Exceptions {
enum Type
{
    Success = 0,                         ///< Success

    /* pkgmgr error */
    ErrorPackageNotFound,                       ///<
    ErrorPackageInvalid,                        ///< invalid widget package
    ErrorPackageLowerVersion,                   ///< given version is lower
    ErrorPackageExecutableNotFound,

    ErrorManifestNotFound = 11,                 ///<
    ErrorManifestInvalid,                       ///<
    ErrorConfigNotFound,                        ///< couldn't find config.xml
    ErrorConfigInvalid,                         ///< invalid config.xml

    ErrorSignatureNotFound = 21,                ///< signature file not exist.
    ErrorSignatureInvalid,                      ///< invalid signature file
    ErrorSignatureVerificationFailed,           ///< failure in verificate
                                                ///< signature
    ErrorRootCertificateNotFound = 31,          ///< couldn't find root
    ErrorCertificationInvaid,                   ///< invalid certification
    ErrorCertificateChainVerificationFailed,    ///< failure in verificate
    ErrorCertificateExpired,                    ///< expire cerification.

    ErrorInvalidPrivilege = 41,                 ///< invalid privilege.
    ErrorPrivilegeLevelViolation,

    ErrorMenuIconNotFound = 51,                 ///<

    ErrorFatalError = 61,                       ///< failure in db operation
    ErrorOutOfStorage,                          ///< failure in shortage of memory
    ErrorOutOfMemory,                           ///< failure in shortage of RAM
    ErrorArgumentInvalid,

    /* wrt-installer error */
    /* 121-140 : reserved for Web installer */
    ErrorPackageAlreadyInstalled = 121,     ///< package already in target.
    ErrorAceCheckFailed,                    ///< failure in ace check.
    ErrorManifestCreateFailed,              ///< failure in creating manifest
    ErrorEncryptionFailed,                  ///< failure in encryption resource
    ErrorInstallOspServcie,                 ///< Failure in installing osp service
    ErrorPluginInstallationFailed,          ///< failure in plugin installation
    ErrorWidgetUninstallationFailed,        ///< failure in uninstallation
    ErrorNotSupportRDSUpdate,               ///< failure in rds update

    ErrorUnknown = 140,                     ///< do not use this error code.
};
}
}

#endif /* JOB_EXCEPTION_ERROR_H */
