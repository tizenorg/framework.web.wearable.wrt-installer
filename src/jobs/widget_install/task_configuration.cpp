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
 * @file    task_configuration.cpp
 * @version 1.0
 * @author  Tomasz Iwanek
 * @brief   implementation file for configuration task
 */
#include "task_configuration.h"

#include <string>
#include <sstream>
#include <memory>
#include <sys/time.h>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include <limits.h>
#include <regex.h>
#include <vconf.h>
#include <web_provider_service.h>

#include <dpl/utils/wrt_utility.h>
#include <dpl/utils/path.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/wrt-dao-ro/config_parser_data.h>
#include <dpl/localization/w3c_file_localization.h>

#include <libiriwrapper.h>
#include <app_manager.h>

#include "root_parser.h"
#include "widget_parser.h"
#include "parser_runner.h"

#include <widget_install/widget_install_errors.h>
#include <widget_install/widget_install_context.h>
#include <widget_install_to_external.h>
#include <widget_install/widget_unzip.h>
#include <widget_install/job_widget_install.h>
#include <widget_install/task_commons.h>
#include <installer_log.h>

using namespace WrtDB;

namespace {
const char* const CONFIG_XML = "config.xml";
const char* const WITH_OSP_XML = "res/wgt/config.xml";
const char* const OSP_MANIFEST_XML = "info/manifest.xml";
#ifdef CORE_HYBRID
const char* const CORE_MANIFEST_XML = "tizen-manifest.xml";
#endif
const char* const WRT_WIDGETS_XML_SCHEMA = "/usr/etc/wrt-installer/widgets.xsd";

//allowed: a-z, A-Z, 0-9
const char* REG_TIZENID_PATTERN = "^[a-zA-Z0-9]{10}.{1,}$";
const char* REG_PKGID_PATTERN = "^[a-zA-Z0-9]{10}$";
const char* REG_NAME_PATTERN = "^[a-zA-Z0-9._-]{1,}$";
const size_t PACKAGE_ID_LENGTH = 10;

static const DPL::String SETTING_VALUE_ENCRYPTION = L"encryption";
static const DPL::String SETTING_VALUE_ENCRYPTION_ENABLE = L"enable";
static const DPL::String SETTING_VALUE_ENCRYPTION_DISABLE = L"disable";
const DPL::String SETTING_VALUE_INSTALLTOEXT_NAME = L"install-location";
const DPL::String SETTING_VALUE_INSTALLTOEXT_PREPER_EXT = L"prefer-external";
const DPL::String SETTING_VALUE_INSTALLTOEXT_AUTO = L"auto";
const std::string XML_EXTENSION = ".xml";

bool hasExtension(const std::string& filename, const std::string& extension)
{
    _D("Looking for extension %s in %s", extension.c_str(), filename.c_str());
    size_t fileLen = filename.length();
    size_t extLen = extension.length();
    if (fileLen < extLen) {
        _E("Filename %s is shorter than extension %s", filename.c_str(), extension.c_str());
        return false;
    }
    return (0 == filename.compare(fileLen - extLen, extLen, extension));
}
} // namespace anonymous

