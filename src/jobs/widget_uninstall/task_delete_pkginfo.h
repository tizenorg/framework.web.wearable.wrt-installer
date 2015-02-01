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
 * @file    task_delete_pkginfo.h
 * @author  Leerang Song(leerang.song@samsung.com)
 * @version 1.0
 * @brief   Header file for uninstaller task delete package infomation
 */

#ifndef WRT_SRC_INSTALLER_CORE_JOB_WIDGET_UNINSTALL_TASK_DELETE_PKGINFO_H_
#define WRT_SRC_INSTALLER_CORE_JOB_WIDGET_UNINSTALL_TASK_DELETE_PKGINFO_H_

#include <dpl/task.h>
#include <string>

struct UninstallerContext;

namespace Jobs {
namespace WidgetUninstall {
class TaskDeletePkgInfo :
    public DPL::TaskDecl<TaskDeletePkgInfo>
{
    UninstallerContext& m_context;

  private:
    void StepDeletePkgInfo();

    void StartStep();
    void EndStep();

  public:
    TaskDeletePkgInfo(UninstallerContext& context);
};
} //namespace WidgetUninstall
} //namespace Jobs

#endif
// WRT_SRC_INSTALLER_CORE_JOB_WIDGET_UNINSTALL_TASK_DELETE_PKGINFO_H_
