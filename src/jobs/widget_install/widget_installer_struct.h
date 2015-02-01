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
 * @file    widget_installer_struct.h
 * @author  Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
 * @author  Grzegorz Krawczyk (g.krawczyk@samsung.com)
 * @version 1.0
 * @brief   Implementation file for widget installer struct
 */
#ifndef WRT_SRC_INSTALLER_CORE_INSTALLER_TASKS_WIDGET_INSTALLER_STRUCT_H_
#define WRT_SRC_INSTALLER_CORE_INSTALLER_TASKS_WIDGET_INSTALLER_STRUCT_H_

//SYSTEM INCLUDES
#include <dpl/assert.h>

//WRT INCLUDES
#include <job_base.h>
#include <job.h>
#include <widget_install/widget_install_errors.h>
#include <wrt_install_mode.h>
#include <wrt_common_types.h>
#include <pkgmgr_signal_interface.h>
#include <memory>
#include <string>

//Widget Installer typedefs
typedef void (*InstallerFinishedCallback)(
    void *userParam,
    std::string tizenId,
    Jobs::Exceptions::Type);

typedef void (*InstallerProgressCallback)(void *userParam,
                                          ProgressPercent percent,
                                          const ProgressDescription &);

namespace Jobs {
namespace WidgetInstall {
//InstallationStruct
typedef Jobs::JobCallbacksBase<InstallerFinishedCallback,
                               InstallerProgressCallback>
WidgetInstallCallbackBase;

//Widget Installation Struct
struct WidgetInstallationStruct : public WidgetInstallCallbackBase
{
    InstallMode m_installMode;
    std::shared_ptr<PackageManager::IPkgmgrSignal> pkgmgrInterface;

    // It must be empty-constructible as a parameter of generic event
    WidgetInstallationStruct() {};
    WidgetInstallationStruct(
        InstallerFinishedCallback finished,
        InstallerProgressCallback progress,
        void *param,
        InstallMode mode,
        std::shared_ptr<PackageManager::IPkgmgrSignal>
        _pkgmgrInterface
        ) :
        WidgetInstallCallbackBase(finished, progress, param),
        m_installMode(mode),
        pkgmgrInterface(_pkgmgrInterface)
    {}
};
} // namespace WidgetInstall
} // namespace Jobs

#endif // WRT_SRC_INSTALLER_CORE_INSTALLER_TASKS_WIDGET_INSTALLER_STRUCT_H_
