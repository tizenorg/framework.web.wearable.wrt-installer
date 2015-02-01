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
 * @file    task_db_update.h
 * @author  Lukasz Wrzosek(l.wrzosek@samsung.com)
 * @version 1.0
 * @brief   Header file for installer task database updating
 */
#ifndef INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_FILE_MANIPULATION_H
#define INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_FILE_MANIPULATION_UPDATE_H

#include <dpl/task.h>
#include <app2ext_interface.h>

class InstallerContext;

namespace Jobs {
namespace WidgetInstall {
class TaskFileManipulation :
    public DPL::TaskDecl<TaskFileManipulation>
{
    InstallerContext& m_context;
    app2ext_handle *m_extHandle;

    // install internal location
    void StepCheckInstallLocation();
    void StepPrepareRootDirectory();
    void StepUnzipWgtFile();
    void StepAbortPrepareRootDirectory();
    void StepLinkForPreload();
    void StartStep();
    void EndStep();

    // install external location
    void prepareExternalDir();

  public:
    TaskFileManipulation(InstallerContext& context);
};
} //namespace WidgetInstall
} //namespace Jobs

#endif // INSTALLER_CORE_JOS_WIDGET_INSTALL_FILE_MANIPULATION_H
