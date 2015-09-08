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
 * @file    task_remove_files.cpp
 * @author  Lukasz Wrzosek(l.wrzosek@samsung.com)
 * @version 1.0
 * @brief   Implementation file for uninstaller task for removing widget files
 */

#include <unistd.h>
#include <widget_uninstall/task_remove_files.h>
#include <widget_uninstall/job_widget_uninstall.h>
#include <widget_uninstall/uninstaller_context.h>
#include <widget_uninstall/widget_uninstall_errors.h>
#include <dpl/wrt-dao-rw/widget_dao.h>
#include <dpl/wrt-dao-ro/widget_config.h>
#include <dpl/assert.h>
#include <dpl/exception.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/utils/path.h>
#include <pkgmgr/pkgmgr_parser.h>
#include <errno.h>
#include <string.h>
#include <widget_install_to_external.h>
#include <installer_log.h>

namespace Jobs {
namespace WidgetUninstall {
using namespace WrtDB;

TaskRemoveFiles::TaskRemoveFiles(UninstallerContext& context) :
    DPL::TaskDecl<TaskRemoveFiles>(this),
    m_context(context)
{
    AddStep(&TaskRemoveFiles::StartStep);
    AddStep(&TaskRemoveFiles::StepRemoveInstallationDirectory);
    AddStep(&TaskRemoveFiles::StepRemoveFinished);
    AddStep(&TaskRemoveFiles::EndStep);
}

TaskRemoveFiles::~TaskRemoveFiles()
{}

void TaskRemoveFiles::StepRemoveInstallationDirectory()
{
    _D("StepRemoveInstallationDirectory started");
    Try {
        int ret = app2ext_get_app_location(m_context.tzPkgid.c_str());

        if (APP2EXT_SD_CARD == ret) {
            _D("Remove external directory");
            Try {
                WidgetInstallToExtSingleton::Instance().initialize(m_context.tzPkgid);
                WidgetInstallToExtSingleton::Instance().uninstallation();
                WidgetInstallToExtSingleton::Instance().deinitialize();
            }
            Catch(WidgetInstallToExt::Exception::ErrorInstallToExt)
            {
                // Continue uninstall even fail to remove external directory.
                // i.e.) SD card isn't inserted or unmounted.
                // This behavior is recommended by platform(app2sd maintainer).
                _E("Fail to remove external directory");
            }
         }
        if (APP2EXT_NOT_INSTALLED != ret) {
            _D("Removing directory");
            m_context.removeStarted = true;
            DPL::Utils::Path widgetDir= m_context.installedPath;
            Try{
                DPL::Utils::Remove(widgetDir);
            } Catch(DPL::Utils::Path::BaseException){
                _E("Removing widget installation directory failed : %s", widgetDir.Fullpath().c_str());
            }
            DPL::Utils::Path dataDir(m_context.locations->getUserDataRootDir());
            Try{
                DPL::Utils::Remove(dataDir);
            } Catch(DPL::Utils::Path::BaseException){
                _W("%s is already removed", dataDir.Fullpath().c_str());
            }
        } else {
            _E("app is not installed");
            ThrowMsg(Exceptions::WidgetNotExist, "failed to get app location");
        }
    } Catch(Exception::RemoveFilesFailed) {
        ThrowMsg(Exceptions::RemoveFileFailure, "Cann't remove directory");
    }
    m_context.job->UpdateProgress(
        UninstallerContext::UNINSTALL_REMOVE_WIDGETDIR,
        "Widget INstallation Directory Removal Finished");
}

void TaskRemoveFiles::StepRemoveFinished()
{
    _D("StepRemoveFinished finished");

    m_context.job->UpdateProgress(
        UninstallerContext::UNINSTALL_REMOVE_FINISHED,
        "Widget remove steps Finished");
}

void TaskRemoveFiles::StartStep()
{
    LOGI("--------- <TaskRemoveFiles> : START ----------");
}

void TaskRemoveFiles::EndStep()
{
    LOGI("--------- <TaskRemoveFiles> : END ----------");
}
} //namespace WidgetUninstall
} //namespace Jobs
