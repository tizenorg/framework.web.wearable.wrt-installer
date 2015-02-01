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
 * @file    job_widget_install.h
 * @author  Radoslaw Wicik r.wicik@samsung.com
 * @author  Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
 * @version 1.0
 * @brief   Implementation file for main installer task
 */
#ifndef WRT_SRC_INSTALLER_CORE_JOB_JOB_WIDGET_INSTALL_H_
#define WRT_SRC_INSTALLER_CORE_JOB_JOB_WIDGET_INSTALL_H_

#include <job.h>
#include <job_base.h>
#include <dpl/utils/widget_version.h>
#include "widget_installer_struct.h"
#include <widget_install/widget_install_context.h>

using namespace Jobs::Exceptions;

namespace Jobs {
namespace WidgetInstall {

typedef JobProgressBase<InstallerContext::InstallStep, InstallerContext::INSTALL_END> InstallerBase;
typedef JobContextBase<Jobs::WidgetInstall::WidgetInstallationStruct> WidgetInstallationBase;

class JobWidgetInstall :
    public Job,
    public InstallerBase,
    public WidgetInstallationBase

{
  private:
    InstallerContext m_installerContext;

    Jobs::Exceptions::Type m_exceptionCaught;
    std::string m_exceptionMessage;

  public:
    /**
     * @brief Automaticaly sets installation process
     */
    JobWidgetInstall(std::string const & widgetPath,
         std::string const &tzPkgId,
         const Jobs::WidgetInstall::WidgetInstallationStruct &installerStruct);

    //overrides
    void SendProgress();
    void SendFinishedSuccess();
    void SendFinishedFailure();
    void SendProgressIconPath(const std::string &path);

    void SaveExceptionData(const Jobs::JobExceptionBase&);
    void displayWidgetInfo();

    //execution paths
    void appendNewInstallationTaskList();
    void appendUpdateInstallationTaskList();
    void appendRDSUpdateTaskList();
    void appendRecoveryTaskList();
    void appendFotaInstallationTaskList();
    void appendFotaUpdateTaskList();

};
} //namespace WidgetInstall
} //namespace Jobs

#endif // WRT_SRC_INSTALLER_CORE_JOB_JOB_WIDGET_INSTALL_H_
