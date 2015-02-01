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
 * @file       task_install_ospsvc.h
 * @author     soyoung kim (sy037.kim@samsung.com)
 * @version    1.0
 */

#ifndef SRC_JOBS_WIDGET_INSTALL_TASK_INSTALL_OSPSVC_H_
#define SRC_JOBS_WIDGET_INSTALL_TASK_INSTALL_OSPSVC_H_

#include <dpl/task.h>
#include <string>

class InstallerContext;

namespace Jobs {
namespace WidgetInstall {
class TaskInstallOspsvc : public DPL::TaskDecl<TaskInstallOspsvc>
{
  private:
    // Installation context
    InstallerContext &m_context;

    void StepInstallOspService();
    void StepUninstallSmack();

    void StepAbortInstall();

    void StartStep();
    void EndStep();

    // return callback
    static int StatusCallback(
        int req_id, const char *pkg_type, const char *pkg_name,
        const char *key, const char *val, const void *pmsg,
        void *priv_data);

  public:
    explicit TaskInstallOspsvc(InstallerContext &installerContext);
};
} // namespace WidgetInstall
} // namespace Jobs
#endif /* SRC_JOBS_WIDGET_INSTALL_TASK_INSTALL_OSPSVC_H_ */
