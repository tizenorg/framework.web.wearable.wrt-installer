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
 * @file    task_manifest_file.cpp
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @version
 * @brief
 */

//SYSTEM INCLUDES
#include <unistd.h>
#include <string>
#include <dpl/assert.h>
#include <dirent.h>
#include <fstream>

//WRT INCLUDES
#include <widget_install/task_manifest_file.h>
#include <widget_install/job_widget_install.h>
#include <widget_install/widget_install_errors.h>
#include <widget_install/widget_install_context.h>
#if USE(WEB_PROVIDER)
#include <web_provider_widget_info.h>
#include <web_provider_plugin_info.h>
#endif
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/wrt-dao-ro/config_parser_data.h>
#include <dpl/file_input.h>
#include <dpl/errno_string.h>
#include <dpl/file_output.h>
#include <dpl/copy.h>
#include <dpl/exception.h>
#include <dpl/foreach.h>
#include <dpl/sstream.h>
#include <dpl/string.h>
#include <dpl/platform.h>
#include <dpl/utils/wrt_utility.h>
#include <map>
#include <libxml_utils.h>
#include <pkgmgr/pkgmgr_parser.h>
#include <dpl/localization/LanguageTagsProvider.h>
#include <dpl/utils/path.h>

#include <installer_log.h>

#define DEFAULT_ICON_NAME   "icon.png"
#define DEFAULT_PREVIEW_NAME   "preview.png"

using namespace WrtDB;

namespace {
typedef std::map<DPL::String, DPL::String> LanguageTagMap;

const char* const STR_TRUE = "true";
const char* const STR_FALSE = "false";
const char* const STR_NODISPLAY = "nodisplay";
const char* const STR_CATEGORY_WEARABLE_CLOCK = "http://tizen.org/category/wearable_clock";
const char* const STR_CATEGORY_WATCH_CLOCK = "com.samsung.wmanager.WATCH_CLOCK";
const char* const STR_CATEGORY_WATCH_APP = "com.samsung.wmanager.WATCH_APP";

#ifdef IME_ENABLED
const char* const STR_CATEGORY_IME = "http://tizen.org/category/ime";
#endif

const char* const STR_CATEGORY_FONT = "http://tizen.org/category/downloadable_font";

const char* const STR_CATEGORY_TTS = "http://tizen.org/category/tts";

#ifdef SERVICE_ENABLED
const char* const STR_CATEGORY_SERVICE = "http://tizen.org/category/service";
#endif

LanguageTagMap getLanguageTagMap()
{
    LanguageTagMap map;

#define ADD(tag, l_tag) map.insert(std::make_pair(L###tag, L###l_tag));
#include "languages.def"
#undef ADD

    return map;
}

DPL::OptionalString getLangTag(const DPL::String& tag)
{
    static LanguageTagMap TagsMap =
        getLanguageTagMap();

    DPL::String langTag = tag;

    _D("Trying to map language tag: %ls", langTag.c_str());
    size_t pos = langTag.find_first_of(L'_');
    if (pos != DPL::String::npos) {
        langTag.erase(pos);
    }
    DPL::OptionalString ret;

    LanguageTagMap::iterator it = TagsMap.find(langTag);
    if (it != TagsMap.end()) {
        ret = it->second;
        _D("Mapping IANA Language tag to language tag: %ls -> %ls", langTag.c_str(), (*ret).c_str());
    }

    return ret;
}
}

