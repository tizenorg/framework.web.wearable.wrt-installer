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
#ifndef WRT_SRC_INSTALLER_CORE_INSTALLER_CONTROLLER_H_
#define WRT_SRC_INSTALLER_CORE_INSTALLER_CONTROLLER_H_

#include <dpl/singleton.h>
#include <dpl/event/controller.h>
#include <dpl/generic_event.h>
#include <string>
#include <map>
#include <dpl/task.h>
#include <dpl/type_list.h>
#include <widget_install/widget_installer_struct.h>
#include <installer_logic.h>
#include <job.h>

/**
 * @brief holds events send to InstallControler
 */
namespace InstallerControllerEvents {
/**
 * @brief Event for inicieting instalation process.
 *
 * This event holds std::string witch should be path to widget package
 */
DECLARE_GENERIC_EVENT_3(InstallWidgetEvent,
            std::string,              // zipFileName
            std::string,              // package id
            Jobs::WidgetInstall::WidgetInstallationStruct) // installerStruct

/**
 * @brief Event for iniciating plugin instalation process.
 * This event holds std::string witch should be path to plugin directory
 * and PluginInstallerStruct which contain
 * StatusCallack, progressCallback and private data for callbacks
 */
DECLARE_GENERIC_EVENT_2(InstallPluginEvent, std::string, PluginInstallerStruct)

/**
 * @brief Event for inicietig widget uninstallation.
 *
 * tizen id is used to point witch widget shuld be uninstalled
 */
DECLARE_GENERIC_EVENT_2(UninstallWidgetEvent,
                        std::string,
                        WidgetUninstallationStruct)

/**
 * @brief Event for pushing installation process forward.
 */
DECLARE_GENERIC_EVENT_1(NextStepEvent, Jobs::Job *)

DECLARE_GENERIC_EVENT_0(InitializeEvent)
DECLARE_GENERIC_EVENT_0(TerminateEvent)
} // namespace InstallerEvents

namespace Logic {
/**
 * @brief Controls Widget installation
 *
 * Main Controler of wiget installation/uninstallation, this is also used
 * for pushing forward each of processes.
 * It waits for three events:
 * <ul>
 *     <li>InstallWidgetEvent</li>
 *     <li>UninstallWidgetEvent</li>
 *     <li>NextStepEvent</li>
 * </ul>
 */

typedef DPL::TypeListDecl<
    InstallerControllerEvents::InstallWidgetEvent,
    InstallerControllerEvents::InstallPluginEvent,
    InstallerControllerEvents::UninstallWidgetEvent,
    InstallerControllerEvents::NextStepEvent,
    InstallerControllerEvents::InitializeEvent,
    InstallerControllerEvents::TerminateEvent>::Type
InstallerControllerEventsSet;

class InstallerController : public DPL::Event::Controller<
        InstallerControllerEventsSet>
{
  protected:
    /**
     * @brief Executed on InstallWidgetEvent received.
     */
    virtual void OnEventReceived(
        const InstallerControllerEvents::InstallWidgetEvent &event);

    /**
     * @brief Executed on InstallPluginEvent received.
     */
    virtual void OnEventReceived(
        const InstallerControllerEvents::InstallPluginEvent &event);

    /**
     * @brief Executed on UninstallWidgetEvent received.
     */
    virtual void OnEventReceived(
        const InstallerControllerEvents::UninstallWidgetEvent &event);
    /**
     * @brief Executed on NextStepEvent received.
     */
    virtual void OnEventReceived(
        const InstallerControllerEvents::NextStepEvent &event);

    virtual void OnEventReceived(
        const InstallerControllerEvents::InitializeEvent &event);
    virtual void OnEventReceived(
        const InstallerControllerEvents::TerminateEvent &event);

  private:
    // Embedded logic
    Logic::InstallerLogic m_installerLogic;

    InstallerController();

    static Eina_Bool AddNextStep(void *data);

    friend class DPL::Singleton<InstallerController>;
};

typedef DPL::Singleton<InstallerController> InstallerControllerSingleton;
}

#endif // INSTALLER_CONTROLLER_H
