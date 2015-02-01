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
#include "installer_controller.h"
#include <dpl/singleton_impl.h>

IMPLEMENT_SINGLETON(Logic::InstallerController)

namespace Logic {
InstallerController::InstallerController()
{}

void InstallerController::OnEventReceived(
    const InstallerControllerEvents::InstallWidgetEvent &event)
{
    std::string fileName = event.GetArg0();
    std::string pkgId = event.GetArg1();
    Jobs::WidgetInstall::WidgetInstallationStruct installerStruct = event.GetArg2();
    m_installerLogic.InstallWidget(fileName, pkgId, installerStruct);
}

void InstallerController::OnEventReceived(
    const InstallerControllerEvents::InstallPluginEvent &event)
{
    std::string dirName = event.GetArg0();
    PluginInstallerStruct installerStruct = event.GetArg1();
    m_installerLogic.InstallPlugin(dirName, installerStruct);
}

void InstallerController::OnEventReceived(
    const InstallerControllerEvents::UninstallWidgetEvent &event)
{
    std::string widgetPkgName = event.GetArg0();
    WidgetUninstallationStruct uninstallerStruct = event.GetArg1();
    m_installerLogic.UninstallWidget(widgetPkgName, uninstallerStruct);
}

Eina_Bool InstallerController::AddNextStep(void *data)
{
    Jobs::Job* model = static_cast<Jobs::Job *>(data);
    CONTROLLER_POST_EVENT(InstallerController,
                          InstallerControllerEvents::NextStepEvent(model));

    return ECORE_CALLBACK_CANCEL;
}

void InstallerController::OnEventReceived(
    const InstallerControllerEvents::NextStepEvent &event)
{
    Jobs::Job* model = event.GetArg0();
    Assert(model != NULL);

    if (m_installerLogic.NextStep(model)) {
        ecore_idler_add(AddNextStep, model);
    }
}

void InstallerController::OnEventReceived(
    const InstallerControllerEvents::InitializeEvent & /*event*/)
{
    m_installerLogic.Initialize();
}

void InstallerController::OnEventReceived(
    const InstallerControllerEvents::TerminateEvent & /*event*/)
{
    m_installerLogic.Terminate();
}
} //Logic