namespace Jobs {
namespace WidgetInstall {
const char * TaskManifestFile::encoding = "UTF-8";

TaskManifestFile::TaskManifestFile(InstallerContext &inCont) :
    DPL::TaskDecl<TaskManifestFile>(this),
    m_context(inCont),
    writer(NULL)
{
    if (InstallMode::Command::RECOVERY == m_context.mode.command) {
        AddStep(&TaskManifestFile::stepGenerateManifest);
    } else {
        AddStep(&TaskManifestFile::stepCopyIconFiles);
#if USE(WEB_PROVIDER)
        AddStep(&TaskManifestFile::stepCopyLiveboxFiles);
#endif
#ifdef SERVICE_ENABLED
        AddStep(&TaskManifestFile::stepCopyServiceIconFiles);
#endif
        AddStep(&TaskManifestFile::stepCopyAccountIconFiles);
        AddStep(&TaskManifestFile::stepCreateExecFile);
        AddStep(&TaskManifestFile::stepCreateLinkNPPluginsFile);
        AddStep(&TaskManifestFile::stepGenerateManifest);
    }
}

TaskManifestFile::~TaskManifestFile()
{}

void TaskManifestFile::stepCreateExecFile()
{
    std::string exec = m_context.locations->getExecFile();
    std::string clientExeStr = GlobalConfig::GetWrtClientExec();

#ifdef MULTIPROCESS_SERVICE_SUPPORT
    //default widget
    std::stringstream postfix;
    postfix << AppControlPrefix::PROCESS_PREFIX << 0;
    std::string controlExec = exec;
    controlExec.append(postfix.str());

    errno = 0;
    if (symlink(clientExeStr.c_str(), controlExec.c_str()) != 0)
    {
        int error = errno;
        if (error)
            _E("Failed to make a symbolic name for a file [%s]", DPL::GetErrnoString(error).c_str());
    }

    // app-control widgets
    unsigned int indexMax = 0;
    FOREACH(it, m_context.widgetConfig.configInfo.appControlList) {
        if (it->m_index > indexMax) {
            indexMax = it->m_index;
        }
    }

    for (std::size_t i = 1; i <= indexMax; ++i) {
        std::stringstream postfix;
        postfix << AppControlPrefix::PROCESS_PREFIX << i;
        std::string controlExec = exec;
        controlExec.append(postfix.str());
        errno = 0;
        if (symlink(clientExeStr.c_str(), controlExec.c_str()) != 0) {
            int error = errno;
            if (error) {
                _E("Failed to make a symbolic name for a file [%s]", DPL::GetErrnoString(error).c_str());
            }
        }
    }
#else
    //default widget
    _D("link -s %s %s", clientExeStr.c_str(), exec.c_str());
    errno = 0;
    if (symlink(clientExeStr.c_str(), exec.c_str()) != 0)
    {
        int error = errno;
        if (error)
            _E("Failed to make a symbolic name for a file [%s]", DPL::GetErrnoString(error).c_str());
    }
#ifdef SERVICE_ENABLED
    std::string serviceExeStr = GlobalConfig::GetWrtServiceExec();

    FOREACH(it, m_context.widgetConfig.configInfo.serviceAppInfoList) {
        std::string serviceExec = m_context.locations->getBinaryDir() + "/" + DPL::ToUTF8String(it->serviceId);
        errno = 0;
        _D("link -s %s %s", serviceExeStr.c_str(), serviceExec.c_str());
        if (symlink(serviceExeStr.c_str(), serviceExec.c_str()) != 0)
        {
            int error = errno;
            if (error)
                _E("Failed to make a symbolic name for a file [%s]", DPL::GetErrnoString(error).c_str());
        }
    }
#endif
#endif
    // creation of box symlink
    ConfigParserData::LiveboxList& liveboxList =
        m_context.widgetConfig.configInfo.m_livebox;
    if (!liveboxList.empty()) {
        std::string boxExec = "/usr/bin/WebProcess";
        std::string boxSymlink = m_context.locations->getExecFile();
        boxSymlink += ".d-box";

        errno = 0;
        if (symlink(boxExec.c_str(), boxSymlink.c_str()) != 0) {
            int error = errno;
            if (error) {
                _E("Failed to make a symbolic name for a file [%s]", DPL::GetErrnoString(error).c_str());
            }
        }
    }

    m_context.job->UpdateProgress(
            InstallerContext::INSTALL_CREATE_EXECFILE,
            "Widget execfile creation Finished");
}

void TaskManifestFile::stepCreateLinkNPPluginsFile()
{
    _D("stepCreateLinkNPPluginsFile");
    if (0 == access(m_context.locations->getNPPluginsDir().c_str(), F_OK)) {
        _D("This webapp has NPPlugins");
        std::string pluginsExec = "/usr/bin/PluginProcess";
        errno = 0;
        if (symlink(pluginsExec.c_str(),
                    m_context.locations->getNPPluginsExecFile().c_str()) != 0) {
            int error = errno;
            if (error) {
                _E("Failed to create symbolic link for npplugins : %ls",
                        DPL::GetErrnoString(error).c_str());
            }
        }
    }
}

void TaskManifestFile::stepCopyIconFiles()
{
    _D("CopyIconFiles");

    //This function copies icon to desktop icon path. For each locale avaliable
    //which there is at least one icon in widget for, icon file is copied.
    //Coping prioritize last positions when coping. If there is several icons
    //with given locale, the one, that will be copied, will be icon
    //which is declared by <icon> tag later than the others in config.xml of
    // widget

    std::vector<Locale> generatedLocales;

    WrtDB::WidgetRegisterInfo::LocalizedIconList & icons =
        m_context.widgetConfig.localizationData.icons;

    for (WrtDB::WidgetRegisterInfo::LocalizedIconList::const_iterator
         icon = icons.begin();
         icon != icons.end();
         ++icon)
    {
        DPL::String src = icon->src;
        FOREACH(locale, icon->availableLocales)
        {
            DPL::String tmp = (icon->isSmall ? L"small_" : L"") + (*locale);
            _D("Icon for locale: %ls is: %ls", tmp.c_str(), src.c_str());

            if (std::find(generatedLocales.begin(), generatedLocales.end(),
                    tmp) != generatedLocales.end())
            {
                _D("Skipping - has that locale");
                continue;
            } else {
                generatedLocales.push_back(tmp);
            }

            DPL::Utils::Path sourceFile(m_context.locations->getSourceDir());
            if (!locale->empty()) {
                sourceFile /= "locales";
                sourceFile /= *locale;
            }
            sourceFile /= src;

            DPL::Utils::Path
                targetFile(m_context.locations->getSharedResourceDir());
            targetFile /= (icon->isSmall ? L"small_" : L"")
                        + getIconTargetFilename(*locale, sourceFile.Extension());

            if (m_context.widgetConfig.packagingType ==
                WrtDB::PKG_TYPE_HOSTED_WEB_APP)
            {
                m_context.locations->setIconTargetFilenameForLocale(
                    targetFile.Fullpath());
            }

            _D("Copying icon: %s -> %s", sourceFile.Filename().c_str(), targetFile.Filename().c_str());

            icon_list.push_back(targetFile.Fullpath());

            Try
            {
                DPL::FileInput input(sourceFile.Fullpath());
                DPL::FileOutput output(targetFile.Fullpath());
                DPL::Copy(&input, &output);
            }

            Catch(DPL::FileInput::Exception::Base)
            {
                // Error while opening or closing source file
                //ReThrowMsg(InstallerException::CopyIconFailed,
                // sourceFile.str());
                _E("Copying widget's icon failed. Widget's icon will not be" \
                    "available from Main Screen");
            }

            Catch(DPL::FileOutput::Exception::Base)
            {
                // Error while opening or closing target file
                //ReThrowMsg(InstallerException::CopyIconFailed,
                // targetFile.str());
                _E("Copying widget's icon failed. Widget's icon will not be" \
                    "available from Main Screen");
            }

            Catch(DPL::CopyFailed)
            {
                // Error while copying
                //ReThrowMsg(InstallerException::CopyIconFailed,
                // targetFile.str());
                _E("Copying widget's icon failed. Widget's icon will not be" \
                    "available from Main Screen");
            }
        }
    }

    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_COPY_ICONFILE,
        "Widget iconfile copy Finished");
}

#ifdef SERVICE_ENABLED
void TaskManifestFile::stepCopyServiceIconFiles()
{
    _D("Copy Service icon files");

    WrtDB::ConfigParserData::ServiceAppInfoList service = m_context.widgetConfig.configInfo.serviceAppInfoList;

    if (service.size() <= 0) {
        return;
    }

    FOREACH(it, service)
    {
        if (it->m_iconsList.empty()) {
            _D("Widget doesn't contain Service icon");
            return;
        }

        ConfigParserData::IconsList iconsList = it->m_iconsList;
        FOREACH(iconIt, iconsList) {
            std::string sourceFile = m_context.locations->getSourceDir() +
                                     '/' +
                                     DPL::ToUTF8String(iconIt->src);
            std::string targetFile = m_context.locations->getSharedResourceDir() +
                                     '/' +
                                     DPL::ToUTF8String(it->serviceId) +
                                     ".png";
            copyFile(sourceFile, targetFile);
        }
    }
}
#endif

#if USE(WEB_PROVIDER)
void TaskManifestFile::stepCopyLiveboxFiles()
{
    _D("Copy Livebox Files");

    using namespace WrtDB;
    ConfigParserData &data = m_context.widgetConfig.configInfo;
    ConfigParserData::LiveboxList liveBoxList = data.m_livebox;

    if (liveBoxList.size() <= 0) {
        return;
    }

    std::ostringstream sourceFile;
    std::ostringstream targetFile;

    FOREACH (boxIt, liveBoxList) {
        ConfigParserData::LiveboxInfo::BoxSizeList boxSizeList =
            (**boxIt).m_boxInfo.m_boxSize;
        FOREACH (sizeIt, boxSizeList) {
            std::string preview = DPL::ToUTF8String((*sizeIt).m_preview);
            if (preview.empty()) {
                continue;
            }
            sourceFile << m_context.locations->getSourceDir() << "/";
            sourceFile << preview;
            targetFile << m_context.locations->getSharedDataDir() << "/";
            targetFile << (**boxIt).m_liveboxId << ".";
            targetFile << DPL::ToUTF8String((*sizeIt).m_size) << "." << DEFAULT_PREVIEW_NAME;

            copyFile(sourceFile.str(), targetFile.str());

            // clear stream objects
            sourceFile.str("");
            targetFile.str("");
        }
        // check this livebox has icon element
        std::string icon = DPL::ToUTF8String((**boxIt).m_icon);
        if (icon.empty()) {
            continue;
        }
        sourceFile << m_context.locations->getSourceDir() << "/";
        sourceFile << icon;
        targetFile << m_context.locations->getSharedDataDir() << "/";
        targetFile << (**boxIt).m_liveboxId << "." << DEFAULT_ICON_NAME;

        copyFile(sourceFile.str(), targetFile.str());

        // clear stream objects
        sourceFile.str("");
        targetFile.str("");
    }
    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_COPY_LIVEBOX_FILES,
        "Livebox files copy Finished");
}
#endif

