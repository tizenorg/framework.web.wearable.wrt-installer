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
 * @file    task_update_files.cpp
 * @author  Soyoung Kim (sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task update files
 */

#include <unistd.h>
#include <utility>
#include <vector>
#include <string>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>

#include <dpl/assert.h>
#include <dpl/foreach.h>
#include <dpl/utils/wrt_utility.h>

#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/exception.h>
#include <dpl/errno_string.h>

#include <widget_install/task_update_files.h>
#include <widget_install/widget_install_context.h>
#include <widget_install/widget_install_errors.h>
#include <widget_install/job_widget_install.h>
#include <widget_install/directory_api.h>
#include <widget_install_to_external.h>
#include <pkgmgr/pkgmgr_parser.h>

#include <installer_log.h>

using namespace WrtDB;

namespace {
inline const char* GetWidgetBackupDirPath()
{
    return "backup";
}
}

namespace Jobs {
namespace WidgetInstall {
TaskUpdateFiles::TaskUpdateFiles(InstallerContext& context) :
    DPL::TaskDecl<TaskUpdateFiles>(this),
    m_context(context)
{
    AddStep(&TaskUpdateFiles::StartStep);
    AddStep(&TaskUpdateFiles::StepBackupDirectory);
    AddStep(&TaskUpdateFiles::EndStep);

    AddAbortStep(&TaskUpdateFiles::StepAbortBackupDirectory);
}

void TaskUpdateFiles::StepBackupDirectory()
{
    _D("StepCreateBackupFolder");

    Try {
        std::string pkgid =
            DPL::ToUTF8String(m_context.widgetConfig.tzPkgid);
        if (APP2EXT_SD_CARD == app2ext_get_app_location(pkgid.c_str())) {
            _D("Installed external directory");
            WidgetInstallToExtSingleton::Instance().initialize(pkgid);
            WidgetInstallToExtSingleton::Instance().disable();
        }
    } Catch(WidgetInstallToExt::Exception::ErrorInstallToExt) {
        _E("Failed disable app2sd");
        ReThrowMsg(Exceptions::BackupFailed, "Error occurs during disable app2sd");
    }

    std::string backPath = m_context.locations->getBackupDir();
    _D("Backup resource directory path : %s", backPath.c_str());
    std::string pkgPath = m_context.locations->getPackageInstallationDir();

    if (0 == access(backPath.c_str(), F_OK)) {
        if (!WrtUtilRemove(backPath)) {
            ThrowMsg(Exceptions::RemovingFolderFailure,
                    "Error occurs during removing backup resource directory");
        }
    }
    _D("copy : %s to %s", pkgPath.c_str(), backPath.c_str());
    if ((rename(pkgPath.c_str(), backPath.c_str())) != 0) {
        _E("Failed to rename %s to %s", pkgPath.c_str(), backPath.c_str());
        ThrowMsg(Exceptions::BackupFailed,
                "Error occurs during rename file");
    }

    WrtUtilMakeDir(pkgPath);
}

void TaskUpdateFiles::StepAbortBackupDirectory()
{
    _D("StepAbortCopyFiles");

    std::string backPath = m_context.locations->getBackupDir();
    std::string pkgPath = m_context.locations->getPackageInstallationDir();

    _D("Backup Folder %s to %s", backPath.c_str(), pkgPath.c_str());

    if (!WrtUtilRemove(pkgPath)) {
        _E("Failed to remove %s", pkgPath.c_str());
    }

    if (rename(backPath.c_str(), pkgPath.c_str()) != 0) {
        _E("Failed to rename %s to %s", backPath.c_str(), pkgPath.c_str());
    }
}

void TaskUpdateFiles::StartStep()
{
    LOGI("--------- <TaskUpdateFiles> : START ----------");
}

void TaskUpdateFiles::EndStep()
{
    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_CREATE_BACKUP_DIR,
        "Backup directory created for update");

    LOGI("--------- <TaskUpdateFiles> : END ----------");
}
} //namespace WidgetInstall
} //namespace Jobs