namespace Jobs {
namespace WidgetInstall {

TaskConfiguration::TaskConfiguration(InstallerContext& context) :
    DPL::TaskDecl<TaskConfiguration>(this),
    m_context(context),
    m_widgetConfig(m_context.widgetConfig.configInfo)
{
    AddStep(&TaskConfiguration::StartStep);

    AddStep(&TaskConfiguration::SetupTempDirStep);
    AddStep(&TaskConfiguration::UnzipConfigurationStep);
    AddStep(&TaskConfiguration::ParseXMLConfigStep);

    AddStep(&TaskConfiguration::TizenIdStep);
    AddStep(&TaskConfiguration::ApplicationTypeStep);
    AddStep(&TaskConfiguration::ResourceEncryptionStep);
    AddStep(&TaskConfiguration::InstallationFSLocationStep);

    AddStep(&TaskConfiguration::DetectUpdateInstallationStep);
    AddStep(&TaskConfiguration::CheckRDSSupportStep);

    AddStep(&TaskConfiguration::ConfigureWidgetLocationStep);
    AddStep(&TaskConfiguration::PkgmgrStartStep);

    AddStep(&TaskConfiguration::AppendTasklistStep);

    AddStep(&TaskConfiguration::EndStep);
}

void TaskConfiguration::StartStep()
{
    LOGI("--------- <TaskConfiguration> : START ----------");
}

void TaskConfiguration::EndStep()
{
    m_context.job->UpdateProgress(InstallerContext::INSTALL_PARSE_CONFIG,
            "Parse config.xml and set structure");
    LOGI("--------- <TaskConfiguration> : END ----------");
}

void TaskConfiguration::PkgmgrStartStep()
{
    pkgMgrInterface()->setPkgname(DPL::ToUTF8String(m_context.widgetConfig.tzPkgid));
    pkgMgrInterface()->sendProgress(0);
}

void TaskConfiguration::AppendTasklistStep()
{
    switch(m_context.widgetConfig.webAppType.appType)
    {
        case APP_TYPE_TIZENWEBAPP:
            if (m_context.mode.installTime == InstallMode::InstallTime::FOTA) {
                if (!m_context.isUpdateMode) {
                    _D("TaskConfiguration -> fota installation task list");
                    m_context.job->appendFotaInstallationTaskList();
                } else {
                    _D("TaskConfiguration -> fota update task list");
                    m_context.job->appendFotaUpdateTaskList();
                }
            } else {
                if (!m_context.isUpdateMode) {
                    _D("TaskConfiguration -> new installation task list");
                    m_context.job->appendNewInstallationTaskList();
                } else {
                    if (m_context.mode.command == InstallMode::Command::REINSTALL) {
                        _D("TaskConfiguration -> rds update task list");
                        m_context.job->appendRDSUpdateTaskList();
                    } else if(m_context.mode.command == InstallMode::Command::RECOVERY) {
                        _D("TaskConfiguration -> recovery task list");
                        m_context.job->appendRecoveryTaskList();
                    } else {
                        _D("TaskConfiguration -> update installation task list");
                        m_context.job->appendUpdateInstallationTaskList();
                    }
                }
            }
            break;
        default:
            ThrowMsg(Jobs::WidgetInstall::Exceptions::WidgetConfigFileInvalid, "Unknown application type");
    }
}

std::shared_ptr<PackageManager::IPkgmgrSignal> TaskConfiguration::pkgMgrInterface()
{
    return m_context.job->GetInstallerStruct().pkgmgrInterface;
}

void TaskConfiguration::SetupTempDirStep()
{
    _D("widgetPath: %s", m_context.requestedPath.c_str());
    _D("tempPath: %s", m_tempDir.c_str());
    if (m_context.mode.extension == InstallMode::ExtensionType::DIR) {
        if (m_context.mode.command ==
                InstallMode::Command::REINSTALL) {
            std::ostringstream tempPathBuilder;
            tempPathBuilder << WrtDB::GlobalConfig::GetUserInstalledWidgetPath();
            tempPathBuilder << WrtDB::GlobalConfig::GetTmpDirPath();
            tempPathBuilder << "/";
            tempPathBuilder << m_context.requestedPath;
            m_tempDir = tempPathBuilder.str();
        } else if(m_context.mode.command == InstallMode::Command::RECOVERY) {
            m_tempDir = Jobs::WidgetInstall::createTempPath(false);
        } else {
            m_tempDir = m_context.requestedPath;
        }
    } else {
        m_tempDir =
            Jobs::WidgetInstall::createTempPath(
                    m_context.mode.rootPath ==
                        InstallMode::RootPath::RO);
    }
}

void TaskConfiguration::UnzipConfigurationStep()
{
    _D("UnzipConfigurationStep");
    if (m_context.mode.extension != InstallMode::ExtensionType::DIR) {
        if(!hasExtension(m_context.requestedPath, XML_EXTENSION)) //unzip everything except xml files
        {
            WidgetUnzip wgtUnzip(m_context.requestedPath);
            wgtUnzip.unzipConfiguration(m_tempDir, &m_context.widgetConfig.packagingType);
            m_configuration += m_tempDir + "/" + CONFIG_XML;
        } else{
            m_context.widgetConfig.packagingType = PKG_TYPE_HOSTED_WEB_APP;
            m_configuration += m_context.requestedPath;
        }
    } else {
        std::string pkgPath = m_tempDir + "/";
        std::string configFile = pkgPath + WITH_OSP_XML;
        if (!WrtUtilFileExists(configFile)) {
            configFile = pkgPath + CONFIG_XML;
            if (!WrtUtilFileExists(configFile)) {
                std::string tzAppId = m_context.requestedPath.
                    substr(m_context.requestedPath.find_last_of("/")+1);
                Try {
                    WidgetDAOReadOnly dao(WidgetDAOReadOnly::getTizenAppId(DPL::FromUTF8String(tzAppId)));
                    pkgPath = DPL::ToUTF8String(*dao.getWidgetInstalledPath());
                    pkgPath += "/";
                    configFile = pkgPath + WITH_OSP_XML;
                }
                Catch(WidgetDAOReadOnly::Exception::WidgetNotExist)
                {
                    _E("Given tizenId not found in database");
                    ThrowMsg(Exceptions::DatabaseFailure, "Given tizenId not found in database");
                }
            }
        }

        std::string osp_manifestFile = pkgPath + OSP_MANIFEST_XML;
        if (WrtUtilFileExists(osp_manifestFile)){
            m_context.widgetConfig.packagingType = PKG_TYPE_HYBRID_WEB_APP;
        }else{
            m_context.widgetConfig.packagingType = PKG_TYPE_NOMAL_WEB_APP;
        }

#ifdef CORE_HYBRID
        std::string core_manifestFile = pkgPath + CORE_MANIFEST_XML;
        if(WrtUtilFileExists(core_manifestFile)){
            m_context.widgetConfig.packagingType = PKG_TYPE_HYBRID_WEB_APP;
        }
#endif

        m_configuration = configFile;
    }
    _D("m_configuration : %s", m_configuration.c_str());
    _D("Package Type : %s", m_context.widgetConfig.packagingType.getPkgtypeToString().c_str());
}

void TaskConfiguration::ParseXMLConfigStep()
{
    _D("ParseXMLConfigStep");
    // Parse config
    ParserRunner parser;
    Try
    {
        _D("m_configuration : %s", m_configuration.c_str());
        if(!DPL::Utils::Path(m_configuration).Exists())
        {
            ThrowMsg(Exceptions::MissingConfig, "Config file not exists");
        }

#ifdef SCHEMA_VALIDATION_ENABLED
        if(!parser.Validate(m_configuration, WRT_WIDGETS_XML_SCHEMA))
        {
            _E("Invalid configuration file - schema validation failed");
            ThrowMsg(Exceptions::WidgetConfigFileInvalid, "Failed to parse config.xml file");
        }
#endif
        parser.Parse(m_configuration,
                ElementParserPtr(
                    new RootParser<WidgetParser>(m_widgetConfig,
                        DPL::
                        FromUTF32String(
                            L"widget"))));
    }
    Catch(ElementParser::Exception::ParseError)
    {
        _E("Failed to parse config.xml file");
        ThrowMsg(Exceptions::WidgetConfigFileInvalid, "Parser exeption");
    }
    Catch(WidgetDAOReadOnly::Exception::WidgetNotExist)
    {
        _E("Failed to find installed widget - give proper tizenId");
        ThrowMsg(Exceptions::RDSDeltaFailure, "WidgetNotExist");
    }
    Catch(Exceptions::WidgetConfigFileNotFound){
        _E("Failed to find config.xml");
        ThrowMsg(Exceptions::MissingConfig, "Parser exeption");
    }

    if (m_context.mode.extension != InstallMode::ExtensionType::DIR) {
        if (!WrtUtilRemove(m_configuration)) {
            _E("Error occurs during removing %s", m_configuration.c_str());
        }
    }

}

// regexec() function does not work properly in specific locale (ex, Estonian)
// So, change locale temporally before call regcomp and regexec
class ScopeLocale {
public:
    ScopeLocale() {
        currentLocale = setlocale(LC_ALL , NULL);
        if (NULL == setlocale(LC_ALL, "C")) {
            _W("Failed to change locale to \"C\"");
        }
    }