void TaskManifestFile::stepCopyAccountIconFiles()
{
    _D("Copy Account icon files");
    WrtDB::ConfigParserData::AccountProvider account =
        m_context.widgetConfig.configInfo.accountProvider;

    if (account.m_iconSet.empty()) {
        _D("Widget doesn't contain Account");
        return;
    }

    FOREACH(it, account.m_iconSet) {
        std::string sourceFile = m_context.locations->getSourceDir() +
                                 '/' +
                                 DPL::ToUTF8String(it->second);
        std::string targetFile = m_context.locations->getSharedResourceDir() +
                                 '/' +
                                 DPL::ToUTF8String(it->second);
        copyFile(sourceFile, targetFile);
    }
}

void TaskManifestFile::copyFile(const std::string& sourceFile,
                                const std::string& targetFile)
{
    Try
    {
        DPL::FileInput input(sourceFile);
        DPL::FileOutput output(targetFile);
        DPL::Copy(&input, &output);
    }
    Catch(DPL::Exception)
    {
        _E("Failed to file copy. %s to %s", sourceFile.c_str(), targetFile.c_str());
        ReThrowMsg(Exceptions::CopyIconFailed, "Error during file copy.");
    }
}

#if USE(WEB_PROVIDER)
bool TaskManifestFile::addBoxUiApplication(Manifest& manifest)
{
    UiApplication uiApp;
    std::string postfix = ".d-box";
    static bool isAdded = false;

    Try
    {
        if (isAdded) {
            _D("UiApplication for d-box is already added");
            return false;
        }
        uiApp.setNodisplay(true);
        uiApp.setTaskmanage(false);
        uiApp.setMultiple(false);
        setWidgetName(manifest, uiApp);
        setWidgetIcons(uiApp);

        // appid for box is like [webapp id].d-box
        setWidgetIds(manifest, uiApp, postfix);
        // executable path for box is like [app path]/bin/[webapp id].d-box
        setWidgetExecPath(uiApp, postfix);
        manifest.addUiApplication(uiApp);
        isAdded = true;

        return true;
    }
    Catch(DPL::Exception)
    {
        _E("Adding UiApplication on xml is failed.");
        isAdded = false;
        return false;
    }
}
#endif

DPL::String TaskManifestFile::getIconTargetFilename(
    const DPL::String& languageTag, const std::string & ext) const
{
    DPL::OStringStream filename;
    TizenAppId appid = m_context.widgetConfig.tzAppid;

    filename << DPL::ToUTF8String(appid).c_str();

    if (!languageTag.empty()) {
        DPL::OptionalString tag = getLangTag(languageTag); // translate en ->
                                                           // en_US etc
        if (!tag) {
            tag = languageTag;
        }
        DPL::String locale =
            LanguageTagsProvider::BCP47LanguageTagToLocale(*tag);

        if (locale.empty()) {
            filename << L"." << languageTag;
        } else {
            filename << L"." << locale;
        }
    }

    if(!ext.empty())
    {
        filename << L"." + DPL::FromUTF8String(ext);
    }
    return filename.str();
}

void TaskManifestFile::saveLocalizedKey(std::ofstream &file,
                                        const DPL::String& key,
                                        const DPL::String& languageTag)
{
    DPL::String locale =
        LanguageTagsProvider::BCP47LanguageTagToLocale(languageTag);

    file << key;
    if (!locale.empty()) {
        file << "[" << locale << "]";
    }
    file << "=";
}

void TaskManifestFile::stepGenerateManifest()
{
    TizenPkgId pkgid = m_context.widgetConfig.tzPkgid;
    manifest_name = pkgid + L".xml";

    // In FOTA environment, Use temporary directory created by pkgmgr.
    // Becuase /tmp can be read-only filesystem.
    if (m_context.mode.installTime == InstallMode::InstallTime::FOTA) {
        manifest_file += L"/opt/share/packages/.recovery/wgt/" + manifest_name;
    } else {
        manifest_file += L"/tmp/" + manifest_name;
    }

    //libxml - init and check
    LibxmlSingleton::Instance().init();

    writeManifest(manifest_file);

    std::ostringstream destFile;
    if (m_context.mode.rootPath == InstallMode::RootPath::RO) {
        destFile << WrtDB::GlobalConfig::GetPreloadManifestPath() << "/";
    } else {
        destFile << WrtDB::GlobalConfig::GetManifestPath() << "/";
    }

    destFile << DPL::ToUTF8String(manifest_name);
    commit_manifest = destFile.str();
    _D("Commiting manifest file : %s", commit_manifest.c_str());

    commitManifest();

    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_CREATE_MANIFEST,
        "Widget Manifest Creation Finished");
}

void TaskManifestFile::commitManifest()
{

    if (!(m_context.mode.rootPath == InstallMode::RootPath::RO &&
                (m_context.mode.installTime == InstallMode::InstallTime::PRELOAD
                 || m_context.mode.installTime == InstallMode::InstallTime::FOTA)
                && m_context.mode.extension == InstallMode::ExtensionType::DIR)) {
        _D("cp %ls %s", manifest_file.c_str(), commit_manifest.c_str());

        DPL::FileInput input(DPL::ToUTF8String(manifest_file));
        DPL::FileOutput output(commit_manifest);
        DPL::Copy(&input, &output);
        _D("Manifest writen to: %s", commit_manifest.c_str());

        //removing temp file
        unlink((DPL::ToUTF8String(manifest_file)).c_str());
        manifest_file = DPL::FromUTF8String(commit_manifest);
    }
}

