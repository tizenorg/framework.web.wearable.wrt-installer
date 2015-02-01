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
 * @file    task_db_update.h
 * @author  Lukasz Wrzosek(l.wrzosek@samsung.com)
 * @version 1.0
 * @brief   Header file for uninstaller task database updating
 */

#ifndef WRT_SRC_INSTALLER_CORE_JOB_WIDGET_UNINSTALL_TASK_DB_UPDATE_H_
#define WRT_SRC_INSTALLER_CORE_JOB_WIDGET_UNINSTALL_TASK_DB_UPDATE_H_

#include <string>
#include <dpl/exception.h>
#include <dpl/task.h>

class UninstallerContext;

namespace Jobs {
namespace WidgetUninstall {
class TaskDbUpdate :
    public DPL::TaskDecl<TaskDbUpdate>
{
    class Exception
    {
      public:
        DECLARE_EXCEPTION_TYPE(DPL::Exception, Base)
        DECLARE_EXCEPTION_TYPE(Base, DbStepFailed)
        DECLARE_EXCEPTION_TYPE(Base, RemoveFilesFailed)
    };

    UninstallerContext& m_context;

  private:
    void StepDbUpdate();
    void StepLiveboxDBDelete();
    void StepRemoveExternalLocations();

    void StartStep();
    void EndStep();

  public:
    TaskDbUpdate(UninstallerContext& context);
    virtual ~TaskDbUpdate();
};
} //namespace WidgetUninstall
} //namespace Jobs

#endif // WRT_SRC_INSTALLER_CORE_JOB_WIDGET_UNINSTALL_TASK_DB_UPDATE_H_
