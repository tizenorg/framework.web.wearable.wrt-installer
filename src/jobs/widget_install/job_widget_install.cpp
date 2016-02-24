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
 * @file    job_widget_install.cpp
 * @author  Radoslaw Wicik r.wicik@samsung.com
 * @author  Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
 * @version 1.0
 * @brief   Implementation file for main installer task
 */
#include <string>
#include <sys/time.h>
#include <ctime>
#include <cstdlib>
#include <limits.h>
#include <regex.h>

#include <dpl/utils/wrt_utility.h>
#include <dpl/utils/path.h>
#include <dpl/localization/w3c_file_localization.h>

#include <pkg-manager/pkgmgr_signal.h>
#include <app_manager.h>

#include "root_parser.h"
#include "widget_parser.h"
#include "parser_runner.h"
#include <widget_install/job_widget_install.h>
#include <widget_install/task_certify.h>
#include <widget_install/task_process_config.h>
#include <widget_install/task_file_manipulation.h>
#include <widget_install/task_ace_check.h>
#include <widget_install/task_smack.h>
#include <widget_install/task_manifest_file.h>
#include <widget_install/task_prepare_files.h>
#include <widget_install/task_recovery.h>
#include <widget_install/task_install_ospsvc.h>
#include <widget_install/task_update_files.h>
#include <widget_install/task_database.h>
#include <widget_install/task_remove_backup.h>
#include <widget_install/task_encrypt_resource.h>
#include <widget_install/task_pkg_info_update.h>
#include <widget_install/task_commons.h>
#include <widget_install/task_prepare_reinstall.h>
#include <widget_install/task_configuration.h>
#include <widget_install/task_user_data_manipulation.h>
#include <widget_install/task_status_check.h>
#include <widget_install_to_external.h>
#include <widget_install/widget_unzip.h>

#include <installer_log.h>

using namespace WrtDB;
using namespace Jobs::Exceptions;

