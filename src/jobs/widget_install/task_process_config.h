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
 * @file    task_process_config.h
 * @author  Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task widget config
 */
#ifndef INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_PROCESS_CONFIG_H
#define INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_PROCESS_CONFIG_H

#include <set>
#include <list>

#include <dpl/task.h>
#include <dpl/string.h>
#include <dpl/optional_typedefs.h>
#include <dpl/wrt-dao-ro/config_parser_data.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/wrt-dao-ro/global_config.h>

#include <wrt_common_types.h>

class InstallerContext;

namespace Jobs {
namespace WidgetInstall {

class TaskProcessConfig :
    public DPL::TaskDecl<TaskProcessConfig>
{
  private:
    class Exception
    {
      public:
        DECLARE_EXCEPTION_TYPE(DPL::Exception, Base)
        DECLARE_EXCEPTION_TYPE(Base, ConfigParseFailed)
    };

    typedef std::list<std::pair<DPL::String, DPL::String> > StringPairList;

    InstallerContext& m_installContext;
    WrtDB::LocaleSet m_localeFolders;
    std::set<DPL::String> m_processedIconSet;
    std::set<DPL::String> m_processedSmallIconSet;

    void StepFillWidgetConfig();
    void ReadLocaleFolders();
    void ProcessLocalizedStartFiles();
    void ProcessStartFile(
        const DPL::OptionalString& path,
        const DPL::OptionalString& type,
        const DPL::OptionalString& encoding =
            DPL::OptionalString(),
        bool typeForcedInConfig = false);
    void ProcessBackgroundPageFile();
    void ProcessLocalizedIcons();
    void ProcessIcon(const WrtDB::ConfigParserData::Icon& icon);
    void ProcessWidgetInstalledPath();
    void ProcessAppControlInfo();
    void ProcessSecurityModel();
    void StepVerifyFeatures();
    void StepVerifyLivebox();
    void StepCheckMinVersionInfo();

    template <typename Ex, const char* Msg>
    void StepCancelInstallation();

    void StartStep();
    void EndStep();

    DPL::String createAuthorWidgetInfo() const;
    bool isFeatureAllowed(
        WrtDB::AppType appType, DPL::String featureName);
    bool isMinVersionCompatible(
        WrtDB::AppType appType,
        const DPL::OptionalString &widgetVersion) const;
    /**
     * @brief Parses version string in format "major.minor.micro anything"
     * Returns false if format is invalid
     */
    bool isTizenWebApp() const;
    bool parseVersionString(const std::string &version, long &majorVersion,
                            long &minorVersion, long &microVersion) const;

    bool fillWidgetConfig(WrtDB::WidgetRegisterInfo& pWidgetConfigInfo,
                          WrtDB::ConfigParserData& configInfo);

  public:
    TaskProcessConfig(InstallerContext& installTaskContext);
};
} //namespace WidgetInstall
} //namespace Jobs

#endif // INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_PROCESS_CONFIG_H
