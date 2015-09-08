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
 * @file    task_install_ospsvc.cpp
 * @author  Soyoung Kim (sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task  install osp service
 */
#include "task_install_ospsvc.h"

#include <unistd.h>
#include <string>

#include <pkgmgr/pkgmgr_parser.h>
#include <pkgmgr-info.h>
#include <fstream>
#include <dpl/errno_string.h>
#include <dpl/foreach.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/utils/bash_utils.h>
#include <privilege-control.h>

#include <widget_install/job_widget_install.h>
#include <widget_install/widget_install_context.h>
#include <widget_install/widget_install_errors.h>

#include <installer_log.h>

#ifdef CORE_HYBRID
#include <dpl/utils/wrt_utility.h>
#endif

using namespace WrtDB;

namespace {
const int MAX_BUF_SIZE = 128;
const char* OSP_INSTALL_STR1 = "/usr/etc/package-manager/backend/tpk -iv ";
const char* OSP_INSTALL_STR2 = " -p ";

#ifdef CORE_HYBRID
const char* CORE_INSTALL_STR1 = "/usr/etc/package-manager/backend/rpm -iv ";
const char* CORE_INSTALL_STR2 = " -p ";
const char* const CORE_MANIFEST_XML = "tizen-manifest.xml";
#endif

}

namespace Jobs {
namespace WidgetInstall {
TaskInstallOspsvc::TaskInstallOspsvc(InstallerContext& context) :
    DPL::TaskDecl<TaskInstallOspsvc>(this),
    m_context(context)
{
    AddStep(&TaskInstallOspsvc::StartStep);
    AddStep(&TaskInstallOspsvc::StepUninstallSmack);
    AddStep(&TaskInstallOspsvc::StepInstallOspService);
    AddStep(&TaskInstallOspsvc::EndStep);
}

void TaskInstallOspsvc::StepUninstallSmack()
{
    std::string pkgId = DPL::ToUTF8String(m_context.widgetConfig.tzPkgid);
    if (m_context.isUpdateMode) {
        _D("StepUninstallSmack");
        if (PC_OPERATION_SUCCESS != perm_app_uninstall(pkgId.c_str())) {
            _E("failure in removing smack rules file");
            ThrowMsg(Exceptions::NotAllowed, "Update failure. "
                    "failure in delete smack rules file before update.");
        }
    }
}

void TaskInstallOspsvc::StepInstallOspService()
{
    _D("Step: installation for hybrid service");

    std::ostringstream commStr;

#ifdef CORE_HYBRID
    bool isCore = false;
    if(WrtUtilFileExists( m_context.locations->getPackageInstallationDir() + "/" + CORE_MANIFEST_XML))
        isCore = true;
#endif

#ifdef CORE_HYBRID
    if( isCore ){
        commStr << CORE_INSTALL_STR1 << BashUtils::escape_arg(
            m_context.locations->getPackageInstallationDir())
            << CORE_INSTALL_STR2 << m_context.certLevel;
    }else{
#endif
    commStr << OSP_INSTALL_STR1 << BashUtils::escape_arg(
        m_context.locations->getPackageInstallationDir())
        << OSP_INSTALL_STR2 << m_context.certLevel;
#ifdef CORE_HYBRID
    }
#endif

    _D("hybrid install command : %s", commStr.str().c_str());

    char readBuf[MAX_BUF_SIZE];
    FILE *fd;
    fd = popen(commStr.str().c_str(), "r");
    if (NULL == fd) {
        _E("Failed to installtion hybrid service");
        ThrowMsg(Exceptions::InstallOspsvcFailed,
                 "Error occurs during\
                install hybrid service");
    }

    if (fgets(readBuf, MAX_BUF_SIZE, fd) == NULL)
    {
        _E("Failed to installtion hybrid service.\
                Inability of reading file.");
        ThrowMsg(Exceptions::InstallOspsvcFailed,
                "Error occurs during\
                install hybrid service");
    }
    _D("return value : %s", readBuf);

    int result = atoi(readBuf);
    if (0 != result) {
        ThrowMsg(Exceptions::InstallOspsvcFailed,
                 "Error occurs during\
                install hybrid service");
    }

    pclose(fd);
}

void TaskInstallOspsvc::StartStep()
{
    LOGI("--------- <TaskInstallOspsvc> : START ----------");
}

void TaskInstallOspsvc::EndStep()
{
    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_INSTALL_OSPSVC,
        "Installed Osp servcie");

    LOGI("--------- <TaskInstallOspsvc> : END ----------");
}
} //namespace WidgetInstall
} //namespace Jobs
