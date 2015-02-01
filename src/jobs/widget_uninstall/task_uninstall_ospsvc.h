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
 * @file    task_uninstall_ospsvc.h
 * @author  Pawel Sikorski(p.sikorski@samsung.com)
 * @version 1.0
 * @brief   Header file for widget uninstall task to uninstall ospsvc
 */

#ifndef WRT_SRC_INSTALLER_CORE_JOB_WIDGET_UNINSTALL_TASK_UNINSTALL_OSPSVC_H
#define WRT_SRC_INSTALLER_CORE_JOB_WIDGET_UNINSTALL_TASK_UNINSTALL_OSPSVC_H

#include <dpl/task.h>

struct UninstallerContext; //forward declaration

namespace Jobs {
namespace WidgetUninstall {
class TaskUninstallOspsvc :
    public DPL::TaskDecl<TaskUninstallOspsvc>
{
  private:
    //context
    UninstallerContext& m_context;

    //steps
    void StepUninstallOspsvc();

    void StartStep();
    void EndStep();

  public:
    TaskUninstallOspsvc(UninstallerContext& context);
    virtual ~TaskUninstallOspsvc();
};
} //namespace WidgetUninstall
} //namespace Jobs

#endif /* WRT_SRC_INSTALLER_CORE_JOB_WIDGET_UNINSTALL_TASK_UNINSTALL_OSPSVC_H */
