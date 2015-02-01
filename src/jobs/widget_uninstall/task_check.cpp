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
 * @file    task_check.cpp
 * @author  Pawel Sikorski(p.sikorski@samsung.com)
 * @version 1.0
 * @brief   Header file for widget uninstall task check
 */
#include <ctime>
#include <dpl/sstream.h>
#include <widget_uninstall/task_check.h>
#include <widget_uninstall/job_widget_uninstall.h>
#include <widget_uninstall/uninstaller_context.h>
#include <widget_uninstall/widget_uninstall_errors.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <app_manager.h>
#include <pkgmgr/pkgmgr_parser.h>
#include <pkgmgr-info.h>
#include <installer_log.h>
#include <web_provider_service.h>

namespace Jobs {
namespace WidgetUninstall {
TaskCheck::TaskCheck(UninstallerContext& context) :
    DPL::TaskDecl<TaskCheck>(this),
    m_context(context)
{
    AddStep(&TaskCheck::StartStep);
    AddStep(&TaskCheck::StepUninstallPreCheck);
    AddStep(&TaskCheck::StepCheckMDM);
    AddStep(&TaskCheck::EndStep);
}

TaskCheck::~TaskCheck()
{}

void TaskCheck::StartStep()
{
    LOGI("--------- <TaskCheck> : START ----------");
}

void TaskCheck::EndStep()
{
    m_context.job->UpdateProgress(UninstallerContext::UNINSTALL_PRECHECK,
                                  "Uninstall pre-checking Finished");
    LOGI("--------- <TaskCheck> : END ----------");
}

void TaskCheck::StepUninstallPreCheck()
{
    FOREACH( it , m_context.tzAppIdList){
        bool isRunning = false;
        std::string tzAppId = DPL::ToUTF8String(*it);
        int ret = app_manager_is_running(tzAppId.c_str(), &isRunning);
        if (APP_MANAGER_ERROR_NONE != ret) {
            _E("Fail to get running state");
            ThrowMsg(Exceptions::PlatformAPIFailure,
                 "Fail to get widget state");
        }

        if (true == isRunning) {
            // get app_context for running application
            // app_context must be released with app_context_destroy
            app_context_h appCtx = NULL;
            ret = app_manager_get_app_context(tzAppId.c_str(), &appCtx);
            if (APP_MANAGER_ERROR_NONE != ret) {
                _E("Fail to get app_context");
                ThrowMsg(Exceptions::AppIsRunning,
                     "Widget is not stopped. Cannot uninstall!");
            }

            // terminate app_context_h
            ret = app_manager_terminate_app(appCtx);
            if (APP_MANAGER_ERROR_NONE != ret) {
                _E("Fail to terminate running application");
                app_context_destroy(appCtx);
                ThrowMsg(Exceptions::AppIsRunning,
                     "Widget is not stopped. Cannot uninstall!");
            } else {
                app_context_destroy(appCtx);
                // app_manager_terminate_app isn't sync API
                // wait until application isn't running (50ms * 100)
                bool isStillRunning = true;
                int checkingloop = 100;
                struct timespec duration = { 0, 50 * 1000 * 1000 };
                while (--checkingloop >= 0) {
                    nanosleep(&duration, NULL);
                    int ret = app_manager_is_running(tzAppId.c_str(), &isStillRunning);
                    if (APP_MANAGER_ERROR_NONE != ret) {
                        _E("Fail to get running state");
                        ThrowMsg(Exceptions::PlatformAPIFailure,
                             "Fail to get widget state");
                    }
                    if (!isStillRunning) {
                        break;
                    }
                }
                if (isStillRunning) {
                    _E("Fail to terminate running application");
                    ThrowMsg(Exceptions::AppIsRunning,
                         "Widget is not stopped. Cannot uninstall!");
                }
                _D("terminate application");
            }
        }
        // TODO: if web package provides mutli-app in one package, we have to call below API with pkgId
        web_provider_service_wait_boxes_removed(tzAppId.c_str());
        _D("web app(%s) and its dynamic boxes can be terminated.", tzAppId.c_str());
    }

    // check if this webapp has dynamic boxes, and request to remove them
    //web_provider_service_wait_boxes_removed(m_context.tzAppid.c_str());
    //_D("web app(%s) and its dynamic boxes can be terminated.", m_context.tzAppid.c_str());
}

void TaskCheck::StepCheckMDM()
{
    _D("StepCheckMDM");

    if (PMINFO_R_OK !=  pkgmgr_parser_check_mdm_policy_for_uninstallation(
                m_context.manifestFile.Fullpath().c_str())) {
        _E("Failed to check mdm policy");
        ThrowMsg(Exceptions::CheckMDMPolicyFailure, "Can't uninstall! Because of MDM policy");
    }
}
} //namespace WidgetUninstall
} //namespace Jobs
