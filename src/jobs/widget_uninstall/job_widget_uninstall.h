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
 * @file job_widet_uninstall.h
 * @brief Uninstaller header file.
 * @author Radoslaw Wicik r.wicik@samsung.com
 */

#ifndef WRT_SRC_INSTALLER_CORE_JOB_WIDGET_UNINSTALL_JOB_WIDGET_UNINSTALL_H_
#define WRT_SRC_INSTALLER_CORE_JOB_WIDGET_UNINSTALL_JOB_WIDGET_UNINSTALL_H_

#include <string>
#include <job.h>
#include <job_base.h>
#include <widget_uninstall/widget_uninstaller_struct.h>
#include <widget_uninstall/uninstaller_context.h>

namespace Jobs {
namespace WidgetUninstall {

enum class WidgetStatus
{
    Ok, NOT_INSTALLED, PREALOAD, ABNORMAL, UNRECOGNIZED
};

typedef JobContextBase<WidgetUninstallationStruct> WidgetUnistallStructBase;
typedef JobProgressBase<UninstallerContext::UninstallStep, UninstallerContext::UNINSTALL_END> UninstallContextBase;

class JobWidgetUninstall :
    public Job,
    public UninstallContextBase,
    public WidgetUnistallStructBase
{
  private:
    UninstallerContext m_context;

    //TODO move it to base class of all jobs
    Jobs::Exceptions::Type m_exceptionCaught;
    std::string m_exceptionMessage;
    // It canbe pkgid or tzAppid
    std::string m_id;

  public:
    /**
     * @brief Uninstaller must to know which widget to uninstall.
     *
     * @param[in] WrtDB::TizenAppId tzAppId - widget to uninstall
     */
    JobWidgetUninstall(const std::string &tizenPkgIdorTizenAppId,
                       const WidgetUninstallationStruct& uninstallerStruct);

    std::string getRemovedTizenId() const;
    bool getRemoveStartedFlag() const;
    bool getRemoveFinishedFlag() const;
    DPL::Utils::Path getManifestFile() const;

    WidgetStatus getWidgetStatus(const std::string &tizenPkgIdorTizenAppId);

    void SendProgress();
    void SendFinishedSuccess();
    void SendFinishedFailure();
    void SaveExceptionData(const Jobs::JobExceptionBase &e);
};
} //namespace WidgetUninstall
} //namespace Jobs

#endif // WRT_SRC_INSTALLER_CORE_JOB_WIDGET_UNINSTALL_JOB_WIDGET_UNINSTALL_H_
