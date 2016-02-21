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
 * @file    api_callbacks_translate.h
 * @author  Pawel Sikorski (p.sikorski@samsung.com)
 * @version 1.0
 * @brief   Source file for api callbacks translate functions
 */

#include <installer_callbacks_translate.h>
#include <dpl/assert.h>
#include <installer_log.h>

namespace InstallerCallbacksTranslate {

// callback for finished install
void installFinishedCallback(void *userParam,
                             std::string tizenId,
                             Jobs::Exceptions::Type status)
{
    Assert(userParam != NULL);

    StatusCallbackStruct *apiStr =
        static_cast<StatusCallbackStruct*>(userParam);

    if (apiStr->status_callback) {
        // Translate error
        WrtErrStatus errorStatus;

        switch (status) {
        case Jobs::Exceptions::Success:
            errorStatus = WRT_SUCCESS;
            break;

        case Jobs::Exceptions::ErrorPackageNotFound:
            errorStatus = WRT_INSTALLER_ERROR_PACKAGE_NOT_FOUND;
            break;

        case Jobs::Exceptions::ErrorPackageInvalid:
            errorStatus = WRT_INSTALLER_ERROR_PACKAGE_INVALID;
            break;

        case Jobs::Exceptions::ErrorPackageLowerVersion:
            errorStatus = WRT_INSTALLER_ERROR_PACKAGE_LOWER_VERSION;
            break;

        case Jobs::Exceptions::ErrorPackageExecutableNotFound:
            errorStatus = WRT_INSTALLER_ERROR_PACKAGE_EXCUTABLE_NOT_FOUND;
            break;

        case Jobs::Exceptions::ErrorManifestNotFound:
            errorStatus = WRT_INSTALLER_ERROR_MANIFEST_NOT_FOUND;
            break;

        case Jobs::Exceptions::ErrorManifestInvalid:
            errorStatus = WRT_INSTALLER_ERROR_MANIFEST_INVALID;
            break;

        case Jobs::Exceptions::ErrorConfigNotFound:
            errorStatus = WRT_INSTALLER_CONFIG_NOT_FOUND;
            break;

        case Jobs::Exceptions::ErrorConfigInvalid:
            errorStatus = WRT_INSTALLER_ERROR_CONFIG_INVALID;
            break;

        case Jobs::Exceptions::ErrorSignatureNotFound:
            errorStatus = WRT_INSTALLER_ERROR_SIGNATURE_NOT_FOUND;
            break;

        case Jobs::Exceptions::ErrorSignatureInvalid:
            errorStatus = WRT_INSTALLER_ERROR_SIGNATURE_INVALID;
            break;

        case Jobs::Exceptions::ErrorSignatureVerificationFailed:
            errorStatus = WRT_INSTALLER_ERROR_SIGNATURE_VERIFICATION_FAILED;
            break;

        case Jobs::Exceptions::ErrorRootCertificateNotFound:
            errorStatus = WRT_INSTALLER_ERROR_ROOT_CERTIFICATE_NOT_FOUND;
            break;

        case Jobs::Exceptions::ErrorCertificationInvaid:
            errorStatus = WRT_INSTALLER_ERROR_CERTIFICATION_INVAID;
            break;

        case
            Jobs::Exceptions::ErrorCertificateChainVerificationFailed:
            errorStatus =
            WRT_INSTALLER_ERROR_CERTIFICATE_CHAIN_VERIFICATION_FAILED;
            break;

        case Jobs::Exceptions::ErrorCertificateExpired:
            errorStatus = WRT_INSTALLER_ERROR_CERTIFICATE_EXPIRED;
            break;

        case Jobs::Exceptions::ErrorInvalidPrivilege:
            errorStatus = WRT_INSTALLER_ERROR_INVALID_PRIVILEGE;
            break;

        case Jobs::Exceptions::ErrorPrivilegeLevelViolation:
            errorStatus = WRT_INSTALLER_ERROR_PRIVILEGE_LEVEL_VIOLATION;
            break;

        case Jobs::Exceptions::ErrorMenuIconNotFound:
            errorStatus = WRT_INSTALLER_ERROR_MENU_ICON_NOT_FOUND;
            break;

        case Jobs::Exceptions::ErrorFatalError:
            errorStatus = WRT_INSTALLER_ERROR_FATAL_ERROR;
            break;

        case Jobs::Exceptions::ErrorOutOfStorage:
            errorStatus = WRT_INSTALLER_ERROR_OUT_OF_STORAGE;
            break;

        case Jobs::Exceptions::ErrorOutOfMemory:
            errorStatus = WRT_INSTALLER_ERROR_OUT_OF_MEMORY;
            break;

        case Jobs::Exceptions::ErrorArgumentInvalid:
            errorStatus = WRT_INSTALLER_ERROR_ARGUMENT_INVALID;
            break;

        case Jobs::Exceptions::ErrorPackageAlreadyInstalled:
            errorStatus = WRT_INSTALLER_ERROR_PACKAGE_ALREADY_INSTALLED;
            break;

        case Jobs::Exceptions::ErrorAceCheckFailed:
            errorStatus = WRT_INSTALLER_ERROR_ACE_CHECK_FAILED;
            break;

        case Jobs::Exceptions::ErrorManifestCreateFailed:
            errorStatus = WRT_INSTALLER_ERROR_MANIFEST_CREATE_FAILED;
            break;

        case Jobs::Exceptions::ErrorEncryptionFailed:
            errorStatus = WRT_INSTALLER_ERROR_ENCRYPTION_FAILED;
            break;

        case Jobs::Exceptions::ErrorInstallOspServcie:
            errorStatus = WRT_INSTALLER_ERROR_INSTALL_OSP_SERVCIE;
            break;

        case Jobs::Exceptions::ErrorInstallPrivilegeUsingLegacyFailed:
            errorStatus = WGT_INSTALLER_ERR_PRIVILEGE_USING_LEGACY_FAILED;
            break;

        case Jobs::Exceptions::ErrorInstallPrivilegeUnknownFailed:
            errorStatus = WGT_INSTALLER_ERR_PRIVILEGE_UNKNOWN_FAILED;
            break;

        case Jobs::Exceptions::ErrorInstallPrivilegeUnauthorizedFailed:
            errorStatus = WGT_INSTALLER_ERR_PRIVILEGE_UNAUTHORIZED_FAILED;
            break;

        default:
            errorStatus = WRT_INSTALLER_ERROR_UNKNOWN;
            break;
        }

        // Callback
        apiStr->status_callback(tizenId, errorStatus, apiStr->userdata);
    } else {
        _D("installFinishedCallback: No callback");
    }
}

// callback for finished install
void uninstallFinishedCallback(void *userParam,
                               std::string tizenId,
                               Jobs::Exceptions::Type status)
{
    Assert(userParam != NULL);

    StatusCallbackStruct *apiStr =
        static_cast<StatusCallbackStruct*>(userParam);

    if (apiStr->status_callback) {
        // Translate error
        WrtErrStatus errorStatus;

        switch (status) {
        case Jobs::Exceptions::Success:
            errorStatus = WRT_SUCCESS;
            break;

        case Jobs::Exceptions::ErrorWidgetUninstallationFailed:
            errorStatus = WRT_INSTALLER_ERROR_UNINSTALLATION_FAILED;
            break;

        case Jobs::Exceptions::ErrorUnknown:
            errorStatus = WRT_INSTALLER_ERROR_UNKNOWN;
            break;

        default:
            errorStatus = WRT_INSTALLER_ERROR_UNKNOWN;
            break;
        }

        // Callback
        apiStr->status_callback(tizenId, errorStatus, apiStr->userdata);
    } else {
        _D("uninstallFinishedCallback: No callback");
    }
}

void pluginInstallFinishedCallback(void *userParam,
                                   Jobs::Exceptions::Type status)
{
    Assert(userParam);

    PluginStatusCallbackStruct *apiStr =
        static_cast<PluginStatusCallbackStruct*>(userParam);

    if (apiStr->statusCallback) {
        // Translate error
        WrtErrStatus errorStatus;

        switch (status) {
        case Jobs::Exceptions::Success:
            errorStatus = WRT_SUCCESS;
            break;
        case Jobs::Exceptions::ErrorPluginInstallationFailed:
            errorStatus = WRT_INSTALLER_ERROR_PLUGIN_INSTALLATION_FAILED;
            break;
        default:
            errorStatus = WRT_INSTALLER_ERROR_UNKNOWN;
            break;
        }

        apiStr->statusCallback(errorStatus, apiStr->userdata);
    } else {
        _D("PluginInstallFinishedCallback: No callback");
    }

    delete apiStr;
}

// callback for progress of install OR uninstall
void installProgressCallback(void *userParam,
                             ProgressPercent percent,
                             const ProgressDescription &description)
{
    Assert(userParam != NULL);

    StatusCallbackStruct *apiStr =
        static_cast<StatusCallbackStruct*>(userParam);

    if (apiStr->progress_callback) {
        //CALLBACK EXEC
        _D("Entered %2.0f%% %s", percent, description.c_str());
        apiStr->progress_callback(static_cast<float>(percent),
                                  description.c_str(),
                                  apiStr->userdata);
    } else {
        _D("installProgressCallback: ignoring NULL callback pointer");
    }
}
} //namespace

