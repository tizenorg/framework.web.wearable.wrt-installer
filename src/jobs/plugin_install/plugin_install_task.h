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
 * @file    install.h
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @author  Grzegorz Krawczyk (g.krawczyk@samgsung.com)
 * @version
 * @brief
 */

#ifndef INSTALL_H_
#define INSTALL_H_

//WRT INCLUDES
#include <dpl/task.h>
#include "plugin_installer_context.h"
#include "plugin_objects.h"

namespace Jobs {
namespace PluginInstall {
class PluginInstallTask :
    public DPL::TaskDecl<PluginInstallTask>
{
  public:
    PluginInstallTask(PluginInstallerContext *inCont);
    virtual ~PluginInstallTask();

  private:
    //data
    PluginInstallerContext *m_context;

    //PluginMetafile
    WrtDB::PluginMetafileData m_pluginInfo;

    //Plugin LibraryObjects
    PluginObjectsPtr m_libraryObjects;

    WrtDB::DbPluginHandle m_pluginHandle;

    bool m_dataFromConfigXML;

    //steps
    void stepCheckPluginPath();
    void stepFindPluginLibrary();
    void stepParseConfigFile();
    void stepCheckIfAlreadyInstalled();
    void stepLoadPluginLibrary();
    void stepRegisterPlugin();
    void stepRegisterFeatures();
    void stepRegisterPluginObjects();
    void stepResolvePluginDependencies();
};
} //namespace Jobs
} //namespace PluginInstall

#endif /* INSTALL_H_ */
