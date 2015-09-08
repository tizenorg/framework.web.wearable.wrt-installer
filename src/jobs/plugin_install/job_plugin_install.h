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
 * @file    job_plugin_install.h
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @version
 * @brief
 */

#ifndef WRT_SRC_INSTALLER_CORE_JOB_JOB_PLUGIN_INSTALL_H_
#define WRT_SRC_INSTALLER_CORE_JOB_JOB_PLUGIN_INSTALL_H_

//SYSTEM INCLUDES
#include <string>

//WRT INCLUDES
#include <job.h>
#include <job_base.h>
#include <plugin_install/plugin_installer_struct.h>
#include <plugin_install/plugin_installer_context.h>
namespace Jobs {
namespace PluginInstall {
class JobPluginInstall :
    public Job,
    public JobProgressBase<PluginInstallerContext::PluginInstallStep,
                           PluginInstallerContext::PLUGIN_INSTALL_END>,
    public JobContextBase<PluginInstallerStruct>
{
  public:
    static const WrtDB::DbPluginHandle INVALID_HANDLE = -1;

  public:
    /**
     * @brief Automaticaly sets installation process
     */
    JobPluginInstall(PluginPath const &pluginPath,
                     const PluginInstallerStruct &installerStruct);

    WrtDB::DbPluginHandle getNewPluginHandle() const
    {
        return m_context.pluginHandle;
    }
    std::string getFilePath() const
    {
        return m_context.pluginFilePath.Fullpath();
    }
    bool isReadyToInstall() const
    {
        return m_context.installationCompleted;
    }

    void SendProgress();
    void SendFinishedSuccess();
    void SendFinishedFailure();
    void SaveExceptionData(const Jobs::JobExceptionBase &e);

  private:
   PluginInstallerContext m_context;

    //TODO move it to base class of all jobs
    //maybe separate JobBase class for this?
    Jobs::Exceptions::Type m_exceptionCaught;
    std::string m_exceptionMessage;
};
} //namespace Jobs
} //namespace PluginInstall

#endif /* WRT_SRC_INSTALLER_CORE_JOB_JOB_PLUGIN_INSTALL_H_ */
