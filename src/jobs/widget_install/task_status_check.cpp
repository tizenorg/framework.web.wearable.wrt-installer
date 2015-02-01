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
 * @file    task_status_check.cpp
 * @author  Soyoung Kim (sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task  install osp service
 */
#include "task_status_check.h"

#include <unistd.h>
#include <string>

#include <app_manager.h>
#include <web_provider_service.h>
#include <dpl/errno_string.h>
#include <dpl/utils/bash_utils.h>

#include <widget_install/job_widget_install.h>
#include <widget_install/widget_install_context.h>
#include <widget_install/widget_install_errors.h>

#include <installer_log.h>

namespace Jobs {
namespace WidgetInstall {
TaskStatusCheck::TaskStatusCheck(InstallerContext& context) :
    DPL::TaskDecl<TaskStatusCheck>(this),
    m_context(context)
{
    AddStep(&TaskStatusCheck::StartStep);
    AddStep(&TaskStatusCheck::CheckAppRunningStateStep);
    AddStep(&TaskStatusCheck::DynamicBoxesRemoveStep);
    AddStep(&TaskStatusCheck::EndStep);
}

void TaskStatusCheck::CheckAppRunningStateStep()
{
    _D("Step: CheckAppRunningStateStep");
    std::string webAppPkgId = DPL::ToUTF8String(m_context.widgetConfig.tzPkgid);

    int ret = 0;
    pkgmgrinfo_pkginfo_h handle;
    ret = pkgmgrinfo_pkginfo_get_pkginfo(webAppPkgId.c_str(), &handle);
    if (ret != PMINFO_R_OK) {
        _E("Can't find [%s] in package manager", webAppPkgId.c_str());
        ThrowMsg(Jobs::WidgetInstall::Exceptions::GetInfoPkgMgrFailed,
                "package id can't find from package manager");
    }
    ret = pkgmgrinfo_appinfo_get_list(handle, PMINFO_ALL_APP,
            terminateRunningApp, NULL);
    if (ret != PMINFO_R_OK) {
        pkgmgrinfo_pkginfo_destroy_pkginfo(handle);
        ThrowMsg(Jobs::WidgetInstall::Exceptions::WidgetRunningError,
                "widget is running");
    }
    pkgmgrinfo_pkginfo_destroy_pkginfo(handle);
}

int TaskStatusCheck::terminateRunningApp(pkgmgrinfo_appinfo_h handle,
        void* /*user_data*/)
{
    char *appId = NULL;
    pkgmgrinfo_appinfo_get_appid(handle, &appId);
    _D("# Terminating App : [%s]", appId);

    bool isRunning = false;
    int ret = app_manager_is_running(appId, &isRunning);
    if (APP_MANAGER_ERROR_NONE != ret) {
        _E("Fail to get running state");
        ThrowMsg(Jobs::WidgetInstall::Exceptions::WidgetRunningError,
                "widget is running");
    }

    if (true == isRunning) {
        // get app_context for running application
        // app_context must be released with app_context_destroy
        app_context_h appCtx = NULL;
        ret =
            app_manager_get_app_context(appId, &appCtx);
        if (APP_MANAGER_ERROR_NONE != ret) {
            _E("Fail to get app_context");
            ThrowMsg(Jobs::WidgetInstall::Exceptions::WidgetRunningError,
                    "widget is running");
        }

        // terminate app_context_h
        ret = app_manager_terminate_app(appCtx);
        if (APP_MANAGER_ERROR_NONE != ret) {
            _E("Fail to terminate running application");
            app_context_destroy(appCtx);
            ThrowMsg(Jobs::WidgetInstall::Exceptions::WidgetRunningError,
                    "widget is running");
        } else {
            app_context_destroy(appCtx);
            // app_manager_terminate_app isn't sync API
            // wait until application isn't running (50ms * 100)
            bool isStillRunning = true;
            int checkingloop = 100;
            struct timespec duration = { 0, 50 * 1000 * 1000 };
            while (--checkingloop >= 0) {
                nanosleep(&duration, NULL);
                int ret = app_manager_is_running(appId,
                        &isStillRunning);
                if (APP_MANAGER_ERROR_NONE != ret) {
                    _E("Fail to get running state");
                    ThrowMsg(Jobs::WidgetInstall::Exceptions::WidgetRunningError,
                            "widget is running");
                }
                if (!isStillRunning) {
                    break;
                }
            }
            if (isStillRunning) {
                _E("Fail to terminate running application");
                ThrowMsg(Jobs::WidgetInstall::Exceptions::WidgetRunningError,
                        "widget is running");
            }
            _D("terminate application");
        }
    }
    return 0;
}

void TaskStatusCheck::DynamicBoxesRemoveStep()
{
    _D("Step: DynamicBoxesRemoveStep");

    // check if this webapp has dynaimc boxes, and request to remove them
    web_provider_service_wait_boxes_removed(
            DPL::ToUTF8String(m_context.widgetConfig.tzAppid).c_str());

    _D("Finished to handle this web app's dynamic box");
}

void TaskStatusCheck::StartStep()
{
    LOGI("--------- <TaskStatusCheck> : START ----------");
}

void TaskStatusCheck::EndStep()
{
    LOGI("--------- <TaskStatusCheck> : END ----------");
}
} //namespace WidgetInstall
} //namespace Jobs