void TaskManifestFile::writeManifest(const DPL::String & path)
{
    _D("Generating manifest file : %ls", path.c_str());
    Manifest manifest;
    UiApplication uiApp;

#ifdef MULTIPROCESS_SERVICE_SUPPORT
    //default widget content
    std::stringstream postfix;
    // index 0 is reserved
    postfix << AppControlPrefix::PROCESS_PREFIX << 0;
    setWidgetExecPath(uiApp, postfix.str());
    setWidgetName(manifest, uiApp);
    setWidgetIds(manifest, uiApp);
    setWidgetIcons(uiApp);
    setWidgetDescription(manifest);
    setWidgetManifest(manifest);
    setWidgetOtherInfo(uiApp);
    setAppCategory(uiApp);
    setMetadata(uiApp);
    // move to the last of this procedure
    //setLiveBoxInfo(manifest);
    setAccount(manifest);
    setPrivilege(manifest);
    manifest.addUiApplication(uiApp);

    //app-control content
    ConfigParserData::AppControlInfoList appControlList =
        m_context.widgetConfig.configInfo.appControlList;
    FOREACH(it, appControlList) {
        UiApplication uiApp;

        uiApp.setTaskmanage(true);
        uiApp.setNodisplay(true);
#ifdef MULTIPROCESS_SERVICE_SUPPORT_INLINE
        uiApp.setTaskmanage(ConfigParserData::AppControlInfo::Disposition::INLINE != it->m_disposition);
        uiApp.setMultiple(ConfigParserData::AppControlInfo::Disposition::INLINE == it->m_disposition);
#endif
        std::stringstream postfix;
        postfix << AppControlPrefix::PROCESS_PREFIX << it->m_index;
        setWidgetExecPath(uiApp, postfix.str());
        setWidgetName(manifest, uiApp);
        setWidgetIds(manifest, uiApp);
        setWidgetIcons(uiApp);
        setAppControlInfo(uiApp, *it);
        setAppCategory(uiApp);
        setMetadata(uiApp);
        manifest.addUiApplication(uiApp);
    }
    // TODO: Must fix again with right method
    // The mainapp attiribute must be set
    // when there are multiple uiapps in mainfest
#ifdef SERVICE_ENABLED
    WrtDB::ConfigParserData::ServiceAppInfoList service = m_context.widgetConfig.configInfo.serviceAppInfoList;

    if (service.size() > 0) {
        FOREACH(it, service) {
            ServiceApplication serviceApp;
            setServiceInfo(serviceApp, *it);
            manifest.addServiceApplication(serviceApp);
        }
    } else {
        _D("Widget doesn't contain service");
    }
#endif

#ifdef IME_ENABLED
    ImeApplication imeApp;
    WrtDB::ConfigParserData::ImeAppInfoList ime = m_context.widgetConfig.configInfo.imeAppInfoList;

    if (ime.size() > 0) {
        extractImeInfo(imeApp);
        manifest.addImeApplication(imeApp);
    } else {
        _D("Widget doesn't contain ime");
    }
#endif

#if USE(WEB_PROVIDER)
    setLiveBoxInfo(manifest);
#endif
#else
    //default widget content
    setWidgetExecPath(uiApp);
    setWidgetName(manifest, uiApp);
    setWidgetIds(manifest, uiApp);
    setWidgetIcons(uiApp);
    setWidgetDescription(manifest);
    setWidgetManifest(manifest);
    setWidgetOtherInfo(uiApp);
    setAppControlsInfo(uiApp);
    setAppCategory(uiApp);
    setMetadata(uiApp);
    // move to the last of this procedure
    //setLiveBoxInfo(manifest);
    setAccount(manifest);
    setPrivilege(manifest);

    manifest.addUiApplication(uiApp);
    // TODO: Must fix again with right method
    // The mainapp attiribute must be set
    // when there are multiple uiapps in mainfest

#ifdef SERVICE_ENABLED
    WrtDB::ConfigParserData::ServiceAppInfoList service = m_context.widgetConfig.configInfo.serviceAppInfoList;

    if (service.size() > 0) {
        FOREACH(it, service) {
            ServiceApplication serviceApp;
            setServiceInfo(serviceApp, *it);
            manifest.addServiceApplication(serviceApp);
        }
    } else {
        _D("Widget doesn't contain service");
    }
#endif

#ifdef IME_ENABLED
    ImeApplication imeApp;
    WrtDB::ConfigParserData::ImeAppInfoList ime = m_context.widgetConfig.configInfo.imeAppInfoList;

    if (ime.size() > 0) {
        extractImeInfo(imeApp);
        manifest.addImeApplication(imeApp);
    } else {
       _D("Widget doesn't contain ime");
    }
#endif

#if USE(WEB_PROVIDER)
    setLiveBoxInfo(manifest);
#endif
#endif

    manifest.generate(path);
    _D("Manifest file serialized");
}

#ifdef SERVICE_ENABLED
void TaskManifestFile::setServiceInfo(ServiceApplication &serviceApp, WrtDB::ConfigParserData::ServiceAppInfo & service)
{
    setWidgetExecPathService(serviceApp, service);
    setWidgetIdsService(serviceApp, service);
    setWidgetNameService(serviceApp, service);
    setWidgetIconService(serviceApp, service);
    setWidgetOtherInfoService(serviceApp);
    setWidgetComponentService(serviceApp);
    setWidgetAutoRestartService(serviceApp, service);
    setWidgetOnBootService(serviceApp, service);
    setAppCategoryService(serviceApp, service);
    setMetadataService(serviceApp, service);
}

void TaskManifestFile::setWidgetComponentService(ServiceApplication &serviceApp)
{
    serviceApp.setComponent(DPL::FromASCIIString("svcapp"));
}

void TaskManifestFile::setWidgetAutoRestartService(ServiceApplication &serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service)
{
    serviceApp.setAutoRestart(service.autoRestart);
}

void TaskManifestFile::setWidgetOnBootService(ServiceApplication &serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service)
{
    serviceApp.setOnBoot(service.onBoot);
}

void TaskManifestFile::setWidgetExecPathService(ServiceApplication &serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service)
{
    if (service.serviceId.empty()) {
        _D("Widget doesn't contain service id");
        return;
    }

    std::string serviceExec = m_context.locations->getBinaryDir() + "/" + DPL::ToUTF8String(service.serviceId);
    serviceApp.setExec(DPL::FromUTF8String(serviceExec));
}

void TaskManifestFile::setWidgetIdsService(ServiceApplication &serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service)
{
    //appid
    if (service.serviceId.empty()) {
        _D("Widget doesn't contain service id");
        return;
    }
    serviceApp.setAppid(service.serviceId);

    //extraid
    TizenAppId appid = m_context.widgetConfig.tzAppid;
    if (!!m_context.widgetConfig.guid) {
        serviceApp.setExtraid(*m_context.widgetConfig.guid);
    } else {
        if (!appid.empty()) {
            serviceApp.setExtraid(DPL::String(L"http://") + appid);
        }
    }

    //type
    serviceApp.setType(DPL::FromASCIIString("webapp"));
}

