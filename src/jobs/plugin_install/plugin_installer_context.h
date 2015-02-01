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
 * @file    plugin_installer_structs.h
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @version
 * @brief   Definition file of plugin installer tasks data structures
 */
#ifndef WRT_SRC_INSTALLERCORE_PLUGININSTALLERTASKS_PLUGININSTALLERCONTEXT_H_
#define WRT_SRC_INSTALLERCORE_PLUGININSTALLERTASKS_PLUGININSTALLERCONTEXT_H_

#include <string>
#include <dpl/wrt-dao-ro/feature_dao_read_only.h>
#include <plugin_path.h>
//#include <plugin_model.h>

using namespace WrtDB;

namespace Jobs {
namespace PluginInstall {
class JobPluginInstall;
}
}

struct PluginInstallerContext
{
    enum PluginInstallStep
    {
        START,
        PLUGIN_PATH,
        CONFIG_FILE,
        PLUGIN_EXISTS_CHECK,
        LOADING_LIBRARY,
        REGISTER_PLUGIN,
        REGISTER_FEATURES,
        REGISTER_OBJECTS,
        RESOLVE_DEPENDENCIES,
        PLUGIN_INSTALL_END
    };

    PluginPath pluginFilePath;           ///< plugin directory
    PluginPath metaFilePath;
    bool m_dataFromConfigXML;
    WrtDB::DbPluginHandle pluginHandle;
    // if this value is true the plugin model may be created
    // if not plugin installation has failed from some reason
    bool installationCompleted;

    //used to set installation progress
    Jobs::PluginInstall::JobPluginInstall* installerTask;
};
#endif // WRT_SRC_INSTALLERCORE_PLUGININSTALLERTASKS_PLUGININSTALLERCONTEXT_H_
