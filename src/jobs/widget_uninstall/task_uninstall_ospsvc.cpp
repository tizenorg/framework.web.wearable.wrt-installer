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
 * @file    task_uninstall_ospsvc.cpp
 * @author  Soyoung Kim(sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Header file for widget uninstall task to uninstall ospsvc
 */
#include <dpl/sstream.h>
#include <dpl/utils/bash_utils.h>
#include <widget_uninstall/task_uninstall_ospsvc.h>
#include <widget_uninstall/job_widget_uninstall.h>
#include <widget_uninstall/uninstaller_context.h>
#include <widget_uninstall/widget_uninstall_errors.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <installer_log.h>

#ifdef CORE_HYBRID
#include <dpl/utils/wrt_utility.h>
#endif

using namespace WrtDB;

namespace {
const int MAX_BUF_SIZE = 128;
const char* OSP_INSTALL_STR = "/usr/etc/package-manager/backend/tpk -uv ";
#ifdef CORE_HYBRID
const char* CORE_INSTALL_STR = "/usr/etc/package-manager/backend/rpm -uv ";
const char* const CORE_MANIFEST_XML = "tizen-manifest.xml";
#endif
}

namespace Jobs {
namespace WidgetUninstall {
TaskUninstallOspsvc::TaskUninstallOspsvc(UninstallerContext& context) :
    DPL::TaskDecl<TaskUninstallOspsvc>(this),
    m_context(context)
{
    AddStep(&TaskUninstallOspsvc::StartStep);
    AddStep(&TaskUninstallOspsvc::StepUninstallOspsvc);
    AddStep(&TaskUninstallOspsvc::EndStep);
}

TaskUninstallOspsvc::~TaskUninstallOspsvc()
{}

void TaskUninstallOspsvc::StepUninstallOspsvc()
{
    _D("Step : Uninstall Osp service");

    std::ostringstream commStr;
#ifdef CORE_HYBRID
    bool isCore = false;
    if(WrtUtilFileExists( m_context.installedPath.Fullpath() + "/" + CORE_MANIFEST_XML))
        isCore = true;
#endif

#ifdef CORE_HYBRID
    if( isCore ){
        commStr << CORE_INSTALL_STR << BashUtils::escape_arg(m_context.tzPkgid);
    }else{
#endif
        commStr << OSP_INSTALL_STR << BashUtils::escape_arg(m_context.tzPkgid);
#ifdef CORE_HYBRID
    }
#endif

    _D("osp uninstall command : %s", commStr.str().c_str());

    char readBuf[MAX_BUF_SIZE];
    FILE *fd;
    fd = popen(commStr.str().c_str(), "r");
    if (NULL == fd) {
        _E("Failed to uninstalltion osp service");
        ThrowMsg(Exceptions::UninstallOspSvcFailed,
                 "Error occurs during\
                uninstall osp service");
    }

    if(fgets(readBuf, MAX_BUF_SIZE, fd) == NULL)
    {
        _E("Failed to uninstalltion osp service\
                        Inability of reading file.");
        ThrowMsg(Exceptions::UninstallOspSvcFailed,
                "Error occurs during\
                uninstall osp service");
    }
    _D("return value : %s", readBuf);

    int result = atoi(readBuf);
    if (0 != result) {
        ThrowMsg(Exceptions::UninstallOspSvcFailed,
                 "Error occurs during\
                install osp service");
    }

    pclose(fd);

    _D("Widget Can be uninstalled. Pkgname : %s", m_context.tzPkgid.c_str());
    m_context.job->UpdateProgress(UninstallerContext::UNINSTALL_REMOVE_OSPSVC,
                                  "Uninstall OSP service finished");
}

void TaskUninstallOspsvc::StartStep()
{
    LOGI("--------- <TaskUninstallOspsvc> : START ----------");
}

void TaskUninstallOspsvc::EndStep()
{
    LOGI("--------- <TaskUninstallOspsvc> : END ----------");
}
} //namespace WidgetUninstall
} //namespace Jobs
