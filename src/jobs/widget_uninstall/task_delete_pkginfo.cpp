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
 * @file    task_delete_pkginfo.cpp
 * @author  Leerang Song(leerang.song@samsung.com)
 * @version 1.0
 * @brief   Implementation file for uninstaller delete package information
 */

#include <string.h>
#include <widget_uninstall/task_delete_pkginfo.h>
#include <widget_uninstall/job_widget_uninstall.h>
#include <widget_uninstall/widget_uninstall_errors.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <pkgmgr/pkgmgr_parser.h>
#include <dpl/assert.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/utils/path.h>
#include <installer_log.h>

namespace Jobs {
namespace WidgetUninstall {
TaskDeletePkgInfo::TaskDeletePkgInfo(
    UninstallerContext& context) :
    DPL::TaskDecl<TaskDeletePkgInfo>(this),
    m_context(context)
{
    AddStep(&TaskDeletePkgInfo::StartStep);
    AddStep(&TaskDeletePkgInfo::StepDeletePkgInfo);
    AddStep(&TaskDeletePkgInfo::EndStep);
}

void TaskDeletePkgInfo::StartStep()
{
    LOGI("--------- <TaskDeletePkgInfo> : START ----------");
}

void TaskDeletePkgInfo::EndStep()
{
    LOGI("--------- <TaskDeletePkgInfo> : END ----------");
}

void TaskDeletePkgInfo::StepDeletePkgInfo()
{
    std::ostringstream manifest_name;
    manifest_name << m_context.tzPkgid << ".xml";
    DPL::Utils::Path pre_manifest("/usr/share/packages");
    pre_manifest /= manifest_name.str();

    if (!(m_context.manifestFile.Exists() == 0 && pre_manifest.Exists())) {
        if (0 !=  pkgmgr_parser_parse_manifest_for_uninstallation(
                    m_context.manifestFile.Fullpath().c_str(), NULL)) {
            _W("Manifest file failed to parse for uninstallation");
        }
    }
    if (!DPL::Utils::TryRemove(m_context.manifestFile)) {
        _W("No manifest file found: %s", m_context.manifestFile.Fullpath().c_str());
    } else {
        _D("Manifest file removed: %s", m_context.manifestFile.Fullpath().c_str());
    }
}
} //namespace WidgetUninstall
} //namespace Jobs