    ~ScopeLocale() {
        if (NULL == setlocale(LC_ALL, currentLocale.c_str())) {
            _W("Failed to set previous locale");
        }
    }

private:
    std::string currentLocale;
};

void TaskConfiguration::TizenIdStep()
{
    bool shouldMakeAppid = false;
    using namespace PackageManager;

    if (!!m_widgetConfig.tizenAppId) {
        _D("Setting tizenAppId provided in config.xml: %s", DPL::ToUTF8String(*m_widgetConfig.tizenAppId).c_str());

        m_context.widgetConfig.tzAppid = *m_widgetConfig.tizenAppId;
        //check package id.
        if (!!m_widgetConfig.tizenPkgId) {
            _D("Setting tizenPkgId provided in config.xml: %s", DPL::ToUTF8String(*m_widgetConfig.tizenPkgId).c_str());

            m_context.widgetConfig.tzPkgid = *m_widgetConfig.tizenPkgId;
        } else {
            DPL::String appid = *m_widgetConfig.tizenAppId;
            if (appid.length() > PACKAGE_ID_LENGTH) {
                m_context.widgetConfig.tzPkgid =
                    appid.substr(0, PACKAGE_ID_LENGTH);
            } else {
                //old version appid only has 10byte random character is able to install for a while.
                //this case appid equal pkgid.
                m_context.widgetConfig.tzPkgid =
                    *m_widgetConfig.tizenAppId;
                shouldMakeAppid = true;
            }
        }
    } else {
        shouldMakeAppid = true;
        TizenPkgId pkgId = WidgetDAOReadOnly::generatePkgId();
        _D("Checking if pkg id is unique");
        while (true) {
            if (!validateTizenPackageID(pkgId)) {
                //path exist, chose another one
                pkgId = WidgetDAOReadOnly::generatePkgId();
                continue;
            }
            break;
        }
        m_context.widgetConfig.tzPkgid = pkgId;
        _D("tizen_id name was generated by WRT: %ls", m_context.widgetConfig.tzPkgid.c_str());
    }

    if (shouldMakeAppid == true) {
        DPL::OptionalString name;
        DPL::OptionalString defaultLocale = m_widgetConfig.defaultlocale;

        FOREACH(localizedData, m_widgetConfig.localizedDataSet)
        {
            Locale i = localizedData->first;
            if (!!defaultLocale) {
                if (defaultLocale == i) {
                    name = localizedData->second.name;
                    break;
                }
            } else {
                name = localizedData->second.name;
                break;
            }
        }

        {
            ScopeLocale locale;
            regex_t regx;
            if (regcomp(&regx, REG_NAME_PATTERN, REG_NOSUB | REG_EXTENDED) != 0) {
                _D("Regcomp failed");
            }

            _D("Name : %ls", !!name ? (*name).c_str() : L"NULL");
            if (!name || (regexec(&regx, DPL::ToUTF8String(*name).c_str(),
                                  static_cast<size_t>(0), NULL, 0) != REG_NOERROR))
            {
                const std::string allowedString("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
                std::ostringstream genName;
                struct timeval tv;
                gettimeofday(&tv, NULL);
                unsigned int seed = time(NULL) + tv.tv_usec;

                genName << "_" << allowedString[rand_r(&seed) % allowedString.length()];
                name = DPL::FromUTF8String(genName.str());
                _D("name was generated by WRT");
            }
            regfree(&regx);
        }

        _D("Name : %ls", (*name).c_str());
        std::ostringstream genid;
        genid << m_context.widgetConfig.tzPkgid << "." << (*name);
        _D("tizen appid was generated by WRT : %s", genid.str().c_str());

        DPL::OptionalString appid = DPL::FromUTF8String(genid.str());
        NormalizeAndTrimSpaceString(appid);
        m_context.widgetConfig.tzAppid = *appid;
    }

    // send start signal of pkgmgr
    pkgMgrInterface()->setPkgname(DPL::ToUTF8String(m_context.widgetConfig.tzPkgid));

    _D("Tizen App Id : %ls", (m_context.widgetConfig.tzAppid).c_str());
    _D("Tizen Pkg Id : %ls", (m_context.widgetConfig.tzPkgid).c_str());
}

void TaskConfiguration::ConfigureWidgetLocationStep()
{
    m_context.locations =
        WidgetLocation(DPL::ToUTF8String(m_context.widgetConfig.tzPkgid),
                       m_context.requestedPath, m_tempDir,
                       m_context.widgetConfig.packagingType,
                       m_context.mode.rootPath ==
                           InstallMode::RootPath::RO,
                           m_context.mode.extension);
    m_context.locations->registerAppid(
        DPL::ToUTF8String(m_context.widgetConfig.tzAppid));
#ifdef SERVICE_ENABLED
    FOREACH(it, m_context.widgetConfig.configInfo.serviceAppInfoList)
    {
        m_context.locations->registerServiceAppid(DPL::ToUTF8String(it->serviceId));
    }
#endif
    _D("widgetSource %s", m_context.requestedPath.c_str());
}

void TaskConfiguration::DetectUpdateInstallationStep()
{
    // checking installed web application
    Try {
        // no excpetion means, it isn't update mode
        WidgetUpdateInfo updateInfo = detectWidgetUpdate(m_widgetConfig,
                                                        m_context.widgetConfig.tzPkgid);

        // to support stub application, prohibit csc downgrade.
        if (!m_context.mode.cscPath.empty() && !!updateInfo.existingVersion && !!updateInfo.incomingVersion) {
            if (*(updateInfo.existingVersion) >= *(updateInfo.incomingVersion)) {
                _D("in case of csc mode, downgrade is not permitted");
                ThrowMsg(Jobs::WidgetInstall::Exceptions::PackageAlreadyInstalled,
                    "package is already installed");
            }
        }

        m_context.isUpdateMode = true;

        //if update, notify pkgmgr that this is update
        pkgMgrInterface()->startJob(InstallationType::UpdateInstallation);
    }
    Catch(WidgetDAOReadOnly::Exception::WidgetNotExist) {
        pkgMgrInterface()->startJob(InstallationType::NewInstallation);

        m_context.isUpdateMode = false;

        if (!validateTizenApplicationID(
            m_context.widgetConfig.tzAppid))
        {
            _E("tizen application ID is already used");
            ThrowMsg(Jobs::WidgetInstall::Exceptions::WidgetConfigFileInvalid,
                "invalid config");
        }
        if (!validateTizenPackageID(m_context.widgetConfig.tzPkgid)) {
            _E("tizen package ID is already used");
            ThrowMsg(Jobs::WidgetInstall::Exceptions::PackageAlreadyInstalled,
                "package is already installed");
        }
    }
}

void TaskConfiguration::CheckRDSSupportStep()
{
    //update needs RDS support to go ahead if REINSTALL command is given
    if(m_context.isUpdateMode)
    {
        if (!checkSupportRDSUpdateIfReinstall(m_widgetConfig)) {
            ThrowMsg(Jobs::WidgetInstall::Exceptions::NotSupportRDSUpdate,
                "RDS update failed");
        }
    }
}

bool TaskConfiguration::validateTizenApplicationID(
    const WrtDB::TizenAppId &tizenAppId)
{
    _D("tizen application ID = [%ls]", tizenAppId.c_str());
    ScopeLocale locale;

    regex_t reg;
    if (regcomp(&reg, REG_TIZENID_PATTERN, REG_NOSUB | REG_EXTENDED) != 0) {
        _D("Regcomp failed");
        return false;
    }

    if (regexec(&reg, DPL::ToUTF8String(tizenAppId).c_str(), 0, NULL, 0)
        == REG_NOMATCH)
    {
        regfree(&reg);
        return false;
    }
    regfree(&reg);

    return true;
}

bool TaskConfiguration::validateTizenPackageID(
    const WrtDB::TizenPkgId &tizenPkgId)
{
    _D("tizen application ID = [%ls]", tizenPkgId.c_str());
    ScopeLocale locale;

    regex_t reg;
    if (regcomp(&reg, REG_PKGID_PATTERN, REG_NOSUB | REG_EXTENDED) != 0)
    {
        _D("Regcomp failed");
        return false;
    }

    if (regexec(&reg, DPL::ToUTF8String(tizenPkgId).c_str(), 0, NULL, 0) == REG_NOMATCH)
    {
        regfree(&reg);
        return false;
    }
    regfree(&reg);

    return true;
}

WidgetUpdateInfo TaskConfiguration::detectWidgetUpdate(
    const ConfigParserData &configInfo,
    const WrtDB::TizenPkgId &pkgId)
{
    _D("Checking up widget package for config.xml...");
    OptionalWidgetVersion incomingVersion;

    if (!!configInfo.version) {
        incomingVersion =
            OptionalWidgetVersion(
                WidgetVersion(*configInfo.version));
    }

    OptionalWidgetVersion optVersion;
    WrtDB::AppType appType = WrtDB::APP_TYPE_UNKNOWN;
    WrtDB::TizenAppId appId;

    //fallback to normal behaviour
    WidgetDAOReadOnly dao(WidgetDAOReadOnly::getHandleByPkgId(pkgId));

    appType = WrtDB::APP_TYPE_TIZENWEBAPP;
    appId = dao.getTizenAppId();

    DPL::OptionalString version = dao.getVersion();
    if (!version.IsNull()) {
        optVersion = OptionalWidgetVersion(WidgetVersion(*version));
    }
    m_detectedType = WrtDB::APP_TYPE_TIZENWEBAPP;

    return WidgetUpdateInfo(
        appId,
        optVersion,
        incomingVersion,
        appType);
}

void TaskConfiguration::ApplicationTypeStep() //TODO: is this really needed as WAC is not supported? - need for card type applciations
{
    AppType widgetAppType = APP_TYPE_UNKNOWN;
    FOREACH(iterator, m_widgetConfig.nameSpaces) {
        _D("namespace = [%ls]", (*iterator).c_str());

        if (*iterator == ConfigurationNamespace::TizenWebAppNamespaceName) {
            widgetAppType = APP_TYPE_TIZENWEBAPP;
            break;
        }
    }

    m_context.widgetConfig.webAppType = widgetAppType;

    _D("type = [%s]", m_context.widgetConfig.webAppType.getApptypeToString().c_str());
}

void TaskConfiguration::ResourceEncryptionStep()
{
    m_context.needEncryption = false;
    FOREACH(it, m_widgetConfig.settingsList)
    {
        if (it->m_name == SETTING_VALUE_ENCRYPTION &&
            it->m_value == SETTING_VALUE_ENCRYPTION_ENABLE)
        {
            _D("resource need encryption");
            m_context.needEncryption = true;
        }
    }
}

void TaskConfiguration::InstallationFSLocationStep()
{
    if (m_context.mode.installTime == InstallMode::InstallTime::NORMAL) {
        FOREACH(it, m_widgetConfig.settingsList) {
            if (it->m_name == SETTING_VALUE_INSTALLTOEXT_NAME) {
                if (it->m_value == SETTING_VALUE_INSTALLTOEXT_AUTO) {
                    m_context.locationType = INSTALL_LOCATION_TYPE_AUTO;
                } else if (it->m_value == SETTING_VALUE_INSTALLTOEXT_PREPER_EXT) {
                    m_context.locationType =
                        INSTALL_LOCATION_TYPE_PREFER_EXTERNAL;
                } else {
                    m_context.locationType =
                        INSTALL_LOCATION_TYPE_INTERNAL_ONLY;
                }
                break;
            }
        }
    } else {
        m_context.locationType = INSTALL_LOCATION_TYPE_INTERNAL_ONLY;
    }
}

bool TaskConfiguration::checkSupportRDSUpdateIfReinstall(const WrtDB::ConfigParserData
        &configInfo)
{
    if (m_context.mode.command ==
            InstallMode::Command::REINSTALL)
    {
        DPL::String configValue = SETTING_VALUE_ENCRYPTION_DISABLE;
        DPL::String dbValue = SETTING_VALUE_ENCRYPTION_DISABLE;

        WidgetDAOReadOnly dao(m_context.widgetConfig.tzAppid);
        WrtDB::WidgetSettings widgetSettings;
        dao.getWidgetSettings(widgetSettings);

        FOREACH(it, widgetSettings) {
            if (it->settingName == SETTING_VALUE_ENCRYPTION) {
                dbValue = it->settingValue;
            }
        }

        FOREACH(data, configInfo.settingsList)
        {
            if (data->m_name == SETTING_VALUE_ENCRYPTION)
            {
                configValue = data->m_value;
            }
        }
        if (configValue != dbValue) {
            _E("Not Support RDS mode because of encryption setting");
            return false;
        }
    }

    return true;
}

}
}
