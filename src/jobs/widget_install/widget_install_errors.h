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
 * @file    installer_errors.h
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @version
 * @brief
 */

#ifndef INSTALLER_ERRORS_H_
#define INSTALLER_ERRORS_H_

#include <dpl/exception.h>
#include <job_exception_base.h>
#include <job_exception_error.h>

//TODO SafeException(...)

using namespace Jobs::Exceptions;

namespace Jobs {
namespace WidgetInstall {
namespace Exceptions {

DECLARE_JOB_EXCEPTION_BASE(JobExceptionBase, Base, ErrorUnknown)

DECLARE_JOB_EXCEPTION(Base, OpenZipFailed, ErrorPackageInvalid)
DECLARE_JOB_EXCEPTION(Base, ZipEmpty, ErrorPackageInvalid)
DECLARE_JOB_EXCEPTION(Base, ExtractFileFailed, ErrorPackageInvalid)
DECLARE_JOB_EXCEPTION(Base, EmptyPluginsDirectory, ErrorPackageInvalid)
DECLARE_JOB_EXCEPTION(Base, PluginsSubdirectory, ErrorPackageInvalid)
DECLARE_JOB_EXCEPTION(Base, RDSDeltaFailure, ErrorPackageInvalid)
DECLARE_JOB_EXCEPTION(Base, MissingConfig, ErrorPackageInvalid)
DECLARE_JOB_EXCEPTION(Base, InvalidStartFile, ErrorPackageInvalid)

DECLARE_JOB_EXCEPTION(Base, PackageLowerVersion, ErrorPackageLowerVersion)

DECLARE_JOB_EXCEPTION(Base, ManifestInvalid, ErrorManifestInvalid)

DECLARE_JOB_EXCEPTION(Base, WidgetConfigFileNotFound, ErrorConfigNotFound)
DECLARE_JOB_EXCEPTION(Base, WidgetConfigFileInvalid, ErrorConfigInvalid)

DECLARE_JOB_EXCEPTION(Base, SignatureNotFound, ErrorSignatureNotFound)

DECLARE_JOB_EXCEPTION(Base, SignatureInvalid, ErrorSignatureInvalid)

DECLARE_JOB_EXCEPTION(Base, SignatureVerificationFailed, ErrorSignatureVerificationFailed)

DECLARE_JOB_EXCEPTION(Base, RootCertificateNotFound, ErrorRootCertificateNotFound)

DECLARE_JOB_EXCEPTION(Base, CertificationInvaid, ErrorCertificationInvaid)
DECLARE_JOB_EXCEPTION(Base, NotMatchedCertification, ErrorCertificationInvaid)

DECLARE_JOB_EXCEPTION(Base, CertificateChainVerificationFailed, ErrorCertificateChainVerificationFailed)

DECLARE_JOB_EXCEPTION(Base, CertificateExpired, ErrorCertificateExpired)

DECLARE_JOB_EXCEPTION(Base, NotAllowed, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, WidgetRunningError, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, DrmDecryptFailed, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, DatabaseFailure, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, RemovingFolderFailure, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, RemovingFileFailure, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, CreateVconfFailure, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, CopyIconFailed, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, FileOperationFailed, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, InstallToExternalFailed, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, BackupFailed, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, InsertNewWidgetFailed, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, RemoveBackupFailed, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, UpdateFailed, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, SetCertificateInfoFailed, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, ErrorExternalInstallingFailure, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, GetInfoPkgMgrFailed, ErrorFatalError)

DECLARE_JOB_EXCEPTION(Base, PackageAlreadyInstalled, ErrorPackageAlreadyInstalled)
DECLARE_JOB_EXCEPTION(Base, AceCheckFailed, ErrorAceCheckFailed)
DECLARE_JOB_EXCEPTION(Base, EncryptionFailed, ErrorEncryptionFailed)
DECLARE_JOB_EXCEPTION(Base, InstallOspsvcFailed, ErrorInstallOspServcie)
DECLARE_JOB_EXCEPTION(Base, PrivilegeLevelViolation, ErrorPrivilegeLevelViolation)
DECLARE_JOB_EXCEPTION(Base, NotSupportRDSUpdate, ErrorNotSupportRDSUpdate)
DECLARE_JOB_EXCEPTION(Base, SmackTransactionFailed, ErrorFatalError)
DECLARE_JOB_EXCEPTION(Base, OutOfStorageFailed, ErrorOutOfStorage)
DECLARE_JOB_EXCEPTION(Base, RecoveryFailed, ErrorFatalError)

DECLARE_JOB_EXCEPTION(Base, PrivilegeUsingLegacyFailed, ErrorInstallPrivilegeUsingLegacyFailed)
DECLARE_JOB_EXCEPTION(Base, PrivilegeUnknownkFailed, ErrorInstallPrivilegeUnknownFailed)
DECLARE_JOB_EXCEPTION(Base, PrivilegeUnauthorizedFailed, ErrorInstallPrivilegeUnauthorizedFailed)
} //namespace
} //namespace
} //namespace

#endif /* INSTALLER_ERRORS_H_ */
