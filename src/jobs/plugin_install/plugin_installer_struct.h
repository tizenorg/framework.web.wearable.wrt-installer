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
 * @file    plugin_installer_struct.h
 * @author  Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
 * @author  Grzegorz Krawczyk (g.krawczyk@samsung.com)
 * @version 1.0
 * @brief   Implementation file for widget installer struct
 */
#ifndef WRT_SRC_INSTALLER_CORE_PLUGIN_INSTALLER_PLUGIN_INSTALLER_STRUCT_H_
#define WRT_SRC_INSTALLER_CORE_PLUGIN_INSTALLER_PLUGIN_INSTALLER_STRUCT_H_

#include <job_base.h>
#include <plugin_install/plugin_installer_errors.h>

//Plugin Installer typedefs
typedef void (*PluginInstallerFinishedCallback)(
    void *userParam,
    Jobs::Exceptions::Type);

//installer progress
typedef void (*PluginInstallerProgressCallback)(
    void *userParam,
    ProgressPercent percent,
    const ProgressDescription &description);

//Plugin Installetion Struct
typedef Jobs::JobCallbacksBase<PluginInstallerFinishedCallback,
                               PluginInstallerProgressCallback>
PluginInstallerStruct;

#endif // WRT_SRC_INSTALLER_CORE_PLUGIN_INSTALLER_PLUGIN_INSTALLER_STRUCT_H_
