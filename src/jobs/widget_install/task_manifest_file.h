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
 * @file    task_manifest_file.h
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @version
 * @brief
 */

#ifndef INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_DESKTOP_FILE_H
#define INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_DESKTOP_FILE_H

//SYSTEM INCLUDES
#include <fstream>

//WRT INCLUDES
#include <dpl/task.h>
#include <dpl/localization/localization_utils.h>
#include <dpl/optional_typedefs.h>

#include <libxml2/libxml/xmlwriter.h>

#include <libxml_utils.h>
#include <widget_install/manifest.h>
#include <dpl/localization/localization_utils.h>

#include <dpl/wrt-dao-ro/widget_dao_read_only.h>

class InstallerContext;

namespace Jobs {
namespace WidgetInstall {
class TaskManifestFile :
    public DPL::TaskDecl<TaskManifestFile>
{
  public:

    DECLARE_EXCEPTION_TYPE(DPL::Exception, Base)
    DECLARE_EXCEPTION_TYPE(Base, ManifestValidationError)
    DECLARE_EXCEPTION_TYPE(Base, ManifestParsingError)

    TaskManifestFile(InstallerContext &inCont);
    virtual ~TaskManifestFile();

  private:
    //context data
    InstallerContext &m_context;

    //TODO stepAbort
    //steps
    void stepCreateExecFile();
    void stepCopyIconFiles();
    void stepCopyLiveboxFiles();
    void stepCopyAccountIconFiles();
#ifdef SERVICE_ENABLED
    void stepCopyServiceIconFiles();
#endif
    void stepGenerateManifest();
    void stepCreateLinkNPPluginsFile();

    void stepAbortParseManifest();

    void StartStep();
    void EndStep();

    //private data
    std::list<std::string> icon_list; //TODO: this should be registered as
                                      // external files
    std::ostringstream backup_dir;
    xmlTextWriterPtr writer;
    DPL::String manifest_name;
    DPL::String manifest_file;
    std::string commit_manifest;

    //private methods

    void writeManifest(const DPL::String & path);
    void commitManifest();

    void setWidgetExecPath(UiApplication & uiApp,
                           const std::string &postfix = std::string());
    void setWidgetName(Manifest & manifest,
                       UiApplication & uiApp);
    void setWidgetIds(Manifest & manifest,
                       UiApplication & uiApp,
                       const std::string &postfix = std::string());
    void setWidgetIcons(UiApplication & uiApp);
    void setWidgetDescription(Manifest & manifest);
    void setWidgetManifest(Manifest & manifest);
    void setWidgetOtherInfo(UiApplication & uiApp);
    void setAppControlsInfo(UiApplication & uiApp);
    void setAppControlInfo(UiApplication & uiApp,
                           const WrtDB::ConfigParserData::AppControlInfo & service);
    void setAppCategory(UiApplication & uiApp);
    void setMetadata(UiApplication & uiApp);
    void setLiveBoxInfo(Manifest& manifest);
    void setAccount(Manifest& uiApp);
    void setPrivilege(Manifest& manifest);

    void generateWidgetName(Manifest & manifest,
                            UiApplication &uiApp,
                            const DPL::OptionalString& tag,
                            DPL::OptionalString name,
                            bool & defaultNameSaved);
    void generateWidgetDescription(Manifest & manifest,
                                   const DPL::OptionalString& tag,
                                   DPL::OptionalString description);
    void generateWidgetIcon(UiApplication & uiApp,
                            const DPL::OptionalString& tag,
                            const DPL::String& language, const std::string &extension,
                            bool isSmall, bool & defaultIconSaved);
    void copyFile(const std::string& sourceFile,
                  const std::string& targetFile);
    bool addBoxUiApplication(Manifest& manifest);

    //for widget update
    DPL::String getIconTargetFilename(const DPL::String& languageTag,
                                      const std::string & ext) const;

    static void saveLocalizedKey(std::ofstream &file,
                                 const DPL::String& key,
                                 const DPL::String& languageTag);

    static const char * encoding;

#ifdef IME_ENABLED
    void extractImeInfo(ImeApplication& imeApp);
    void setWidgetNameIME(ImeApplication& imeApp);
    void setWidgetIdsIME(ImeApplication& imeApp, const std::string& postfix = std::string());
    void generateWidgetNameIME(ImeApplication& imeApp, const DPL::OptionalString& tag, DPL::OptionalString name, bool& defaultNameSaved);
    void setWidgetUuidIME(ImeApplication& imeApp);
    void setWidgetLanguageIME(ImeApplication& imeApp);
    void setWidgetTypeIME(ImeApplication& imeApp);
    void setWidgetOptionIME(ImeApplication& imeApp);
#endif

#ifdef SERVICE_ENABLED
    void setServiceInfo(ServiceApplication& serviceApp, WrtDB::ConfigParserData::ServiceAppInfo & service);
    void setWidgetExecPathService(ServiceApplication & serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service);
    void setWidgetIdsService(ServiceApplication & serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service);
    void setWidgetNameService(ServiceApplication & serviceApp, WrtDB::ConfigParserData::ServiceAppInfo & service);
    void setWidgetIconService(ServiceApplication & serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service);
    void setWidgetOtherInfoService(ServiceApplication & serviceApp);
    void setWidgetComponentService(ServiceApplication & serviceApp);
    void setWidgetAutoRestartService(ServiceApplication & serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service);
    void setWidgetOnBootService(ServiceApplication & serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service);
    void setAppCategoryService(ServiceApplication & serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service);
    void setMetadataService(ServiceApplication & serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service);
#endif
};
} //namespace WidgetInstall
} //namespace Jobs

#endif /* INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_DESKTOP_FILE_H */
