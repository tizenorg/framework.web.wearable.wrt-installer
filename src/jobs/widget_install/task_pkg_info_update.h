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
 * @file       task_pkg_info_update.h
 * @author     soyoung kim (sy037.kim@samsung.com)
 * @version    1.0
 */

#ifndef SRC_JOBS_WIDGET_INSTALL_TASK_PKG_INFO_UPDATE_H_
#define SRC_JOBS_WIDGET_INSTALL_TASK_PKG_INFO_UPDATE_H_

#include <dpl/task.h>
#include <string>
#include <pkgmgr_installer.h>

class InstallerContext;

namespace Jobs {
namespace WidgetInstall {
class TaskPkgInfoUpdate : public DPL::TaskDecl<TaskPkgInfoUpdate>
{
  private:
    // Installation context
    InstallerContext &m_context;

    void StepPkgInfo();
    void StepSetCertiInfo();
    void SetCertiInfo(int source);
    void StepSetEndofInstallation();

    void stepAbortParseManifest();
    void StepAbortCertiInfo();

    void StartStep();
    void EndStep();

    pkgmgr_instcertinfo_h m_pkgHandle;
    std::string m_manifest;

  public:
    explicit TaskPkgInfoUpdate(InstallerContext &installerContext);
};
} // namespace WidgetInstall
} // namespace Jobs
#endif /* SRC_JOBS_WIDGET_INSTALL_TASK_PKG_INFO_UPDATE_H_ */
