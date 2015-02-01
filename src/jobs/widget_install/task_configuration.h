/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file    task_configuration.h
 * @version 1.0
 * @author  Tomasz Iwanek
 * @brief   header file for configuration task
 */
#ifndef TASK_CONFIGURATION_H
#define TASK_CONFIGURATION_H

#include <string>

#include <dpl/task.h>
#include <dpl/wrt-dao-ro/config_parser_data.h>

#include <widget_install/widget_update_info.h>
#include <widget_install/widget_install_context.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>

#include <pkg-manager/pkgmgr_signal.h>

class InstallerContext;

namespace Jobs {
namespace WidgetInstall {

class TaskConfiguration : public DPL::TaskDecl<TaskConfiguration>
{
    InstallerContext& m_context;
    std::string m_tempDir;
    WrtDB::ConfigParserData &m_widgetConfig;
    std::string m_configuration;

    WidgetUpdateInfo m_widgetUpdateInfo;
    WrtDB::AppType m_detectedType;

    void parseWidgetXMLConfig(
        const std::string &widgetSource,
        const std::string &tempPath,
        WrtDB::PackagingType pkgType,
        bool isReinstall);
    WidgetUpdateInfo detectWidgetUpdate(
        const WrtDB::ConfigParserData &configInfo,
        const WrtDB::TizenPkgId &PkgId);

    bool validateTizenApplicationID(const WrtDB::TizenAppId &tizenAppId);
    bool validateTizenPackageID(const WrtDB::TizenPkgId &tizenPkgId);
    bool checkSupportRDSUpdateIfReinstall(const WrtDB::ConfigParserData &configInfo);
    bool getDefaultExternalStorage();
    bool getMMCStatus();

    std::shared_ptr<PackageManager::IPkgmgrSignal> pkgMgrInterface();

    //steps
    void StartStep();

    void SetupTempDirStep();
    void UnzipConfigurationStep();
    void ParseXMLConfigStep();

    void TizenIdStep();
    void DetectUpdateInstallationStep();
    void PkgmgrStartStep();

    void ApplicationTypeStep();
    void ResourceEncryptionStep();

    void InstallationFSLocationStep();

    void ConfigureWidgetLocationStep();
    void CheckRDSSupportStep();

    void AppendTasklistStep();
    void EndStep();

public:
    TaskConfiguration(InstallerContext& context);
};

}
}

#endif // TASK_CONFIGURATION_H
