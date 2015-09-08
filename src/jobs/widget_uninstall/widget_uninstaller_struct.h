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
 * @file    widget_uninstaller_struct.h
 * @author  Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
 * @version 1.0
 * @brief   Implementation file for widget installer struct
 */
#ifndef WRT_SRC_INSTALLER_CORE_UNINSTALLER_TASKS_WIDGET_INSTALLER_STRUCT_H_
#define WRT_SRC_INSTALLER_CORE_UNINSTALLER_TASKS_WIDGET_INSTALLER_STRUCT_H_

//SYSTEM INCLUDES
#include <dpl/assert.h>

//WRT INCLUDES
#include <job_base.h>
#include <wrt_common_types.h>
#include <widget_uninstall/widget_uninstall_errors.h>
#include <pkgmgr_signal_interface.h>
#include <memory>

//Widget Uninstaller typedefs
typedef void (*UninstallerFinishedCallback)(
    void *userParam,
    std::string tizenId,
    Jobs::Exceptions::Type);

typedef void (*UninstallerProgressCallback)(
    void *userParam,
    ProgressPercent percent,
    const ProgressDescription &description);

//UninstallationStruct
typedef Jobs::JobCallbacksBase<UninstallerFinishedCallback,
                               UninstallerProgressCallback>
WidgetUninstallCallbackBase;

struct WidgetUninstallationStruct : public WidgetUninstallCallbackBase
{
    std::shared_ptr<PackageManager::IPkgmgrSignal> pkgmgrInterface;

    // It must be empty-constructible as a parameter of generic event
    WidgetUninstallationStruct()
    {}

    WidgetUninstallationStruct(
        UninstallerFinishedCallback finished,
        UninstallerProgressCallback progress,
        void *param,
        std::shared_ptr<PackageManager::IPkgmgrSignal>
        _pkgmgrInterface
        ) :
        WidgetUninstallCallbackBase(finished, progress, param),
        pkgmgrInterface(_pkgmgrInterface)
    {}
};
#endif // WRT_SRC_INSTALLER_CORE_UNINSTALLER_TASKS_WIDGET_INSTALLER_STRUCT_H_
