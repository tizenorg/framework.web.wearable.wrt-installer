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
 * @file    task_smack.cpp
 * @author  Piotr Kozbial (p.kozbial@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task smack
 */

#include <widget_uninstall/task_smack.h>
#include <widget_uninstall/job_widget_uninstall.h>
#include <widget_uninstall/uninstaller_context.h>
#include <dpl/optional_typedefs.h>
#include <installer_log.h>
#include <privilege-control.h>

namespace Jobs {
namespace WidgetUninstall {
TaskSmack::TaskSmack(UninstallerContext& context) :
    DPL::TaskDecl<TaskSmack>(this),
    m_context(context)
{
    AddStep(&TaskSmack::StartStep);
    AddStep(&TaskSmack::Step);
    AddStep(&TaskSmack::EndStep);
}

void TaskSmack::Step()
{
    _D("------------------------> SMACK: Jobs::WidgetUninstall::TaskSmack::Step()");

    if (PC_OPERATION_SUCCESS != perm_begin()) {
        _E("failure in smack transaction begin");
        return;
    }

    const char* pkgId = m_context.tzPkgid.c_str();
    if (PC_OPERATION_SUCCESS != perm_app_revoke_permissions(pkgId)) {
        _E("failure in revoking smack permissions");
    }

    if (PC_OPERATION_SUCCESS != perm_app_uninstall(pkgId)) {
        _E("failure in removing smack rules file");
    }

    if (PC_OPERATION_SUCCESS != perm_end()) {
        _E("failure in smack transaction end");
        return;
    }
}

void TaskSmack::StartStep()
{
    LOGI("--------- <TaskSmack> : START ----------");
}

void TaskSmack::EndStep()
{
    m_context.job->UpdateProgress(
        UninstallerContext::UNINSTALL_SMACK_DISABLE,
        "Widget SMACK Disabled");

    LOGI("--------- <TaskSmack> : END ----------");
}
} //namespace WidgetUninstall
} //namespace Jobs
