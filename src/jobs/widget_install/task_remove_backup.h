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
 * @file    task_remove_backup.h
 * @author  Soyoung kim(sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Header file for installer task backup files remove
 */
#ifndef INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_REMOVE_BACKUP_FILES_H
#define INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_REMOVE_BACKUP_FILES_H

#include <dpl/task.h>

class InstallerContext;

namespace Jobs {
namespace WidgetInstall {
class TaskRemoveBackupFiles :
    public DPL::TaskDecl<TaskRemoveBackupFiles>
{
  private:
    InstallerContext& m_context;

    void StepRemoveBackupFiles();
    void StepDeleteBackupDB();
    void StepDeleteBackupWidgetInterfaceDB();

    void StartStep();
    void EndStep();

  public:
    TaskRemoveBackupFiles(InstallerContext& context);
};
} //namespace WidgetInstall
} //namespace Jobs

#endif // INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_REMOVE_BACKUP_FILES_H
