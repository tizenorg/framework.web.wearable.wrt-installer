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
 * @file    task_remove_custom_handlers.h
 * @author  Przemyslaw Ciezkowski (p.ciezkowski@samsung.com)
 * @version 1.0
 * @brief   Header file for uninstaller - remove custom handlers
 */
#ifndef INSTALLER_CORE_JOBS_WIDGET_UNINSTALL_TASK_REMOVE_CUSTOM_HANDLERS_H
#define INSTALLER_CORE_JOBS_WIDGET_UNINSTALL_TASK_REMOVE_CUSTOM_HANDLERS_H

#include <dpl/task.h>

class UninstallerContext;

namespace Jobs {
namespace WidgetUninstall {
class TaskRemoveCustomHandlers :
    public DPL::TaskDecl<TaskRemoveCustomHandlers>
{
  private:
    UninstallerContext& m_context;

    void Step();

    void StartStep();
    void EndStep();

  public:
    TaskRemoveCustomHandlers(UninstallerContext& context);
};
} //namespace WidgetUninstall
} //namespace Jobs

#endif /* INSTALLER_CORE_JOBS_WIDGET_UNINSTALL_TASK_REMOVE_CUSTOM_HANDLERS_H */
