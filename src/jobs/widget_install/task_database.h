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
 * @file    task_database.h
 * @author  Lukasz Wrzosek(l.wrzosek@samsung.com)
 * @author  Soyoung kim(sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Header file for installer task database updating
 */
#ifndef INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_DATABASE_H
#define INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_DATABASE_H

#include <dpl/task.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>

#include <dpl/wrt-dao-ro/widget_dao_read_only.h>

class InstallerContext;

namespace Jobs {
namespace WidgetInstall {
class TaskDatabase :
    public DPL::TaskDecl<TaskDatabase>
{
  private:
    InstallerContext& m_context;
    WrtDB::ExternalLocationList m_externalLocationsToRemove;

    //TODO: temporary needed until security-server start to use pkgName instead
    //of widget handle
    std::list<WrtDB::DbWidgetHandle> m_handleToRemoveList;
    std::list<WrtDB::DbWidgetHandle> m_handleList;
    std::list<WrtDB::TizenAppId> m_backAppIdList;
    WrtDB::TizenAppId m_orginAppId;

    void StepRegisterExternalFiles();
    void StepWrtDBInsert();
    void StepAceDBInsert();
    void StepSecurityOriginDBInsert();
    void StepWidgetInterfaceDBInsert();
    void StepRemoveExternalFiles();
    void StepLiveboxDBInsert();

    void StepAbortDBInsert();
    void StepAbortAceDBInsert();
    void StepAbortWidgetInterfaceDBInsert();

    void StartStep();
    void EndStep();

  public:
    TaskDatabase(InstallerContext& context);
};
} //namespace WidgetInstall
} //namespace Jobs

#endif // INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_DATABASE_H