void TaskManifestFile::setWidgetNameService(ServiceApplication &serviceApp, WrtDB::ConfigParserData::ServiceAppInfo & service)
{
    if (service.m_localizedDataSet.empty()) {
        _D("Widget doesn't contain service name");
        return;
    }

    ConfigParserData::LocalizedDataSet &localizedDataSet = service.m_localizedDataSet;
    FOREACH(localizedData, localizedDataSet) {
        Locale i = localizedData->first;
        DPL::OptionalString localeTag = getLangTag(i);
        if (localeTag.IsNull()) {
            localeTag = i;
        }
        DPL::OptionalString name = localizedData->second.name;

        if (!!name) {
            if (!!localeTag) {
                DPL::String locale =
                    LanguageTagsProvider::BCP47LanguageTagToLocale(*localeTag);

                if (!locale.empty()) {
                    serviceApp.addLabel(LabelType(*name, *localeTag));
                } else {
                    serviceApp.addLabel(LabelType(*name));
                }
            } else {
                serviceApp.addLabel(LabelType(*name));
            }
        }
    }
}

void TaskManifestFile::setWidgetIconService(ServiceApplication & serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service)
{
    if (service.m_iconsList.empty()) {
        _D("Widget doesn't contain service icon");
        return;
    }

    DPL::String icon =
        DPL::FromUTF8String(m_context.locations->getSharedResourceDir()) +
        DPL::String(L"/") +
        DPL::String(service.serviceId) + DPL::String(L".png");
    serviceApp.addIcon(icon);
}

void TaskManifestFile::setWidgetOtherInfoService(ServiceApplication &serviceApp)
{
    serviceApp.setNodisplay(true);
    serviceApp.setTaskmanage(false);
    serviceApp.setMultiple(false);

    FOREACH(it, m_context.widgetConfig.configInfo.settingsList)
    {
        if (!strcmp(DPL::ToUTF8String(it->m_name).c_str(), STR_NODISPLAY)) {
            if (!strcmp(DPL::ToUTF8String(it->m_value).c_str(), STR_TRUE)) {
                serviceApp.setNodisplay(true);
                serviceApp.setTaskmanage(false);
            } else {
                serviceApp.setNodisplay(false);
                serviceApp.setTaskmanage(true);
            }
        }
    }
}

void TaskManifestFile::setAppCategoryService(ServiceApplication & serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service)
{
    if (service.m_categoryList.empty()) {
        _D("Widget doesn't contain service category");
        return;
    }

    FOREACH(it, service.m_categoryList) {
        serviceApp.addAppCategory(*it);
    }
}

void TaskManifestFile::setMetadataService(ServiceApplication & serviceApp, const WrtDB::ConfigParserData::ServiceAppInfo & service)
{
    if (service.m_metadataList.empty()) {
        _D("Widget doesn't contain service metadata");
        return;
    }

    FOREACH(it, service.m_metadataList) {
        MetadataType metadataType(it->key, it->value);
        serviceApp.addMetadata(metadataType);
    }
}
#endif

#ifdef IME_ENABLED
void TaskManifestFile::extractImeInfo(ImeApplication &imeApp)
{
    setWidgetNameIME(imeApp);
    setWidgetIdsIME(imeApp);
    setWidgetUuidIME(imeApp);
    setWidgetLanguageIME(imeApp);
    setWidgetTypeIME(imeApp);
    setWidgetOptionIME(imeApp);
}

void TaskManifestFile::setWidgetUuidIME(ImeApplication &imeApp)
{
    WrtDB::ConfigParserData::ImeAppInfoList ime = m_context.widgetConfig.configInfo.imeAppInfoList;

    FOREACH(it, ime)
    {
        imeApp.addUuid(it->uuid);
    }
}

void TaskManifestFile::setWidgetLanguageIME(ImeApplication &imeApp)
{
    WrtDB::ConfigParserData::ImeAppInfoList ime = m_context.widgetConfig.configInfo.imeAppInfoList;

    FOREACH(it, ime)
    {
        FOREACH(lang, it->languageList) {
            imeApp.addLanguage(*lang);
        }
    }
}

void TaskManifestFile::setWidgetTypeIME(ImeApplication &imeApp)
{
    imeApp.addIseType(DPL::FromASCIIString("SOFTWARE_KEYBOARD_ISE"));
}

void TaskManifestFile::setWidgetOptionIME(ImeApplication &imeApp)
{
    imeApp.addOption(DPL::FromASCIIString("STAND_ALONE"));
    imeApp.addOption(DPL::FromASCIIString("NEED_SCREEN_INFO"));
    imeApp.addOption(DPL::FromASCIIString("AUTO_RESTART"));
}

void TaskManifestFile::setWidgetIdsIME(ImeApplication & imeApp, const std::string &postfix)
{
    //appid
    TizenAppId appid = m_context.widgetConfig.tzAppid;
    if (!postfix.empty()) {
        appid = DPL::FromUTF8String(DPL::ToUTF8String(appid).append(postfix));
    }
    imeApp.setAppid(appid);
}

void TaskManifestFile::setWidgetNameIME(ImeApplication &imeApp)
{
    bool defaultNameSaved = false;

    DPL::OptionalString defaultLocale =
        m_context.widgetConfig.configInfo.defaultlocale;
    std::pair<DPL::String,
              WrtDB::ConfigParserData::LocalizedData> defaultLocalizedData;
    //labels
    FOREACH(localizedData, m_context.widgetConfig.configInfo.localizedDataSet)
    {
        Locale i = localizedData->first;
        DPL::OptionalString tag = getLangTag(i); // translate en -> en_US etc
        if (!tag) {
            tag = i;
        }
        DPL::OptionalString name = localizedData->second.name;
        generateWidgetNameIME(imeApp, tag, name, defaultNameSaved);

        //store default locale localized data
        if (!!defaultLocale && defaultLocale == i) {
            defaultLocalizedData = *localizedData;
        }
    }

    if (!!defaultLocale && !defaultNameSaved) {
        DPL::OptionalString name = defaultLocalizedData.second.name;
        generateWidgetNameIME(imeApp,
                           DPL::OptionalString(),
                           name,
                           defaultNameSaved);
    }
}

void TaskManifestFile::generateWidgetNameIME(ImeApplication &imeApp,
                                          const DPL::OptionalString& tag,
                                          DPL::OptionalString name,
                                          bool & defaultNameSaved)
{
    if (!!name) {
        if (!!tag) {
            DPL::String locale =
                LanguageTagsProvider::BCP47LanguageTagToLocale(*tag);

            if (!locale.empty()) {
                imeApp.addLabel(LabelType(*name, *tag));
            } else {
                imeApp.addLabel(LabelType(*name));
            }
        } else {
            defaultNameSaved = true;
            imeApp.addLabel(LabelType(*name));
        }
    }
}
#endif

void TaskManifestFile::setWidgetExecPath(UiApplication & uiApp,
                                         const std::string &postfix)
{
    std::string exec = m_context.locations->getExecFile();
    if (!postfix.empty()) {
        exec.append(postfix);
    }
    _D("exec = %s", exec.c_str());
    uiApp.setExec(DPL::FromASCIIString(exec));
}

