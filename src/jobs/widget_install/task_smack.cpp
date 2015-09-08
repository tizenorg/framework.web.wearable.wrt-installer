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
/*
 * @file    task_smack.cpp
 * @author  Piotr Kozbial (p.kozbial@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task smack
 */

#include <widget_install/task_smack.h>
#include <widget_install/task_certify.h>
#include <widget_install/widget_install_context.h>
#include <widget_install/widget_install_errors.h>
#include <widget_install/job_widget_install.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>
#include <dpl/foreach.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/platform.h>
#include <dpl/utils/bash_utils.h>
#include <dpl/utils/path.h>
#include <vcore/Certificate.h>
#include <vcore/CryptoHash.h>
#include <vcore/SignatureFinder.h>
#include <privilege-control.h>
#include <sys/smack.h>
#include <sstream>
#include <installer_log.h>
#include <privilege-control.h>

using namespace WrtDB;
using namespace ValidationCore;

namespace {
const int MAX_BUF_SIZE = 128;
void freeList(const char** list) {
    for (int i = 0; list[i] != NULL; i++)
    {
        delete(list[i]);
    }
    delete[] list;
}
}

namespace Jobs {
namespace WidgetInstall {
TaskSmack::TaskSmack(InstallerContext& context) :
    DPL::TaskDecl<TaskSmack>(this),
    m_context(context)
{
    AddStep(&TaskSmack::StartStep);
    AddStep(&TaskSmack::StepSetInstall);
    AddStep(&TaskSmack::StepSmackAppPrivilegeVersion);
    AddStep(&TaskSmack::StepSmackFolderLabeling);
    AddStep(&TaskSmack::StepSmackPrivilege);
    AddStep(&TaskSmack::StepAddLabelNPRuntime);
    AddStep(&TaskSmack::StepLabelSignatureFiles);
    AddStep(&TaskSmack::EndStep);

    AddAbortStep(&TaskSmack::StepAbortSmack);
}

void TaskSmack::StepSetInstall()
{
    _D("----------------> SMACK: StepStartSetSmack()");

    m_pkgId = DPL::ToUTF8String(m_context.widgetConfig.tzPkgid);

    if (PC_OPERATION_SUCCESS != perm_app_install(m_pkgId.c_str())) {
        ThrowMsg(Exceptions::NotAllowed, "Instalation failure. "
                "failure in creating smack rules file.");
    }
}

//Note: perm_app_set_privilege_version calling order is important.
//perm_app_install->perm_app_set_privilege_version->perm_app_setup_path/perm_app_enable_permissions
void TaskSmack::StepSmackAppPrivilegeVersion()
{
    _D("----------------> SMACK:\
            Jobs::WidgetInstall::TaskSmack::StepSmackAppPrivilegeVersion()");

    DPL::OptionalString minVersion = m_context.widgetConfig.configInfo.tizenMinVersionRequired;
    std::string version = DPL::ToUTF8String(*minVersion);
    if (version.empty()) {
        _W("setting app privilege version - default");
        return;
    }

    if (PC_OPERATION_SUCCESS != perm_app_set_privilege_version(m_pkgId.c_str(),
        version.c_str())) {
        _W("failure in setting app privilege version for version %s", version.c_str());
        ThrowMsg(Exceptions::NotAllowed, "Failure in setting app privilege version.");
    } else {
        _D("Success in setting app privilege version for version %s", version.c_str());
    }
}

void TaskSmack::StepSmackFolderLabeling()
{
    _D("----------------> SMACK:\
            Jobs::WidgetInstall::TaskSmack::SmackFolderLabelingStep()");

    /* /opt/usr/apps/[pkgid] directory's label is "_" */
    if (PC_OPERATION_SUCCESS != perm_app_setup_path(m_pkgId.c_str(),
                m_context.locations->getPackageInstallationDir().c_str(),
                APP_PATH_ANY_LABEL, "_")) {
        _W("Add label to %s", m_context.locations->getPackageInstallationDir().c_str());
    }

    /* for prelaod */
    if (m_context.mode.installTime == InstallMode::InstallTime::PRELOAD) {
        if (PC_OPERATION_SUCCESS != perm_app_setup_path(m_pkgId.c_str(),
                    m_context.locations->getUserDataRootDir().c_str(),
                    APP_PATH_ANY_LABEL, "_")) {
        }
    }

    /* res directory */
    std::string resDir = m_context.locations->getPackageInstallationDir() +
        "/res";

    if (PC_OPERATION_SUCCESS != perm_app_setup_path(m_pkgId.c_str(), resDir.c_str(),
                APP_PATH_PRIVATE)) {
        _W("Add label to %s", resDir.c_str());
    }

    /* data directory */
    if (PC_OPERATION_SUCCESS != perm_app_setup_path(m_pkgId.c_str(),
                m_context.locations->getPrivateStorageDir().c_str(),
                APP_PATH_PRIVATE)) {
        _W("Add label to %s", m_context.locations->getPrivateStorageDir().c_str());
    }

    /* tmp directory */
    if (PC_OPERATION_SUCCESS != perm_app_setup_path(m_pkgId.c_str(),
                m_context.locations->getPrivateTempStorageDir().c_str(),
                APP_PATH_PRIVATE))
    {
        _W("Add label to %s", m_context.locations->getPrivateTempStorageDir().c_str());
    }

    /* bin directory */
    if( m_context.widgetConfig.packagingType != PKG_TYPE_HYBRID_WEB_APP){
        if (PC_OPERATION_SUCCESS != perm_app_setup_path(m_pkgId.c_str(),
                    m_context.locations->getBinaryDir().c_str(),
                    APP_PATH_PRIVATE)) {
            _W("Add label to %s", m_context.locations->getBinaryDir().c_str());
        }
    }else{
        // for hybrid webapp
        std::string native_label = m_pkgId + ".native";
        if (PC_OPERATION_SUCCESS != perm_app_setup_path(m_pkgId.c_str(),
                    m_context.locations->getBinaryDir().c_str(),
                    APP_PATH_ANY_LABEL, native_label.c_str())) {
            _W("Add label to %s", m_context.locations->getBinaryDir().c_str());
        }
        // webapp executable
        perm_app_setup_path(m_pkgId.c_str(), m_context.locations->getExecFile().c_str(), APP_PATH_PRIVATE);
#ifdef SERVICE_ENABLED
        // Service app
        FOREACH(it, m_context.widgetConfig.configInfo.serviceAppInfoList) {
            std::string serviceExec = m_context.locations->getBinaryDir() + "/" + DPL::ToUTF8String(it->serviceId);
            perm_app_setup_path(m_pkgId.c_str(),serviceExec.c_str(),APP_PATH_PRIVATE);
        }
#endif
#if USE(WEB_PROVIDER)
        ConfigParserData::LiveboxList& liveboxList = m_context.widgetConfig.configInfo.m_livebox;
        if (!liveboxList.empty()) {
            std::string dBoxExec = m_context.locations->getExecFile() + ".d-box";
            perm_app_setup_path(m_pkgId.c_str(),dBoxExec.c_str(),APP_PATH_PRIVATE);
        }
#endif
    }

    if(!setLabelForSharedDir(m_pkgId.c_str())) {
        _W("Add label to shared directory");
    }

    /* TODO : set label at wrt-client */
}

void TaskSmack::StepSmackPrivilege()
{
    _D("----------------> SMACK:\
        Jobs::WidgetInstall::TaskSmack::SmackPrivilegeStep()");

    app_type_t app_type = PERM_APP_TYPE_WRT;
    switch(m_context.certLevel) {
        case Jobs::WidgetInstall::TaskCertify::Level::PLATFORM:
            app_type = PERM_APP_TYPE_WRT_PLATFORM;
            break;
        case Jobs::WidgetInstall::TaskCertify::Level::PARTNER:
            app_type = PERM_APP_TYPE_WRT_PARTNER;
            break;
    }

    std::string id = DPL::ToUTF8String(m_context.widgetConfig.tzPkgid);
    char* appId = NULL;
    appId = (char*)calloc(1, id.length() + 1);
    if (appId == NULL) {
        ThrowMsg(Exceptions::NotAllowed, "Failure in calloc");
    }
    snprintf(appId, id.length() + 1, "%s", id.c_str());

    WrtDB::ConfigParserData::PrivilegeList privileges =
        m_context.widgetConfig.configInfo.privilegeList;

    char** perm_list = new char*[privileges.size() + 1];
    int index = 0;
    FOREACH(it, privileges) {
        _D("Permission : %ls", it->name.c_str());
        int length = DPL::ToUTF8String(it->name).length();
        char *priv = new char[length + 1];
        snprintf(priv, length + 1, "%s",
                 DPL::ToUTF8String(it->name).c_str());
        perm_list[index++] = priv;
    }
    perm_list[index] = NULL;

    if (PC_OPERATION_SUCCESS != perm_app_enable_permissions(appId, app_type,
                const_cast<const char **>(perm_list), true)) {
        _W("failure in contructing smack rules based on perm_list");
    }

    free(appId);
    index = 0;
    while (NULL != perm_list[index]) {
        delete [] perm_list[index++];
    }
    delete [] perm_list;

    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_SMACK_ENABLE,
        "Widget SMACK Enabled");
}

void TaskSmack::StepAddLabelNPRuntime()
{
    _D("----------------> SMACK:\
            Jobs::WidgetInstall::TaskSmack::StepAddLabelNPRuntime()");

    if (0 == access(m_context.locations->getNPPluginsDir().c_str(), F_OK)) {
        if (PC_OPERATION_SUCCESS !=
                perm_app_setup_path(DPL::ToUTF8String(m_context.widgetConfig.tzPkgid).c_str(),
                    m_context.locations->getNPPluginsExecFile().c_str(),
                    PERM_APP_PATH_NPRUNTIME)) {
            _E("failed to set smack execute label to %s",
                    m_context.locations->getNPPluginsExecFile().c_str());
        }
    }
}


void TaskSmack::StepLabelSignatureFiles()
{
    _D("----------------> SMACK:\
            Jobs::WidgetInstall::TaskSmack::StepLabelSignatureFiles()");

    DPL::Utils::Path widgetPath{
            m_context.locations->getPackageInstallationDir()};
    widgetPath /= WrtDB::GlobalConfig::GetWidgetSrcPath();

    SignatureFileInfoSet signatureFiles;
    SignatureFinder signatureFinder(widgetPath.Fullpath());
    if (SignatureFinder::NO_ERROR != signatureFinder.find(signatureFiles)) {
        ThrowMsg(Exceptions::SignatureNotFound,
                 "Error while discovering signature files.");
    }

    for (auto it = signatureFiles.cbegin(); it != signatureFiles.cend(); ++it) {
        auto sigPath = widgetPath / it->getFileName();

        _D("Setting label to %s", sigPath.Fullpath().c_str());
        if (PC_OPERATION_SUCCESS != perm_app_setup_path(m_pkgId.c_str(),
                    sigPath.Fullpath().c_str(),
                    APP_PATH_ANY_LABEL, "_")) {
            _W("Failed to set label to %s", sigPath.Fullpath().c_str());
        }
    }
}

void TaskSmack::StepRevokeForUpdate()
{
    _D("----------------> SMACK:\
        Jobs::WidgetInstall::TaskSmack::StepRevokePrivilegeForUpdate()");

    if (PC_OPERATION_SUCCESS != perm_app_revoke_permissions(m_pkgId.c_str())) {
        _W("failure in revoking smack permissions");
    }
}

void TaskSmack::StepAbortSmack()
{
    _D("----------------> SMACK:\
            Jobs::WidgetInstall::TaskSmack::StepAbortSmack()");

    if (PC_OPERATION_SUCCESS != perm_app_revoke_permissions(m_pkgId.c_str())) {
        _W("failure in revoking smack permissions");
    }

    if (PC_OPERATION_SUCCESS != perm_app_uninstall(m_pkgId.c_str())) {
        _W("failure in removing smack rules file");
    }
}

bool TaskSmack::setLabelForSharedDir(const char* pkgId)
{
    /* /shared directory */
    if (PC_OPERATION_SUCCESS != perm_app_setup_path(pkgId,
                m_context.locations->getSharedRootDir().c_str(),
                APP_PATH_ANY_LABEL, "_")) {
        _W("Add label to %s", m_context.locations->getUserDataRootDir().c_str());
    }

    /* /shared/res directory */
    if (PC_OPERATION_SUCCESS != perm_app_setup_path(pkgId,
                m_context.locations->getSharedResourceDir().c_str(),
                APP_PATH_ANY_LABEL, "_")) {
        _W("Add label to %s", m_context.locations->getSharedResourceDir().c_str());
    }

    /* /shared/trusted directory */
    CertificatePtr rootCert = m_context.widgetSecurity.getAuthorCertificatePtr();
    if (!!rootCert) {
        ValidationCore::Crypto::Hash::SHA1 sha1;

        /*  ValidationCore::Crypto::Hash throws non-dpl namespace exception
         *  to remove dpl dependency of cert-svc
         *
         *  [changed] Validation::Crypto::Hash can throws
         *  - ValidationCore::Crypto::Hash::OutOfSequence
         *  - ValidationCore::Crypto::Hash::AppendFailed
         *
         *  [from]    Validation::Crypto::Hash could throws
         *  - DPL::Exception::OutOfSequence
         *  - DPL::Exception::AppendFailed
         */
        sha1.Append(rootCert->getDER());
        sha1.Finish();
        std::string sha1String = sha1.ToBase64String();
        size_t iPos = sha1String.find("/");
        while(iPos < std::string::npos) {
            sha1String.replace(iPos, 1, "#");
            iPos = sha1String.find("/");
        }

        _D("sha1 label string : %s", sha1String.c_str());

        if (PC_OPERATION_SUCCESS != perm_app_setup_path(pkgId,
                    m_context.locations->getSharedTrustedDir().c_str(),
                    APP_PATH_GROUP_RW, sha1String.c_str())) {
            _W("Add label to %s", m_context.locations->getBinaryDir().c_str());
        }
    }

    /* /shared/data directory */
    if (PC_OPERATION_SUCCESS != perm_app_setup_path(pkgId,
                m_context.locations->getSharedDataDir().c_str(),
                APP_PATH_PUBLIC_RO)) {
        _W("Add label to %s", m_context.locations->getSharedDataDir().c_str());
    }

    return true;
}

void TaskSmack::StartStep()
{
    LOGI("--------- <TaskSmack> : START ----------");
    if (PC_OPERATION_SUCCESS != perm_begin()) {
        LOGE("Failed to smack transaction begin.");
        ThrowMsg(Exceptions::SmackTransactionFailed, "Failed to smack transaction begin");
    }
}

void TaskSmack::EndStep()
{
    LOGI("--------- <TaskSmack> : END ----------");
    if (PC_OPERATION_SUCCESS != perm_end()) {
        LOGE("Failed to smack transaction end.");
        ThrowMsg(Exceptions::SmackTransactionFailed, "Failed to smack transaction end");
    }
}
} //namespace WidgetInstall
} //namespace Jobs
