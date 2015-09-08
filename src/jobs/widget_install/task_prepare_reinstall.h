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
 * @file    task_prepare_reinstall.h
 * @author  Jihoon Chung(jihoon.chung@samsung.com)
 * @version 1.0
 * @brief   Header file for installer task prepare reinstalling
 */
#ifndef INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_PREPARE_REINSTALL_H
#define INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_PREPARE_REINSTALL_H

#include <list>
#include <dpl/task.h>

#include <widget_install/widget_install_context.h>

class InstallerContext;

namespace Jobs {
namespace WidgetInstall {
class TaskPrepareReinstall :
    public DPL::TaskDecl<TaskPrepareReinstall>
{
  public:
    TaskPrepareReinstall(InstallerContext& context);

  private:
    // install internal location
    void StepPrepare();
    void StepParseRDSDelta();
    void StepVerifyRDSDelta();
    void StepAddFile();
    void StepDeleteFile();
    void StepModifyFile();
    void StepUpdateSmackLabel();

    void StepAbortPrepareReinstall();

    void StartStep();
    void EndStep();

    InstallerContext& m_context;
    // TODO : replace multimap
    std::list<std::string> m_addFileList;
    std::list<std::string> m_deleteFileList;
    std::list<std::string> m_modifyFileList;
    std::string m_sourcePath;
    std::string m_installedPath;
};
} //namespace WidgetInstall
} //namespace Jobs

#endif // INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_PREPARE_REINSTALL_H
