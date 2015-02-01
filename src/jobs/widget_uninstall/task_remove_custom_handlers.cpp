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
 * @file    task_remove_custom_handlers.cpp
 * @author  Przemyslaw Ciezkowski (p.ciezkowski@samsung.com)
 * @version 1.0
 * @brief   File for uninstaller - remove custom handlers
 */

#include <widget_uninstall/task_remove_custom_handlers.h>
#include <widget_uninstall/uninstaller_context.h>
#include <dpl/optional_typedefs.h>
#include <appsvc.h>
#include <wrt-commons/custom-handler-dao-ro/CustomHandlerDatabase.h>
#include <wrt-commons/custom-handler-dao-rw/custom_handler_dao.h>
#include <installer_log.h>

namespace Jobs {
namespace WidgetUninstall {
TaskRemoveCustomHandlers::TaskRemoveCustomHandlers(UninstallerContext& context)
    :
    DPL::TaskDecl<TaskRemoveCustomHandlers>(this),
    m_context(context)
{
    AddStep(&TaskRemoveCustomHandlers::StartStep);
    AddStep(&TaskRemoveCustomHandlers::Step);
    AddStep(&TaskRemoveCustomHandlers::EndStep);
}

void TaskRemoveCustomHandlers::Step()
{
    CustomHandlerDB::Interface::attachDatabaseRW();
    FOREACH(it, m_context.tzAppIdList){
    _D("Removing widget from appsvc");
        int result = appsvc_unset_defapp(DPL::ToUTF8String(*it).c_str());
    _D("Result: %d", result);

        CustomHandlerDB::CustomHandlerDAO handlersDao(*it);
    handlersDao.removeWidgetProtocolHandlers();
    handlersDao.removeWidgetContentHandlers();
    }
    CustomHandlerDB::Interface::detachDatabase();
}

void TaskRemoveCustomHandlers::StartStep()
{
    LOGI("--------- <TaskRemoveCustomHandlers> : START ----------");
}

void TaskRemoveCustomHandlers::EndStep()
{
    LOGI("--------- <TaskRemoveCustomHandlers> : END ----------");
}
} //namespace WidgetUninstall
} //namespace Jobs
