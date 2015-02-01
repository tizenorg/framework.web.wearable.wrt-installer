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
 * @file    task_db_update.cpp
 * @author  Lukasz Wrzosek(l.wrzosek@samsung.com)
 * @version 1.0
 * @brief   Implementation file for uninstaller task database updating
 */

#ifdef DBOX_ENABLED
#include <web_provider_livebox_info.h>
#endif
#include <widget_uninstall/task_db_update.h>
#include <widget_uninstall/job_widget_uninstall.h>
#include <widget_uninstall/widget_uninstall_errors.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/utils/path.h>
#include <ace_api_install.h>
#include <dpl/assert.h>
#include <ace-common/ace_api_common.h>
#include <dpl/wrt-dao-rw/widget_dao.h>
#include <installer_log.h>

using namespace WrtDB;

namespace Jobs {
namespace WidgetUninstall {
TaskDbUpdate::TaskDbUpdate(UninstallerContext& context) :
    DPL::TaskDecl<TaskDbUpdate>(this),
    m_context(context)
{
    AddStep(&TaskDbUpdate::StartStep);
    AddStep(&TaskDbUpdate::StepRemoveExternalLocations);
    AddStep(&TaskDbUpdate::StepDbUpdate);
#ifdef DBOX_ENABLED
    AddStep(&TaskDbUpdate::StepLiveboxDBDelete);
#endif
    AddStep(&TaskDbUpdate::EndStep);
}

TaskDbUpdate::~TaskDbUpdate()
{}

void TaskDbUpdate::StepDbUpdate()
{
    Try
    {
        //TODO: widget handle should not be used any more
        FOREACH(it , m_context.tzAppIdList){
            ace_unregister_widget(static_cast<ace_widget_handle_t>(WidgetDAOReadOnly::getHandle(*it)));
            WidgetDAO::unregisterWidget(*it);
        }
        _D("Unregistered widget successfully!");
    }
    Catch(DPL::DB::SqlConnection::Exception::Base)
    {
        _E("Failed to handle StepDbUpdate!");
        ReThrowMsg(Exceptions::DatabaseFailure,
                   "Failed to handle StepDbUpdate!");
    }
}

#ifdef DBOX_ENABLED
void TaskDbUpdate::StepLiveboxDBDelete()
{
    FOREACH(it, m_context.tzAppIdList){
    int ret =
            web_provider_livebox_delete_by_app_id(DPL::ToUTF8String(*it).c_str());

    if (ret < 0) {
        _D("failed to delete box info");
    } else {
            _D("delete box info: %s", it);
        }
    }
}
#endif

void TaskDbUpdate::StepRemoveExternalLocations()
{
    if (!m_context.removeAbnormal) {
        FOREACH(it, m_context.tzAppIdList){
            WidgetDAO dao(*it);
            _D("Removing external locations:");
            WrtDB::ExternalLocationList externalPaths = dao.getWidgetExternalLocations();
            FOREACH(file, externalPaths)
            {
                DPL::Utils::Path path(*file);
                if(path.Exists()){
                    if(path.IsFile()){
                        _D("  -> %s", path.Fullpath().c_str());
                        Try {
                            DPL::Utils::Remove(path);
                        } Catch(DPL::Utils::Path::BaseException){
                            _E("Failed to remove the file: %s", path.Fullpath().c_str());
                        }
                    } else if (path.IsDir()){
                        _D("  -> %s", path.Fullpath().c_str());
                        Try {
                            DPL::Utils::Remove(path);
                        } Catch(DPL::Utils::Path::BaseException){
                            Throw(Jobs::WidgetUninstall::TaskDbUpdate::
                                Exception::RemoveFilesFailed);
                        }
                    }
                } else {
                    _W("  -> %s(no such a path)", path.Fullpath().c_str());
                }
            }
            dao.unregisterAllExternalLocations();
        }
    }
}

void TaskDbUpdate::StartStep()
{
    LOGI("--------- <TaskDbUpdate> : START ----------");
}

void TaskDbUpdate::EndStep()
{
    m_context.job->UpdateProgress(
        UninstallerContext::UNINSTALL_DB_UPDATE,
        "Widget DB Update Finished");

    LOGI("--------- <TaskDbUpdate> : END ----------");
}
} //namespace WidgetUninstall
} //namespace Jobs