void TaskManifestFile::setWidgetName(Manifest & manifest,
                                     UiApplication & uiApp)
{
    bool defaultNameSaved = false;

    DPL::OptionalString defaultLocale =
        m_context.widgetConfig.configInfo.defaultlocale;
    std::pair<DPL::String,
              WrtDB::ConfigParserData::LocalizedData> defaultLocalizedData;
    //labels
    FOREACH(localizedData, m_context.widgetConfig.configInfo.localizedDataSet)
    {
        Locale i = localizedData->first;
        DPL::OptionalString tag = getLangTag(i); // translate en -> en_US etc
        if (!tag) {
            tag = i;
        }
        DPL::OptionalString name = localizedData->second.name;
        generateWidgetName(manifest, uiApp, tag, name, defaultNameSaved);

        //store default locale localized data
        if (!!defaultLocale && defaultLocale == i) {
            defaultLocalizedData = *localizedData;
        }
    }

    if (!!defaultLocale && !defaultNameSaved) {
        DPL::OptionalString name = defaultLocalizedData.second.name;
        generateWidgetName(manifest,
                           uiApp,
                           DPL::OptionalString(),
                           name,
                           defaultNameSaved);
    }
}

void TaskManifestFile::setWidgetIds(Manifest & manifest,
                                    UiApplication & uiApp,
                                    const std::string &postfix)
{
    //appid
    TizenAppId appid = m_context.widgetConfig.tzAppid;
    if (!postfix.empty()) {
        appid = DPL::FromUTF8String(DPL::ToUTF8String(appid).append(postfix));
    }
    uiApp.setAppid(appid);

    //ambient_support
    WrtDB::ConfigParserData::CategoryList categoryList = m_context.widgetConfig.configInfo.categoryList;

    if (!categoryList.empty()) {
        FOREACH(it, categoryList) {
            if ((DPL::ToUTF8String(*it) == STR_CATEGORY_WEARABLE_CLOCK) ||
                (DPL::ToUTF8String(*it) == STR_CATEGORY_WATCH_CLOCK)) {
                if (!!m_context.widgetConfig.configInfo.ambient) {
                    uiApp.setAmbientSupport(*m_context.widgetConfig.configInfo.ambient);
                }
            }
        }
    }

    //extraid
    if (!!m_context.widgetConfig.guid) {
        uiApp.setExtraid(*m_context.widgetConfig.guid);
    } else {
        if (!appid.empty()) {
            uiApp.setExtraid(DPL::String(L"http://") + appid);
        }
    }

    //type
    uiApp.setType(DPL::FromASCIIString("webapp"));
    manifest.setType(L"wgt");
}

void TaskManifestFile::generateWidgetName(Manifest & manifest,
                                          UiApplication &uiApp,
                                          const DPL::OptionalString& tag,
                                          DPL::OptionalString name,
                                          bool & defaultNameSaved)
{
    if (!!name) {
        if (!!tag) {
            DPL::String locale =
                LanguageTagsProvider::BCP47LanguageTagToLocale(*tag);

            if (!locale.empty()) {
                uiApp.addLabel(LabelType(*name, *tag));
            } else {
                uiApp.addLabel(LabelType(*name));
                manifest.addLabel(LabelType(*name));
            }
        } else {
            defaultNameSaved = true;
            uiApp.addLabel(LabelType(*name));
            manifest.addLabel(LabelType(*name));
        }
    }
}

void TaskManifestFile::setWidgetIcons(UiApplication & uiApp)
{
    //TODO this file will need to be updated when user locale preferences
    //changes.
    bool defaultIconSaved = false;

    DPL::OptionalString defaultLocale =
        m_context.widgetConfig.configInfo.defaultlocale;

    std::vector<Locale> generatedLocales;
    WrtDB::WidgetRegisterInfo::LocalizedIconList & icons =
        m_context.widgetConfig.localizationData.icons;

    for (WrtDB::WidgetRegisterInfo::LocalizedIconList::const_iterator
         icon = icons.begin();
         icon != icons.end();
         ++icon)
    {
        FOREACH(locale, icon->availableLocales)
        {
            DPL::String tmp = (icon->isSmall ? L"small_" : L"") + (*locale);
            if (std::find(generatedLocales.begin(), generatedLocales.end(),
                          tmp) != generatedLocales.end())
            {
                _D("Skipping - has that locale - already in manifest");
                continue;
            } else {
                generatedLocales.push_back(tmp);
            }
            DPL::OptionalString tag = getLangTag(*locale); // translate en ->
                                                           // en_US etc
            if (!tag) {
                tag = *locale;
            }

            generateWidgetIcon(uiApp, tag, *locale, DPL::Utils::Path(icon->src).Extension(), icon->isSmall, defaultIconSaved);
        }
    }
    if (!!defaultLocale && !defaultIconSaved) {
        generateWidgetIcon(uiApp, DPL::OptionalString(),
                           DPL::String(),
                           std::string(),
                           false,
                           defaultIconSaved);
    }
}

void TaskManifestFile::generateWidgetIcon(UiApplication & uiApp,
                                          const DPL::OptionalString& tag,
                                          const DPL::String& language,
                                          const std::string & extension,
                                          bool isSmall,
                                          bool & defaultIconSaved)
{
    DPL::String locale;
    if (!!tag) {
        locale = LanguageTagsProvider::BCP47LanguageTagToLocale(*tag);
    } else {
        defaultIconSaved = true;
    }

    DPL::Utils::Path
        iconText(m_context.locations->getSharedResourceDir());
    iconText /= (isSmall ? L"small_" : L"") + getIconTargetFilename(language, extension);
    if (iconText.Exists()) {
        if (!locale.empty()) {
            uiApp.addIcon(IconType(DPL::FromUTF8String(iconText.Fullpath()), locale, isSmall));
        } else {
            uiApp.addIcon(IconType(DPL::FromUTF8String(iconText.Fullpath()), isSmall));
        }

        _D("Icon file : %s", iconText.Fullpath().c_str());
        m_context.job->SendProgressIconPath(iconText.Fullpath());
    }
}

void TaskManifestFile::setWidgetDescription(Manifest & manifest)
{
    FOREACH(localizedData, m_context.widgetConfig.configInfo.localizedDataSet)
    {
        Locale i = localizedData->first;
        DPL::OptionalString tag = getLangTag(i); // translate en -> en_US etc
        if (!tag) {
            tag = i;
        }
        DPL::OptionalString description = localizedData->second.description;
        generateWidgetDescription(manifest, tag, description);
    }
}

void TaskManifestFile::generateWidgetDescription(Manifest & manifest,
                                                 const DPL::OptionalString& tag,
                                                  DPL::OptionalString description)
{
    if (!!description) {
        if (!!tag) {
            DPL::String locale =
                LanguageTagsProvider::BCP47LanguageTagToLocale(*tag);
            if (!locale.empty()) {
                manifest.addDescription(DescriptionType(*description, locale));
            } else {
                manifest.addDescription(DescriptionType(*description));
            }
        } else {
            manifest.addDescription(DescriptionType(*description));
        }
    }
}

