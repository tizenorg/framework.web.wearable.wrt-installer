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
 * @file    task_smack.h
 * @author  Piotr Kozbial (p.kozbial@samsung.com)
 * @version 1.0
 * @brief   Header file for installer task smack
 */
#ifndef INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_SMACK_H
#define INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_SMACK_H

#include <dpl/task.h>

class InstallerContext;

namespace Jobs {
namespace WidgetInstall {
class TaskSmack :
    public DPL::TaskDecl<TaskSmack>
{
  private:
    InstallerContext& m_context;
    std::string m_pkgId;

    void StepSetInstall();
    void StepSmackAppPrivilegeVersion();
    void StepSmackFolderLabeling();
    void StepSmackPrivilege();
    void StepAddLabelNPRuntime();
    void StepLabelSignatureFiles();
    void StepRevokeForUpdate();
    void StepAbortSmack();

    bool setLabelForSharedDir(const char* pkgId);

    void StartStep();
    void EndStep();

  public:
    TaskSmack(InstallerContext& context);
};
} //namespace WidgetInstall
} //namespace Jobs

#endif /* INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_SMACK_H */
