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
 * @file    task_user_data_manipulation.cpp
 * @author  Soyoung Kim (sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task user data folder 
 */
#ifndef JOS_WIDGET_INSTALL_TASK_USER_DATA_MANIPULATION_H
#define JOS_WIDGET_INSTALL_TASK_USER_DATA_MANIPULATION_H

#include <dpl/task.h>

class InstallerContext;

namespace Jobs {
namespace WidgetInstall {
class TaskUserDataManipulation :
    public DPL::TaskDecl<TaskUserDataManipulation>
{
    InstallerContext& m_context;

    void StartStep();
    void EndStep();
    void StepCreatePrivateStorageDir();
    void StepCreateSharedFolder();
    void StepLinkForPreload();

  public:
    TaskUserDataManipulation(InstallerContext& context);
};
} //namespace WidgetInstall
} //namespace Jobs

#endif //JOS_WIDGET_INSTALL_TASK_USER_DATA_MANIPULATION_H 