void TaskManifestFile::setWidgetManifest(Manifest & manifest)
{
    manifest.setPackage(m_context.widgetConfig.tzPkgid);

    if (!!m_context.widgetConfig.version) {
        manifest.setVersion(*m_context.widgetConfig.version);
    }
    DPL::String email = (!!m_context.widgetConfig.configInfo.authorEmail ?
                         *m_context.widgetConfig.configInfo.authorEmail : L"");
    DPL::String href = (!!m_context.widgetConfig.configInfo.authorHref ?
                        *m_context.widgetConfig.configInfo.authorHref : L"");
    DPL::String name = (!!m_context.widgetConfig.configInfo.authorName ?
                        *m_context.widgetConfig.configInfo.authorName : L"");
    manifest.addAuthor(Author(email, href, L"", name));

    if (!m_context.callerPkgId.empty()) {
        manifest.setStoreClientId(m_context.callerPkgId);
    }

    // set csc path
    if (!m_context.mode.cscPath.empty()) {
        manifest.setCscPath(DPL::FromUTF8String(m_context.mode.cscPath));
    }

    // set api-version(required_version)
    if (!!m_context.widgetConfig.configInfo.tizenMinVersionRequired) {
        manifest.setApiVersion(*m_context.widgetConfig.configInfo.tizenMinVersionRequired);
    }
}

void TaskManifestFile::setWidgetOtherInfo(UiApplication & uiApp)
{
    FOREACH(it, m_context.widgetConfig.configInfo.settingsList)
    {
        if (!strcmp(DPL::ToUTF8String(it->m_name).c_str(), STR_NODISPLAY)) {
            if (!strcmp(DPL::ToUTF8String(it->m_value).c_str(), STR_TRUE)) {
                uiApp.setNodisplay(true);
                uiApp.setTaskmanage(false);
            } else {
                uiApp.setNodisplay(false);
                uiApp.setTaskmanage(true);
            }
        }
    }
    //TODO
    //There is no "X-TIZEN-PackageType=wgt"
    //There is no X-TIZEN-PackageID in manifest "X-TIZEN-PackageID=" <<
    // DPL::ToUTF8String(*widgetID).c_str()
    //There is no Comment in pkgmgr "Comment=Widget application"
    //that were in desktop file
}

void TaskManifestFile::setAppControlsInfo(UiApplication & uiApp)
{
    WrtDB::ConfigParserData::AppControlInfoList appControlList =
        m_context.widgetConfig.configInfo.appControlList;

    if (appControlList.empty()) {
        _D("Widget doesn't contain app control");
        return;
    }

     // x-tizen-svc=http://tizen.org/appcontrol/operation/pick|NULL|image;
    FOREACH(it, appControlList) {
        setAppControlInfo(uiApp, *it);
    }
}

void TaskManifestFile::setAppControlInfo(UiApplication & uiApp,
                                         const WrtDB::ConfigParserData::AppControlInfo & service)
{
    // x-tizen-svc=http://tizen.org/appcontrol/operation/pick|NULL|image;
    AppControl appControl;
    if (!service.m_operation.empty()) {
        appControl.addOperation(service.m_operation); //TODO: encapsulation?
    }
    if (!service.m_uriList.empty()) {
        FOREACH(uri, service.m_uriList) {
            appControl.addUri(*uri);
        }
    }
    if (!service.m_mimeList.empty()) {
        FOREACH(mime, service.m_mimeList) {
            appControl.addMime(*mime);
        }
    }
    uiApp.addAppControl(appControl);
}

void TaskManifestFile::setAppCategory(UiApplication &uiApp)
{
    WrtDB::ConfigParserData::CategoryList categoryList =
        m_context.widgetConfig.configInfo.categoryList;

    bool hasPredefinedCategory = false;
    FOREACH(it, categoryList) {
        if (!(*it).empty()) {
            uiApp.addAppCategory(*it);
            if ((DPL::ToUTF8String(*it) == STR_CATEGORY_WEARABLE_CLOCK) ||
                (DPL::ToUTF8String(*it) == STR_CATEGORY_WATCH_CLOCK)) {
                // in case of idle clock,
                // nodisplay should be set to true
                uiApp.setNodisplay(true);
                uiApp.setTaskmanage(false);
                hasPredefinedCategory = true;
            }
            else if (DPL::ToUTF8String(*it) == STR_CATEGORY_WATCH_APP) {
                hasPredefinedCategory = true;
            }
            else if (DPL::ToUTF8String(*it) == STR_CATEGORY_FONT) {
                uiApp.setNodisplay(true);
                uiApp.setTaskmanage(false);
                hasPredefinedCategory = true;
            }
            else if (DPL::ToUTF8String(*it) == STR_CATEGORY_TTS) {
                uiApp.setNodisplay(true);
                uiApp.setTaskmanage(false);
                hasPredefinedCategory = true;
            }
#ifdef IME_ENABLED
            else if (DPL::ToUTF8String(*it) == STR_CATEGORY_IME) {
                uiApp.setNodisplay(true);
                uiApp.setTaskmanage(false);
                hasPredefinedCategory = true;
            }
#endif
        }
    }
#ifdef SERVICE_ENABLED
    if (m_context.widgetConfig.configInfo.serviceAppInfoList.size() > 0) {
        hasPredefinedCategory = true;
    }
#endif
    // Tizen W feature
    // Add default app category (watch_app) except for watch_clock
    if(!hasPredefinedCategory) {
        // If the nodisplay attribute is true, does not insert any predefined category.
        // Some preloaded applications (like font package) shouldn't be visible
        // in app-tray and host-manager.
        // But, the nodisplay attribute can be set by "partner" or "platform" privilege only.
        if (!uiApp.isNoDisplay()) {
            uiApp.addAppCategory(DPL::FromASCIIString(STR_CATEGORY_WATCH_APP));
        }
    }
}

void TaskManifestFile::setMetadata(UiApplication &uiApp)
{
    WrtDB::ConfigParserData::MetadataList metadataList =
        m_context.widgetConfig.configInfo.metadataList;

    if (metadataList.empty()) {
        _D("Web application doesn't contain metadata");
        return;
    }
    FOREACH(it, metadataList) {
        MetadataType metadataType(it->key, it->value);
        uiApp.addMetadata(metadataType);
    }
}

