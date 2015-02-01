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
 * @file    job_plugin_install.cpp
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @version
 * @brief
 */
#include <plugin_install/job_plugin_install.h>
#include <plugin_install/plugin_install_task.h>
#include "plugin_objects.h"
#include <wrt_common_types.h>
#include <installer_log.h>

namespace Jobs {
namespace PluginInstall {
JobPluginInstall::JobPluginInstall(PluginPath const &pluginPath,
                                   const PluginInstallerStruct &installerStruct)
    :
    Job(PluginInstallation),
    JobContextBase<PluginInstallerStruct>(installerStruct),
    m_exceptionCaught(Jobs::Exceptions::Success)
{
    //
    // Init installer context
    //
    m_context.pluginFilePath = pluginPath;
    m_context.pluginHandle = INVALID_HANDLE;
    m_context.installationCompleted = false;

    m_context.installerTask = this;
    //
    // Create main installation tasks
    //
    AddTask(new PluginInstallTask(&m_context));
}

void JobPluginInstall::SendProgress()
{
    if (GetProgressFlag() && GetInstallerStruct().progressCallback != NULL) {
        _D("Call Plugin install progressCallback");
        GetInstallerStruct().progressCallback(GetInstallerStruct().userParam,
                                              GetProgressPercent(),
                                              GetProgressDescription());
    }
}

void JobPluginInstall::SendFinishedSuccess()
{
    PluginHandle handle = getNewPluginHandle();

    if (handle != Jobs::PluginInstall::JobPluginInstall::INVALID_HANDLE &&
        isReadyToInstall())
    {
        _D("Call Plugin install success finishedCallback");
        GetInstallerStruct().finishedCallback(GetInstallerStruct().userParam,
                                              Jobs::Exceptions::Success);
    } else {
        _D("Call Plugin install waiting finishedCallback");
        GetInstallerStruct().finishedCallback(GetInstallerStruct().userParam,
                                              Jobs::Exceptions::ErrorPluginInstallationFailed);

        _D("Installation: %s NOT possible", getFilePath().c_str());
    }
}

void JobPluginInstall::SendFinishedFailure()
{
    LOGE(COLOR_ERROR "Error in plugin installation step: %d" COLOR_END, m_exceptionCaught);
    LOGE(COLOR_ERROR "Message: %s" COLOR_END, m_exceptionMessage.c_str());
    fprintf(stderr, "[Err:%d] %s", m_exceptionCaught, m_exceptionMessage.c_str());

    _D("Call Plugin install failure finishedCallback");
    GetInstallerStruct().finishedCallback(GetInstallerStruct().userParam,
                                          m_exceptionCaught);
}

void JobPluginInstall::SaveExceptionData(const Jobs::JobExceptionBase &e)
{
    m_exceptionCaught = static_cast<Jobs::Exceptions::Type>(e.getParam());
    m_exceptionMessage = e.GetMessage();
}
} //namespace Jobs
} //namespace PluginInstall