namespace Jobs {
namespace WidgetInstall {

JobWidgetInstall::JobWidgetInstall(
    std::string const &widgetPath,
    std::string const &tzPkgId,
    const Jobs::WidgetInstall::WidgetInstallationStruct &
    installerStruct) :
    Job(UnknownInstallation),
    JobContextBase<Jobs::WidgetInstall::WidgetInstallationStruct>(installerStruct),
    m_exceptionCaught(Jobs::Exceptions::Success)
{
    m_installerContext.mode = m_jobStruct.m_installMode;
    m_installerContext.requestedPath = widgetPath;
    m_jobStruct.pkgmgrInterface->setPkgname(tzPkgId);

    if (InstallMode::Command::RECOVERY == m_installerContext.mode.command) {
        m_installerContext.widgetConfig.tzPkgid = DPL::FromUTF8String(tzPkgId);
        AddTask(new TaskRecovery(m_installerContext));
    }

    //start configuration of installation
    AddTask(new TaskConfiguration(m_installerContext));

    // Init installer context
    m_installerContext.installStep = InstallerContext::INSTALL_START;
    m_installerContext.job = this;

    m_installerContext.callerPkgId
            = DPL::FromUTF8String(m_jobStruct.pkgmgrInterface->getCallerId());
    _D("Caller Package Id : %s", DPL::ToUTF8String(m_installerContext.callerPkgId).c_str());
}

void JobWidgetInstall::appendNewInstallationTaskList()
{
    _D("Configure installation succeeded");
    m_installerContext.job->SetProgressFlag(true);

    AddTask(new TaskFileManipulation(m_installerContext));
    AddTask(new TaskProcessConfig(m_installerContext));
    if (m_installerContext.widgetConfig.packagingType ==
        WrtDB::PKG_TYPE_HOSTED_WEB_APP)
    {
        AddTask(new TaskPrepareFiles(m_installerContext));
    }
    AddTask(new TaskCertify(m_installerContext));
    AddTask(new TaskUserDataManipulation(m_installerContext));
    if (m_installerContext.needEncryption) {
        AddTask(new TaskEncryptResource(m_installerContext));
    }
    AddTask(new TaskManifestFile(m_installerContext));
    if (m_installerContext.widgetConfig.packagingType ==
        PKG_TYPE_HYBRID_WEB_APP) {
        AddTask(new TaskInstallOspsvc(m_installerContext));
    }
    AddTask(new TaskDatabase(m_installerContext));
    AddTask(new TaskAceCheck(m_installerContext));
    AddTask(new TaskSmack(m_installerContext));
    AddTask(new TaskPkgInfoUpdate(m_installerContext));
}

void JobWidgetInstall::appendUpdateInstallationTaskList()
{
    _D("Configure installation updated");
    _D("Widget Update");
    m_installerContext.job->SetProgressFlag(true);
    AddTask(new TaskStatusCheck(m_installerContext));

    if (m_installerContext.mode.command ==
        InstallMode::Command::REINSTALL) {
        AddTask(new TaskPrepareReinstall(m_installerContext));
    }

    if (m_installerContext.mode.extension !=
            InstallMode::ExtensionType::DIR) {
        AddTask(new TaskUpdateFiles(m_installerContext));
        AddTask(new TaskFileManipulation(m_installerContext));
    }

    AddTask(new TaskProcessConfig(m_installerContext));

    if (m_installerContext.widgetConfig.packagingType ==
        WrtDB::PKG_TYPE_HOSTED_WEB_APP) {
        AddTask(new TaskPrepareFiles(m_installerContext));
    }

    AddTask(new TaskCertify(m_installerContext));
    AddTask(new TaskUserDataManipulation(m_installerContext));
    if (m_installerContext.needEncryption) {
        AddTask(new TaskEncryptResource(m_installerContext));
    }
    AddTask(new TaskManifestFile(m_installerContext));
    if (m_installerContext.widgetConfig.packagingType ==
        PKG_TYPE_HYBRID_WEB_APP) {
        AddTask(new TaskInstallOspsvc(m_installerContext));
    }

    AddTask(new TaskDatabase(m_installerContext));
    AddTask(new TaskAceCheck(m_installerContext));
    //TODO: remove widgetHandle from this task and move before database task
    // by now widget handle is needed in ace check
    // Any error in acecheck while update will break widget
    AddTask(new TaskSmack(m_installerContext));
    AddTask(new TaskRemoveBackupFiles(m_installerContext));
    AddTask(new TaskPkgInfoUpdate(m_installerContext));
}

void JobWidgetInstall::appendRDSUpdateTaskList()
{
    _D("Configure installation RDS updated");
    m_installerContext.job->SetProgressFlag(true);
    AddTask(new TaskStatusCheck(m_installerContext));
    AddTask(new TaskPrepareReinstall(m_installerContext));
    AddTask(new TaskCertify(m_installerContext));
}

void JobWidgetInstall::appendRecoveryTaskList()
{
    _D("Recovery Task");
    m_installerContext.job->SetProgressFlag(true);

    AddTask(new TaskProcessConfig(m_installerContext));

    AddTask(new TaskCertify(m_installerContext));
    AddTask(new TaskManifestFile(m_installerContext));
    if (m_installerContext.widgetConfig.packagingType ==
        PKG_TYPE_HYBRID_WEB_APP) {
        AddTask(new TaskInstallOspsvc(m_installerContext));
    }
    AddTask(new TaskSmack(m_installerContext));
}

void JobWidgetInstall::appendFotaInstallationTaskList()
{
    /* TODO */
    _D("Configure installation succeeded");
    m_installerContext.job->SetProgressFlag(true);

    AddTask(new TaskProcessConfig(m_installerContext));
    AddTask(new TaskCertify(m_installerContext));
    AddTask(new TaskUserDataManipulation(m_installerContext));
    if (m_installerContext.needEncryption) {
        AddTask(new TaskEncryptResource(m_installerContext));
    }
    AddTask(new TaskManifestFile(m_installerContext));
    if (m_installerContext.widgetConfig.packagingType ==
        PKG_TYPE_HYBRID_WEB_APP)
    {
        AddTask(new TaskInstallOspsvc(m_installerContext));
    }
    AddTask(new TaskDatabase(m_installerContext));
    AddTask(new TaskAceCheck(m_installerContext));
    AddTask(new TaskSmack(m_installerContext));
    AddTask(new TaskRemoveBackupFiles(m_installerContext));
    AddTask(new TaskPkgInfoUpdate(m_installerContext));
}

void JobWidgetInstall::appendFotaUpdateTaskList()
{
    _D("Configure installation updated");
    _D("Widget Update");
    m_installerContext.job->SetProgressFlag(true);

    AddTask(new TaskProcessConfig(m_installerContext));
    AddTask(new TaskCertify(m_installerContext));
    AddTask(new TaskUserDataManipulation(m_installerContext));
    if (m_installerContext.needEncryption) {
        AddTask(new TaskEncryptResource(m_installerContext));
    }

    AddTask(new TaskManifestFile(m_installerContext));
    if (m_installerContext.widgetConfig.packagingType ==
        PKG_TYPE_HYBRID_WEB_APP)
    {
        AddTask(new TaskInstallOspsvc(m_installerContext));
    }

    AddTask(new TaskDatabase(m_installerContext));
    AddTask(new TaskAceCheck(m_installerContext));
    //TODO: remove widgetHandle from this task and move before database task
    // by now widget handle is needed in ace check
    // Any error in acecheck while update will break widget
    AddTask(new TaskSmack(m_installerContext));
    AddTask(new TaskRemoveBackupFiles(m_installerContext));
    AddTask(new TaskPkgInfoUpdate(m_installerContext));
}

void JobWidgetInstall::SendProgress()
{
    using namespace PackageManager;
    if (GetProgressFlag() != false) {
        if (GetInstallerStruct().progressCallback != NULL) {
            // send progress signal of pkgmgr
            GetInstallerStruct().pkgmgrInterface->sendProgress(GetProgressPercent());

            _D("Call widget install progressCallback");
            GetInstallerStruct().progressCallback(
                GetInstallerStruct().userParam,
                GetProgressPercent(),
                GetProgressDescription());
        }
    }
}

void JobWidgetInstall::SendProgressIconPath(const std::string &path)
{
    using namespace PackageManager;
    if (GetProgressFlag() != false) {
        if (GetInstallerStruct().progressCallback != NULL) {
            // send progress signal of pkgmgr
            GetInstallerStruct().pkgmgrInterface->sendIconPath(path);
        }
    }
}

void JobWidgetInstall::SendFinishedSuccess()
{
    using namespace PackageManager;
    // TODO : sync should move to separate task.
    sync();

    if (INSTALL_LOCATION_TYPE_PREFER_EXTERNAL == m_installerContext.locationType) {
        if (m_installerContext.isUpdateMode) {
            WidgetInstallToExtSingleton::Instance().postUpgrade(true);
        } else {
            WidgetInstallToExtSingleton::Instance().postInstallation(true);
        }
        WidgetInstallToExtSingleton::Instance().deinitialize();
    }

    //inform widget info
    JobWidgetInstall::displayWidgetInfo();

    TizenAppId& tizenId = m_installerContext.widgetConfig.tzAppid;

    // send signal of pkgmgr
    GetInstallerStruct().pkgmgrInterface->endJob(m_exceptionCaught);

    _D("Call widget install successfinishedCallback");
    GetInstallerStruct().finishedCallback(GetInstallerStruct().userParam,
                                          DPL::ToUTF8String(
                                              tizenId), Jobs::Exceptions::Success);
}

void JobWidgetInstall::SendFinishedFailure()
{
    using namespace PackageManager;

    // print error message
    LOGE(COLOR_ERROR "Error number: %d" COLOR_END, m_exceptionCaught);
    LOGE(COLOR_ERROR "Message: %s" COLOR_END, m_exceptionMessage.c_str());
    fprintf(stderr, "[Err:%d] %s", m_exceptionCaught, m_exceptionMessage.c_str());

    TizenAppId & tizenId = m_installerContext.widgetConfig.tzAppid;

    _D("Call widget install failure finishedCallback");

    // send signal of pkgmgr
    GetInstallerStruct().pkgmgrInterface->endJob(m_exceptionCaught);

    GetInstallerStruct().finishedCallback(GetInstallerStruct().userParam,
                                          DPL::ToUTF8String(
                                              tizenId), m_exceptionCaught);
}

void JobWidgetInstall::SaveExceptionData(const Jobs::JobExceptionBase &e)
{
    m_exceptionCaught = static_cast<Jobs::Exceptions::Type>(e.getParam());
    m_exceptionMessage = e.GetMessage();
}

void JobWidgetInstall::displayWidgetInfo()
{
    if (m_installerContext.widgetConfig.webAppType.appType == WrtDB::APP_TYPE_TIZENWEBAPP)
    {
        WidgetDAOReadOnly dao(m_installerContext.widgetConfig.tzAppid);

        std::ostringstream out;
        WidgetLocalizedInfo localizedInfo =
        W3CFileLocalization::getLocalizedInfo(dao.getTizenAppId());

        out << std::endl <<
        "===================================== INSTALLED WIDGET INFO =========" \
        "============================";
        out << std::endl << "Name:                        " << localizedInfo.name;
    out << std::endl << "AppId:                     " << dao.getTizenAppId();
        WidgetSize size = dao.getPreferredSize();
        out << std::endl << "Width:                       " << size.width;
        out << std::endl << "Height:                      " << size.height;
        out << std::endl << "Start File:                  " <<
    W3CFileLocalization::getStartFile(dao.getTizenAppId());
        out << std::endl << "Version:                     " << dao.getVersion();
        out << std::endl << "Licence:                     " <<
        localizedInfo.license;
        out << std::endl << "Licence Href:                " <<
        localizedInfo.licenseHref;
        out << std::endl << "Description:                 " <<
        localizedInfo.description;
        out << std::endl << "Widget Id:                   " << dao.getGUID();

    OptionalWidgetIcon icon = W3CFileLocalization::getIcon(dao.getTizenAppId());
        DPL::OptionalString iconSrc =
            !!icon ? icon->src : DPL::OptionalString::Null;
        out << std::endl << "Icon:                        " << iconSrc;

        out << std::endl << "Preferences:";
        {
            PropertyDAOReadOnly::WidgetPreferenceList list = dao.getPropertyList();
            FOREACH(it, list)
            {
                out << std::endl << "  Key:                       " <<
                it->key_name;
                out << std::endl << "      Readonly:              " <<
                it->readonly;
            }
        }

        out << std::endl;

        _D("%s", out.str().c_str());
    }
}
} //namespace WidgetInstall
} //namespace Jobs
