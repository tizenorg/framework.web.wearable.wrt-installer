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
 * @file    task_process_config.cpp
 * @author  Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task widget config
 */

#include <string>
#include <sys/stat.h>
#include <dirent.h>

#include <dpl/errno_string.h>
#include <dpl/foreach.h>
#include <dpl/localization/w3c_file_localization.h>
#include <dpl/singleton_impl.h>
#include <dpl/utils/mime_type_utils.h>
#include <dpl/utils/wrt_global_settings.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/wrt-dao-ro/config_parser_data.h>
#include <dpl/wrt-dao-rw/feature_dao.h>
#include <dpl/platform.h>
#include <widget_install/job_widget_install.h>
#include <widget_install/task_process_config.h>
#include <widget_install/widget_install_context.h>
#include <widget_install/widget_install_errors.h>
#include <widget_parser.h>
#if USE(WEB_PROVIDER)
#include <web_provider_plugin_info.h>
#include <web_provider_widget_info.h>
#endif
#include <manifest.h>

#include <installer_log.h>

namespace { // anonymous
const DPL::String BR = DPL::FromUTF8String("<br>");
const std::string WINDGET_INSTALL_NETWORK_ACCESS = "network access";
}

namespace Jobs {
namespace WidgetInstall {

TaskProcessConfig::TaskProcessConfig(InstallerContext& installContext) :
    DPL::TaskDecl<TaskProcessConfig>(this),
    m_installContext(installContext)
{
    AddStep(&TaskProcessConfig::StartStep);
    AddStep(&TaskProcessConfig::ReadLocaleFolders);
    AddStep(&TaskProcessConfig::StepFillWidgetConfig);
    AddStep(&TaskProcessConfig::ProcessLocalizedStartFiles);
    AddStep(&TaskProcessConfig::ProcessBackgroundPageFile);
    AddStep(&TaskProcessConfig::ProcessLocalizedIcons);
    AddStep(&TaskProcessConfig::ProcessWidgetInstalledPath);
    AddStep(&TaskProcessConfig::ProcessAppControlInfo);
    AddStep(&TaskProcessConfig::ProcessSecurityModel);
    AddStep(&TaskProcessConfig::StepVerifyFeatures);
#if USE(WEB_PROVIDER)
    AddStep(&TaskProcessConfig::StepVerifyLivebox);
#endif
    AddStep(&TaskProcessConfig::StepCheckMinVersionInfo);
    AddStep(&TaskProcessConfig::EndStep);
}

void TaskProcessConfig::StepFillWidgetConfig()
{
    if (!fillWidgetConfig(m_installContext.widgetConfig,
                         m_installContext.widgetConfig.configInfo))
    {
        _E("Widget configuration is illformed");
        ThrowMsg(Exception::ConfigParseFailed, "Widget configuration is illformed");
    }
}

void TaskProcessConfig::ReadLocaleFolders()
{
    _D("Reading locale");
    //Adding default locale
    m_localeFolders.insert(L"");

    std::string localePath =
        m_installContext.locations->getSourceDir() + "/locales";

    DIR* localeDir = opendir(localePath.c_str());
    if (!localeDir) {
        _D("No /locales directory in the widget package.");
        return;
    }

    struct stat statStruct;
    struct dirent dirent;
    struct dirent *result;
    int return_code;
    errno = 0;
    for (return_code = readdir_r(localeDir, &dirent, &result);
            result != NULL && return_code == 0;
            return_code = readdir_r(localeDir, &dirent, &result))
    {
        DPL::String dirName = DPL::FromUTF8String(dirent.d_name);
        std::string absoluteDirName = localePath + "/";
        absoluteDirName += dirent.d_name;

        if (stat(absoluteDirName.c_str(), &statStruct) != 0) {
            _E("stat() failed with %s", DPL::GetErrnoString().c_str());
            continue;
        }

        if (S_ISDIR(statStruct.st_mode)) {
            //Yes, we ignore current, parent & hidden directories
            if (dirName[0] != L'.') {
                _D("Adding locale directory \"%ls\"", dirName.c_str());
                m_localeFolders.insert(dirName);
            }
        }
    }

    if (return_code != 0 || errno != 0) {
        _E("readdir_r() failed with %s", DPL::GetErrnoString().c_str());
    }

    if (-1 == closedir(localeDir)) {
        _E("Failed to close dir: %s with error: %s", localePath.c_str(), DPL::GetErrnoString().c_str());
    }

    m_installContext.job->UpdateProgress(InstallerContext::INSTALL_WIDGET_CONFIG1, "Read locale folders");
}

void TaskProcessConfig::ProcessLocalizedStartFiles()
{
    typedef DPL::String S;
    ProcessStartFile(
        m_installContext.widgetConfig.configInfo.startFile,
        m_installContext.widgetConfig.configInfo.
            startFileContentType,
        m_installContext.widgetConfig.configInfo.startFileEncoding,
        true);
    ProcessStartFile(S(L"index.htm"), S(L"text/html"));
    ProcessStartFile(S(L"index.html"), S(L"text/html"));
    ProcessStartFile(S(L"index.svg"), S(L"image/svg+xml"));
    ProcessStartFile(S(L"index.xhtml"), S(L"application/xhtml+xml"));
    ProcessStartFile(S(L"index.xht"), S(L"application/xhtml+xml"));
    // TODO: we need better check if in current locales widget is valid
    FOREACH(it, m_installContext.widgetConfig.localizationData.startFiles) {
        if (it->propertiesForLocales.size() > 0) {
            return;
        }
    }
    ThrowMsg(Exceptions::InvalidStartFile,
             "The Widget has no valid start file");
}

void TaskProcessConfig::ProcessStartFile(const DPL::OptionalString& path,
                                        const DPL::OptionalString& type,
                                        const DPL::OptionalString& encoding,
                                        bool typeForcedInConfig)
{
    using namespace WrtDB;

    if (!!path) {
        WidgetRegisterInfo::LocalizedStartFile startFileData;
        startFileData.path = *path;

        FOREACH(i, m_localeFolders) {
            DPL::String pathPrefix = *i;
            if (!pathPrefix.empty()) {
                pathPrefix = L"locales/" + pathPrefix + L"/";
            }

            DPL::String relativePath = pathPrefix + *path;
            DPL::String absolutePath = DPL::FromUTF8String(
                    m_installContext.locations->getSourceDir()) + L"/" +
                relativePath;
            _D("absolutePath : %ls", absolutePath.c_str());

            // get property data from packaged app
            if (WrtUtilFileExists(DPL::ToUTF8String(absolutePath))) {
                WidgetRegisterInfo::StartFileProperties startFileProperties;
                if (!!type) {
                    startFileProperties.type = *type;
                } else {
                    startFileProperties.type =
                        MimeTypeUtils::identifyFileMimeType(absolutePath);
                }

                //proceed only if MIME type is supported
                if (MimeTypeUtils::isMimeTypeSupportedForStartFile(
                        startFileProperties.type))
                {
                    if (!!encoding) {
                        startFileProperties.encoding = *encoding;
                    } else {
                        MimeTypeUtils::MimeAttributes attributes =
                            MimeTypeUtils::getMimeAttributes(
                                startFileProperties.type);
                        if (attributes.count(L"charset") > 0) {
                            startFileProperties.encoding =
                                attributes[L"charset"];
                        } else {
                            startFileProperties.encoding = L"UTF-8";
                        }
                    }

                    startFileData.propertiesForLocales[*i] =
                        startFileProperties;
                } else {
                    //9.1.16.5.content.8
                    //(there seems to be no similar requirement in .6,
                    //so let's throw only when mime type is
                    // provided explcitly in config.xml)
                    if (typeForcedInConfig) {
                        ThrowMsg(Exceptions::WidgetConfigFileInvalid,
                                 "Unsupported MIME type for start file.");
                    }
                }
            } else {
                // set property data for hosted start url
                // Hosted start url only support TIZEN WebApp
                if (m_installContext.widgetConfig.webAppType == APP_TYPE_TIZENWEBAPP)
                {
                    std::string startPath = DPL::ToUTF8String(
                            startFileData.path);

                    if (strstr(startPath.c_str(),
                               "http") == startPath.c_str())
                    {
                        WidgetRegisterInfo::StartFileProperties
                            startFileProperties;
                        if (!!type) {
                            startFileProperties.type = *type;
                        }
                        if (!!encoding) {
                            startFileProperties.encoding = *encoding;
                        }
                        startFileData.propertiesForLocales[*i] =
                            startFileProperties;
                    }
                }
            }
        }

        m_installContext.widgetConfig.localizationData.startFiles.push_back(
            startFileData);
    }
}

void TaskProcessConfig::ProcessBackgroundPageFile()
{
    if (!!m_installContext.widgetConfig.configInfo.backgroundPage) {
        // check whether file exists
        DPL::String backgroundPagePath = DPL::FromUTF8String(
                m_installContext.locations->getSourceDir()) + L"/" +
            *m_installContext.widgetConfig.configInfo.backgroundPage;
        //if no then cancel installation
        if (!WrtUtilFileExists(DPL::ToUTF8String(backgroundPagePath))) {
            ThrowMsg(Exceptions::WidgetConfigFileInvalid,
                     L"Given background page file not found in archive");
        }
    }
}

void TaskProcessConfig::ProcessLocalizedIcons()
{
    using namespace WrtDB;
    FOREACH(i, m_installContext.widgetConfig.configInfo.iconsList)
    {
        ProcessIcon(*i);
    }
    ProcessIcon(ConfigParserData::Icon(L"icon.svg"));
    ProcessIcon(ConfigParserData::Icon(L"icon.ico"));
    ProcessIcon(ConfigParserData::Icon(L"icon.png"));
    ProcessIcon(ConfigParserData::Icon(L"icon.gif"));
    ProcessIcon(ConfigParserData::Icon(L"icon.jpg"));
}

void TaskProcessConfig::ProcessIcon(const WrtDB::ConfigParserData::Icon& icon)
{
    _D("enter");
    bool isAnyIconValid = false;
    //In case a default filename is passed as custom filename in config.xml, we
    //need to keep a set of already processed filenames to avoid icon
    // duplication
    //in database.

    using namespace WrtDB;

    std::set<DPL::String> &checkDuplication = icon.isSmall ? m_processedSmallIconSet : m_processedIconSet;

    if (checkDuplication.count(icon.src) > 0) {
        _D("duplication %ls ", icon.src.c_str());
        return;
    }
    checkDuplication.insert(icon.src);

    LocaleSet localesAvailableForIcon;

    FOREACH(i, m_localeFolders)
    {
        DPL::String pathPrefix = *i;
        if (!pathPrefix.empty()) {
            pathPrefix = L"locales/" + pathPrefix + L"/";
        }

        DPL::String relativePath = pathPrefix + icon.src;
        DPL::String absolutePath = DPL::FromUTF8String(
                m_installContext.locations->getSourceDir()) + L"/" +
            relativePath;

        if (WrtUtilFileExists(DPL::ToUTF8String(absolutePath))) {
            DPL::String type = MimeTypeUtils::identifyFileMimeType(absolutePath);

            if (MimeTypeUtils::isMimeTypeSupportedForIcon(type)) {
                isAnyIconValid = true;
                localesAvailableForIcon.insert(*i);
                _D("Icon absolutePath: %ls, assigned locale: %ls, type: %ls",
                    absolutePath.c_str(), (*i).c_str(), type.c_str());
            }
        }
    }

    if (isAnyIconValid) {
        WidgetRegisterInfo::LocalizedIcon localizedIcon(icon,
                                                        localesAvailableForIcon);
        m_installContext.widgetConfig.localizationData.icons.push_back(
            localizedIcon);
    }
}

void TaskProcessConfig::ProcessWidgetInstalledPath()
{
    _D("ProcessWidgetInstalledPath");
    m_installContext.widgetConfig.widgetInstalledPath =
        DPL::FromUTF8String(
            m_installContext.locations->getPackageInstallationDir());
}

void TaskProcessConfig::ProcessAppControlInfo()
{
    _D("ProcessAppControlInfo");
    using namespace WrtDB;

    // In case of dispostion is inline, set the seperate execute
    int index = 1;
    // 0 index is reserved by default execute
    FOREACH(it, m_installContext.widgetConfig.configInfo.appControlList) {
        if (it->m_disposition ==
            ConfigParserData::AppControlInfo::Disposition::INLINE)
        {
            it->m_index = index++;
        } else {
            it->m_index = 0;
        }
    }
}

void TaskProcessConfig::ProcessSecurityModel()
{
    // 0104.  If the "required_version" specified in the Web Application's
    // configuration is 2.2 or higher and if the Web Application's
    // configuration is "CSP-compatible configuration", then the WRT MUST be
    // set to "CSP-based security mode". Otherwise, the WRT MUST be set to
    // "WARP-based security mode".
    // 0105.  A Web Application configuration is "CSP-compatible configuration"
    // if the configuration includes one or more of
    // <tizen:content-security-policy> /
    // <tizen:content-security-policy-report-only> /
    // <tizen:allow-navigation> elements.

    bool isSecurityModelV1 = false;
    bool isSecurityModelV2 = false;
    WrtDB::ConfigParserData &data = m_installContext.widgetConfig.configInfo;

    if (!!data.cspPolicy ||
        !!data.cspPolicyReportOnly ||
        !data.allowNavigationInfoList.empty())
    {
        data.accessInfoSet.clear();
    }

    // WARP is V1
    if (!data.accessInfoSet.empty()) {
        isSecurityModelV1 = true;
    }

    // CSP & allow-navigation is V2
    if (!!data.cspPolicy ||
        !!data.cspPolicyReportOnly ||
        !data.allowNavigationInfoList.empty())
    {
        isSecurityModelV2 = true;
    }

    if (isSecurityModelV1 && isSecurityModelV2) {
        _E("Security model is conflict");
        ThrowMsg(Exceptions::NotAllowed, "Security model is conflict");
    } else if (isSecurityModelV1) {
        data.securityModelVersion =
            WrtDB::ConfigParserData::SecurityModelVersion::SECURITY_MODEL_V1;
    } else if (isSecurityModelV2) {
        data.securityModelVersion =
            WrtDB::ConfigParserData::SecurityModelVersion::SECURITY_MODEL_V2;
    } else {
        data.securityModelVersion =
            WrtDB::ConfigParserData::SecurityModelVersion::SECURITY_MODEL_V1;
    }

    m_installContext.job->UpdateProgress(
        InstallerContext::INSTALL_WIDGET_CONFIG2,
        "Finished process security model");
}

void TaskProcessConfig::StepCheckMinVersionInfo()
{
    if (!isMinVersionCompatible(
            m_installContext.widgetConfig.webAppType.appType,
            m_installContext.widgetConfig.minVersion))
    {
        _E("Platform version lower than required -> cancelling installation");
        ThrowMsg(Exceptions::NotAllowed,
                 "Platform version does not meet requirements");
    }

    m_installContext.job->UpdateProgress(
        InstallerContext::INSTALL_WIDGET_CONFIG2,
        "Check MinVersion Finished");
}

void TaskProcessConfig::StepVerifyFeatures()
{
    using namespace WrtDB;
    ConfigParserData &data = m_installContext.widgetConfig.configInfo;
    ConfigParserData::FeaturesList list = data.featuresList;
    ConfigParserData::FeaturesList newList;

    //in case of tests, this variable is unused
    std::string featureInfo;
    FOREACH(it, list)
    {
        // check feature vender for permission
        // WAC, TIZEN WebApp cannot use other feature

        if (!isFeatureAllowed(m_installContext.widgetConfig.webAppType.appType,
                              it->name))
        {
            _D("This application type not allowed to use this feature");
            ThrowMsg(
                Exceptions::WidgetConfigFileInvalid,
                "This app type [" <<
                m_installContext.widgetConfig.webAppType.getApptypeToString()
                                  <<
                "] cannot be allowed to use [" <<
                DPL::ToUTF8String(it->name) + "] feature");
        } else {
            newList.insert(*it);
            featureInfo += DPL::ToUTF8String(it->name);
            featureInfo += DPL::ToUTF8String(BR);
        }
    }
    if (!data.accessInfoSet.empty()) {
        featureInfo += WINDGET_INSTALL_NETWORK_ACCESS;
        featureInfo += DPL::ToUTF8String(BR);
    }
    data.featuresList = newList;

    m_installContext.job->UpdateProgress(
        InstallerContext::INSTALL_WIDGET_CONFIG2,
        "Widget Config step2 Finished");
}

#if USE(WEB_PROVIDER)
void TaskProcessConfig::StepVerifyLivebox()
{
    using namespace WrtDB;
    ConfigParserData &data = m_installContext.widgetConfig.configInfo;
    ConfigParserData::LiveboxList liveBoxList = data.m_livebox;

    if (liveBoxList.size() <= 0) {
        return;
    }

    FOREACH (it, liveBoxList) {
        std::string boxType;

        size_t found = (**it).m_liveboxId.find_last_of(L".");
        if (found != std::string::npos) {
            if (0 != (**it).m_liveboxId.compare(0, found,
                        m_installContext.widgetConfig.tzAppid)) {
                _E("Invalid app-widget id (doesn't begin with application id)");
                ThrowMsg(Exceptions::WidgetConfigFileInvalid,
                        "Invalid app-widget id(doesn't begin with application id)");
            }
        }

        if ((**it).m_type.empty()) {
            boxType = web_provider_widget_get_default_type();
        } else {
            boxType = DPL::ToUTF8String((**it).m_type);
        }

        _D("livebox type: %s", boxType.c_str());

        ConfigParserData::LiveboxInfo::BoxSizeList boxSizeList =
            (**it).m_boxInfo.m_boxSize;
        char** boxSize = static_cast<char**>(
            malloc(sizeof(char*)* boxSizeList.size()));
        if (boxSize == NULL) {
            _E("malloc is failed");
            continue;
        }

        int boxSizeCnt = 0;
        FOREACH (m, boxSizeList) {
            boxSize[boxSizeCnt++] = strdup(DPL::ToUTF8String((*m).m_size).c_str());
        }

        bool chkSize = web_provider_plugin_check_supported_size(
            boxType.c_str(), boxSize, boxSizeCnt);

        for(int i = 0; i < boxSizeCnt; i++) {
            free(boxSize[i]);
        }
        free(boxSize);

        if(!chkSize) {
            _E("Invalid boxSize");
            ThrowMsg(Exceptions::WidgetConfigFileInvalid, "Invalid boxSize");
        }
    }
}
#endif

bool TaskProcessConfig::isFeatureAllowed(WrtDB::AppType appType,
                                        DPL::String featureName)
{
    using namespace WrtDB;
    _D("AppType = [%s]", WidgetType(appType).getApptypeToString().c_str());
    _D("FetureName = [%ls]", featureName.c_str());

    AppType featureType = APP_TYPE_UNKNOWN;
    std::string featureStr = DPL::ToUTF8String(featureName);
    const char* feature = featureStr.c_str();

    // check prefix of  feature name
    if (strstr(feature, PluginsPrefix::TIZENPluginsPrefix) == feature) {
        // Tizen WebApp feature
        featureType = APP_TYPE_TIZENWEBAPP;
    } else if (strstr(feature, PluginsPrefix::W3CPluginsPrefix) == feature) {
        // W3C standard feature
        // Both WAC and TIZEN WebApp are possible to use W3C plugins
        return true;
    } else {
        // unknown feature
        // unknown feature will be checked next step
        return true;
    }

    if (appType == featureType) {
        return true;
    }
    return false;
}

bool TaskProcessConfig::parseVersionString(const std::string &version,
                                          long &majorVersion,
                                          long &minorVersion,
                                          long &microVersion) const
{
    std::istringstream inputString(version);
    inputString >> majorVersion;
    if (inputString.bad() || inputString.fail()) {
        _W("Invalid minVersion format.");
        return false;
    }
    inputString.get(); // skip period
    inputString >> minorVersion;
    if (inputString.bad() || inputString.fail()) {
        _W("Invalid minVersion format");
        return false;
    }
    inputString.get(); // skip period
    inputString >> microVersion;
    if (inputString.bad() || inputString.fail()) {
        _W("Invalid microVersion format");
       microVersion = 0;
    }

    return true;
}

bool TaskProcessConfig::isMinVersionCompatible(
    WrtDB::AppType appType,
    const DPL::OptionalString &
    widgetVersion) const
{
    if (!widgetVersion || (*widgetVersion).empty()) {
        if (appType == WrtDB::AppType::APP_TYPE_TIZENWEBAPP) {
            return false;
        } else {
            _W("minVersion attribute is empty. WRT assumes platform "
               "supports this widget.");
            return true;
        }
    }

    //Parse widget version
    long majorWidget = 0, minorWidget = 0, microWidget = 0;
    if (!parseVersionString(DPL::ToUTF8String(*widgetVersion), majorWidget,
                            minorWidget, microWidget))
    {
        _W("Invalid format of widget version string.");
        return false;
    }

    //Parse supported version
    long majorSupported = 0, minorSupported = 0, microSupported = 0;
    std::string version;
    if (appType == WrtDB::AppType::APP_TYPE_TIZENWEBAPP) {
        version = TIZEN_VERSION;
    } else {
        _W("Invaild AppType");
        return false;
    }

    if (!parseVersionString(version,
                            majorSupported, minorSupported, microSupported))
    {
        _W("Invalid format of platform version string.");
        return true;
    }

    if (majorWidget > majorSupported ||
        (majorWidget == majorSupported && minorWidget > minorSupported) ||
        (majorWidget == majorSupported && minorWidget == minorSupported && microWidget > microSupported))
    {
        _W("Platform doesn't support this widget.");
        return false;
    }
    return true;
}

bool TaskProcessConfig::isTizenWebApp() const
{
    if (m_installContext.widgetConfig.webAppType.appType == WrtDB::AppType::APP_TYPE_TIZENWEBAPP)
    {
        return true;
    }
    return false;
}

bool TaskProcessConfig::fillWidgetConfig(
    WrtDB::WidgetRegisterInfo& pWidgetConfigInfo,
    WrtDB::ConfigParserData& configInfo)
{
    pWidgetConfigInfo.guid = configInfo.widget_id;

    if (!!configInfo.version) {
        if (!pWidgetConfigInfo.version) {
            pWidgetConfigInfo.version = configInfo.version;
        } else {
            if (pWidgetConfigInfo.version != configInfo.version) {
                _E("Invalid archive");
                return false;
            }
        }
    }
    if (!!configInfo.minVersionRequired) {
        pWidgetConfigInfo.minVersion = configInfo.minVersionRequired;
    } else if (!!configInfo.tizenMinVersionRequired) {
        pWidgetConfigInfo.minVersion = configInfo.tizenMinVersionRequired;
    }
    return true;
}

void TaskProcessConfig::StartStep()
{
    LOGI("--------- <TaskProcessConfig> : START ----------");
}

void TaskProcessConfig::EndStep()
{
    LOGI("--------- <TaskProcessConfig> : END ----------");
}

} //namespace WidgetInstall
} //namespace Jobs