#if USE(WEB_PROVIDER)
void TaskManifestFile::setLiveBoxInfo(Manifest& manifest)
{
    ConfigParserData::LiveboxList& liveboxList =
        m_context.widgetConfig.configInfo.m_livebox;

    if (liveboxList.empty()) {
        _D("no livebox");
        return;
    }

    if (!addBoxUiApplication(manifest)) {
        _D("error during adding UiApplication for d-box");
        return;
    }

    FOREACH(it, liveboxList) {
        _D("setLiveBoxInfo");
        LiveBoxInfo liveBox;
        WrtDB::ConfigParserData::OptionalLiveboxInfo ConfigInfo = *it;
        DPL::String appid = m_context.widgetConfig.tzAppid;

        if (ConfigInfo->m_liveboxId != L"") {
            liveBox.setLiveboxId(ConfigInfo->m_liveboxId);
        }

        if (ConfigInfo->m_primary != L"") {
            liveBox.setPrimary(ConfigInfo->m_primary);
        }

        if (ConfigInfo->m_autoLaunch != L"") {
            liveBox.setAutoLaunch(ConfigInfo->m_autoLaunch);
        }

        if (ConfigInfo->m_updatePeriod != L"") {
            liveBox.setUpdatePeriod(ConfigInfo->m_updatePeriod);
        }

        std::list<std::pair<DPL::String, DPL::String> > boxLabelList;
        if (!ConfigInfo->m_label.empty()) {
            FOREACH(im, ConfigInfo->m_label) {
                std::pair<DPL::String, DPL::String> boxSize;
                Locale i = (*im).first;
                // translate en -> en_US etc
                DPL::OptionalString tag = getLangTag(i);
                if (!tag) {
                    tag = i;
                }
                boxSize.first = (*tag);
                boxSize.second = (*im).second;
                boxLabelList.push_back(boxSize);
            }
            liveBox.setLabel(boxLabelList);
        }

        DPL::String defaultLocale =
            DPL::FromUTF8String(m_context.locations->getPackageInstallationDir()) +
            DPL::String(L"/res/wgt/");

        if (ConfigInfo->m_icon != L"") {
            DPL::String icon =
                DPL::FromUTF8String(m_context.locations->getSharedDataDir()) +
                DPL::String(L"/") +
                ConfigInfo->m_liveboxId + DPL::String(L".icon.png");
            liveBox.setIcon(icon);
        }

        if (ConfigInfo->m_boxInfo.m_boxSrc.empty() ||
            ConfigInfo->m_boxInfo.m_boxSize.empty())
        {
            _D("Widget doesn't contain box");
            return;
        } else {
            BoxInfoType box;
            if (!ConfigInfo->m_boxInfo.m_boxSrc.empty()) {
                if ((0 == ConfigInfo->m_boxInfo.m_boxSrc.compare(0, 4, L"http"))
                    || (0 ==
                        ConfigInfo->m_boxInfo.m_boxSrc.compare(0, 5, L"https")))
                {
                    box.boxSrc = ConfigInfo->m_boxInfo.m_boxSrc;
                } else {
                    box.boxSrc = defaultLocale + ConfigInfo->m_boxInfo.m_boxSrc;
                }
            }

            if (ConfigInfo->m_boxInfo.m_boxMouseEvent == L"true") {
                std::string boxType;
                if (ConfigInfo->m_type == L"") {
                    // in case of default livebox
                    boxType = web_provider_widget_get_default_type();
                } else {
                    boxType = DPL::ToUTF8String(ConfigInfo->m_type);
                }

                int box_scrollable =
                    web_provider_plugin_get_box_scrollable(boxType.c_str());

                if (box_scrollable) {
                    box.boxMouseEvent = L"true";
                } else {
                    box.boxMouseEvent = L"false";
                }
            } else {
                box.boxMouseEvent = L"false";
            }

            if (ConfigInfo->m_boxInfo.m_boxTouchEffect == L"true") {
                box.boxTouchEffect = L"true";
            } else {
                box.boxTouchEffect= L"false";
            }

            ConfigParserData::LiveboxInfo::BoxSizeList boxSizeList =
                ConfigInfo->m_boxInfo.m_boxSize;
            FOREACH(it, boxSizeList) {
                if (!(*it).m_preview.empty()) {
                    (*it).m_preview =
                        DPL::FromUTF8String(m_context.locations->getSharedDataDir()) +
                        DPL::String(L"/") +
                        ConfigInfo->m_liveboxId + DPL::String(L".") +
                        (*it).m_size + DPL::String(L".preview.png");
                }
                box.boxSize.push_back((*it));
            }

            if (!ConfigInfo->m_boxInfo.m_pdSrc.empty()
                && !ConfigInfo->m_boxInfo.m_pdWidth.empty()
                && !ConfigInfo->m_boxInfo.m_pdHeight.empty())
            {
                if ((0 == ConfigInfo->m_boxInfo.m_pdSrc.compare(0, 4, L"http"))
                    || (0 == ConfigInfo->m_boxInfo.m_pdSrc.compare(0, 5, L"https")))
                {
                    box.pdSrc = ConfigInfo->m_boxInfo.m_pdSrc;
                } else {
                    box.pdSrc = defaultLocale + ConfigInfo->m_boxInfo.m_pdSrc;
                }
                box.pdWidth = ConfigInfo->m_boxInfo.m_pdWidth;
                box.pdHeight = ConfigInfo->m_boxInfo.m_pdHeight;
            }
            liveBox.setBox(box);
        }
        manifest.addLivebox(liveBox);
    }
}
#endif

void TaskManifestFile::setAccount(Manifest& manifest)
{
    WrtDB::ConfigParserData::AccountProvider account =
        m_context.widgetConfig.configInfo.accountProvider;

    AccountProviderType provider;

    if (account.m_iconSet.empty()) {
        _D("Widget doesn't contain Account");
        return;
    }
    if (account.m_multiAccountSupport) {
        provider.multiAccount = L"true";
    } else {
        provider.multiAccount = L"false";
    }
    provider.appid = m_context.widgetConfig.tzAppid;

    FOREACH(it, account.m_iconSet) {
        std::pair<DPL::String, DPL::String> icon;

        if (it->first == ConfigParserData::IconSectionType::DefaultIcon) {
            icon.first = L"account";
        } else if (it->first == ConfigParserData::IconSectionType::SmallIcon) {
            icon.first = L"account-small";
        }

        // account manifest requires absolute path for icon
        // /opt/apps/[package]/shared/res/[icon_path]
        icon.second = DPL::FromUTF8String(m_context.locations->getSharedResourceDir()) +
                      DPL::String(L"/") +
                      it->second;
        provider.icon.push_back(icon);
    }

    FOREACH(it, account.m_displayNameSet) {
        provider.name.push_back(LabelType(it->second, it->first));
    }

    FOREACH(it, account.m_capabilityList) {
        provider.capability.push_back(*it);
    }

    Account accountInfo;
    accountInfo.addAccountProvider(provider);
    manifest.addAccount(accountInfo);
}

void TaskManifestFile::setPrivilege(Manifest& manifest)
{
    WrtDB::ConfigParserData::PrivilegeList privileges =
        m_context.widgetConfig.configInfo.privilegeList;

    PrivilegeType privilege;

    FOREACH(it, privileges)
    {
        privilege.addPrivilegeName(it->name);
    }

    manifest.addPrivileges(privilege);
}

void TaskManifestFile::StartStep()
{
    LOGI("--------- <TaskManifestFile> : START ----------");
}

void TaskManifestFile::EndStep()
{
    LOGI("--------- <TaskManifestFile> : END ----------");
}

} //namespace WidgetInstall
} //namespace Jobs
