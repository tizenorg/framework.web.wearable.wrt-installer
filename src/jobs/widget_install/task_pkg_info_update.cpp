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
 * @file    task_pkg_info_update.cpp
 * @author  Soyoung Kim (sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task information about package
 * update
 */
#include "task_pkg_info_update.h"

#include <unistd.h>
#include <string>

#include <fstream>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/foreach.h>
#include <dpl/sstream.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <pkgmgr_installer.h>
#include <pkgmgr/pkgmgr_parser.h>
#include <pkgmgr-info.h>
#include <vcore/CryptoHash.h>

#include <widget_install/job_widget_install.h>
#include <widget_install/widget_install_context.h>
#include <widget_install/widget_install_errors.h>

#include <installer_log.h>

using namespace WrtDB;

namespace {
}

namespace Jobs {
namespace WidgetInstall {
TaskPkgInfoUpdate::TaskPkgInfoUpdate(InstallerContext& context) :
    DPL::TaskDecl<TaskPkgInfoUpdate>(this),
    m_context(context)
{
    AddStep(&TaskPkgInfoUpdate::StartStep);
    AddStep(&TaskPkgInfoUpdate::StepPkgInfo);
    AddStep(&TaskPkgInfoUpdate::StepSetCertiInfo);
    AddStep(&TaskPkgInfoUpdate::EndStep);
    AddStep(&TaskPkgInfoUpdate::StepSetEndofInstallation);

    AddAbortStep(&TaskPkgInfoUpdate::StepAbortCertiInfo);
    AddAbortStep(&TaskPkgInfoUpdate::stepAbortParseManifest);
}

void TaskPkgInfoUpdate::StepPkgInfo()
{
    int code = 0;
    char* updateTags[3] = {NULL, };

    char preloadTrue[] = "preload=true";
    char removableTrue[] = "removable=true";
    char removableFalse[] = "removable=false";

    if (InstallMode::InstallTime::CSC == m_context.mode.installTime
        || InstallMode::InstallTime::PRELOAD == m_context.mode.installTime) {
        updateTags[0] = preloadTrue;
        if (m_context.mode.removable) {
            updateTags[1] = removableTrue;
        } else {
            updateTags[1] = removableFalse;
        }
        updateTags[2] = NULL;
    }

    if (m_context.mode.rootPath == InstallMode::RootPath::RO) {
        m_manifest += "/usr/share/packages/";
    } else {
        m_manifest += "/opt/share/packages/";
    }
    m_manifest += DPL::ToUTF8String(m_context.widgetConfig.tzPkgid) + ".xml";
    _D("manifest file : %s", m_manifest.c_str());

    if (m_context.isUpdateMode || (
                m_context.mode.rootPath == InstallMode::RootPath::RO
                && (m_context.mode.installTime == InstallMode::InstallTime::PRELOAD
                    || m_context.mode.installTime == InstallMode::InstallTime::FOTA)
                && m_context.mode.extension == InstallMode::ExtensionType::DIR)) {

        code = pkgmgr_parser_parse_manifest_for_upgrade(
                m_manifest.c_str(), (updateTags[0] == NULL) ? NULL : updateTags);

        if (code != 0) {
            _E("Manifest parser error: %d", code);
            ThrowMsg(Exceptions::ManifestInvalid, "Parser returncode: " << code);
        }
    } else {
        code = pkgmgr_parser_parse_manifest_for_installation(
                m_manifest.c_str(), (updateTags[0] == NULL) ? NULL : updateTags);

        if (code != 0) {
            _E("Manifest parser error: %d", code);
            ThrowMsg(Exceptions::ManifestInvalid, "Parser returncode: " << code);
        }
    }

    m_context.job->UpdateProgress(
            InstallerContext::INSTALL_PKGINFO_UPDATE,
            "Manifest Update Finished");
    _D("Manifest parsed");
}

void TaskPkgInfoUpdate::StepSetCertiInfo()
{
    _D("StepSetCertiInfo");

    if (pkgmgr_installer_create_certinfo_set_handle(&m_pkgHandle) < 0) {
        _E("pkgmgrInstallerCreateCertinfoSetHandle fail");
        ThrowMsg(Exceptions::SetCertificateInfoFailed,
                 "Failed to create certificate handle");
    }

    SetCertiInfo(SIGNATURE_AUTHOR);
    SetCertiInfo(SIGNATURE_DISTRIBUTOR);
    SetCertiInfo(SIGNATURE_DISTRIBUTOR2);

    if ((pkgmgr_installer_save_certinfo(
             const_cast<char*>(DPL::ToUTF8String(
                                   m_context.widgetConfig.tzPkgid).c_str()),
             m_pkgHandle)) < 0)
    {
        _E("pkgmgrInstallerSaveCertinfo fail");
        ThrowMsg(Exceptions::SetCertificateInfoFailed,
                 "Failed to Installer Save Certinfo");
    } else {
        _D("Succeed to save Certinfo");
    }

    if (pkgmgr_installer_destroy_certinfo_set_handle(m_pkgHandle) < 0) {
        _E("pkgmgrInstallerDestroyCertinfoSetHandle fail");
    }
}

void TaskPkgInfoUpdate::SetCertiInfo(int source)
{
    _D("Set CertiInfo to pkgmgr : %d", source);
    CertificateChainList certificateChainList;
    m_context.widgetSecurity.getCertificateChainList(certificateChainList,
            (CertificateSource)source);

    FOREACH(it, certificateChainList)
    {
        _D("Insert certinfo to pkgmgr structure");

        ValidationCore::CertificateCollection chain;

        if (false == chain.load(*it)) {
            _E("Chain is broken");
            ThrowMsg(Exceptions::SetCertificateInfoFailed,
                     "Failed to Installer Save Certinfo");
        }

        if (!chain.sort()) {
            _E("Chain failed at sorting");
        }

        ValidationCore::CertificateList list = chain.getCertificateList();

        FOREACH(certIt, list)
        {
            pkgmgr_instcert_type instCertType = PM_SET_AUTHOR_ROOT_CERT;
            if (source == SIGNATURE_AUTHOR) {
                _D("set SIGNATURE_AUTHOR");
                if ((*certIt)->isRootCert()) {
                    instCertType = PM_SET_AUTHOR_ROOT_CERT;
                } else {
                    if ((*certIt)->isCA()) {
                        instCertType = PM_SET_AUTHOR_INTERMEDIATE_CERT;
                    } else {
                        instCertType = PM_SET_AUTHOR_SIGNER_CERT;
                    }
                }
            } else if (source == SIGNATURE_DISTRIBUTOR) {
                _D("Set SIGNATURE_DISTRIBUTOR");
                if ((*certIt)->isRootCert()) {
                    instCertType = PM_SET_DISTRIBUTOR_ROOT_CERT;
                } else {
                    if ((*certIt)->isCA()) {
                        instCertType = PM_SET_DISTRIBUTOR_INTERMEDIATE_CERT;
                    } else {
                        instCertType = PM_SET_DISTRIBUTOR_SIGNER_CERT;
                    }
                }
            } else if (source == SIGNATURE_DISTRIBUTOR2) {
                _D("Set SIGNATURE_DISTRIBUTOR2");
                if ((*certIt)->isRootCert()) {
                    instCertType = PM_SET_DISTRIBUTOR2_ROOT_CERT;
                } else {
                    if ((*certIt)->isCA()) {
                        instCertType = PM_SET_DISTRIBUTOR2_INTERMEDIATE_CERT;
                    } else {
                        instCertType = PM_SET_DISTRIBUTOR2_SIGNER_CERT;
                    }
                }
            } else {
                _D("UNKNOWN..");
            }
            _D("cert type : %d", instCertType);
            if ((pkgmgr_installer_set_cert_value(
                     m_pkgHandle,
                     instCertType,
                     const_cast<char*>(((*certIt)->getBase64()).c_str()))) < 0)
            {
                _E("pkgmgrInstallerSetCertValue fail");
                ThrowMsg(Exceptions::SetCertificateInfoFailed,
                         "Failed to Set CertValue");
            }
        }
    }
}

void TaskPkgInfoUpdate::StepAbortCertiInfo()
{
    if ((pkgmgr_installer_delete_certinfo(
             const_cast<char*>(DPL::ToUTF8String(
                                   m_context.widgetConfig.tzPkgid).c_str()))) <
        0)
    {
        _E("pkgmgr_installer_delete_certinfo fail");
    }
}

void TaskPkgInfoUpdate::StartStep()
{
    LOGI("--------- <TaskPkgInfoUpdate> : START ----------");
}

void TaskPkgInfoUpdate::EndStep()
{
    m_context.job->UpdateProgress(
            InstallerContext::INSTALL_SET_CERTINFO,
            "Save certinfo to pkgmgr");

    LOGI("--------- <TaskPkgInfoUpdate> : END ----------");
}

void TaskPkgInfoUpdate::stepAbortParseManifest()
{
    _E("[Parse Manifest] Abroting....");

    int code = pkgmgr_parser_parse_manifest_for_uninstallation(
            m_manifest.c_str(), NULL);

    if (0 != code) {
        _W("Manifest parser error: %d", code);
        ThrowMsg(Exceptions::ManifestInvalid, "Parser returncode: " << code);
    }
    int ret = unlink(m_manifest.c_str());
    if (0 != ret) {
        _W("No manifest file found: %s", m_manifest.c_str());
    }
}

void TaskPkgInfoUpdate::StepSetEndofInstallation()
{
    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_END,
        "End installation");
}

} //namespace WidgetInstall
} //namespace Jobs
