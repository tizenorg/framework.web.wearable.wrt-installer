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
 *
 *
 * @file       backendlib.cpp
 * @author     Soyoung Kim (sy037.kim@samsung.com)
 * @version    0.1
 * @brief      This is implementation file for providing widget information
 *             to package manager
 */
#include "package-manager-plugin.h"
#include <package-manager.h>
#include <regex.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <vcore/VCore.h>
#include <dpl/wrt-dao-ro/WrtDatabase.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/wrt-dao-ro/feature_dao_read_only.h>
#include <dpl/wrt-dao-ro/widget_config.h>
#include <string>
#include <dpl/db/sql_connection.h>
#include <dpl/foreach.h>
#include <dpl/utils/folder_size.h>
#include <dpl/wrt-dao-ro/wrt_db_types.h>
#include <dpl/copy.h>
#include <dpl/abstract_waitable_input_adapter.h>
#include <dpl/abstract_waitable_output_adapter.h>
#include <dpl/zip_input.h>
#include <dpl/binary_queue.h>
#include <dpl/file_input.h>
#include <dpl/wrt-dao-ro/config_parser_data.h>
#include <dpl/localization/LanguageTagsProvider.h>
#include <dpl/optional_typedefs.h>
#include "root_parser.h"
#include "widget_parser.h"
#include "parser_runner.h"
#include <installer_log.h>

using namespace WrtDB;

#undef TRUE
#undef FALSE
#define TRUE 0
#define FALSE -1
#define GET_DIRECTORY_SIZE_KB(x)    (x) / 1024

#ifdef __cplusplus
extern "C"
{
#endif

static void pkg_native_plugin_on_unload();
static int pkg_plugin_app_is_installed(const char *pkg_name);
static int pkg_plugin_get_installed_apps_list(const char *category,
                                              const char *option,
                                              package_manager_pkg_info_t **list,
                                              int *count);
static int pkg_plugin_get_app_detail_info(
    const char *pkg_name,
    package_manager_pkg_detail_info_t *
    pkg_detail_info);
static int pkg_plugin_get_app_detail_info_from_package(
    const char *pkg_path,
    package_manager_pkg_detail_info_t
    *pkg_detail_info);

pkgmgr_info *pkgmgr_client_check_pkginfo_from_file(const char *pkg_path);

static void pkg_native_plugin_on_unload()
{
    _D("pkg_native_plugin_unload() is called");
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

static int pkg_plugin_app_is_installed(const char *pkg_name)
{
    const char* REG_PKGID_PATTERN = "^[a-zA-Z0-9]{10}$";
    _D("pkg_plugin_app_is_installed() is called");

    WrtDB::WrtDatabase::attachToThreadRO();

    {
        ScopeLocale locale;
        regex_t reg;
        if (regcomp(&reg, REG_PKGID_PATTERN, REG_NOSUB | REG_EXTENDED) != 0) {
            _D("Regcomp failed");
        }

        if (!(regexec(&reg, pkg_name,
                        static_cast<size_t>(0), NULL, 0) == 0))
        {
            _E("Invalid argument : %s", pkg_name);
            return FALSE;
        }
    }

    Try {
        WrtDB::TizenPkgId pkgid(DPL::FromUTF8String(pkg_name));
        WrtDB::TizenAppId appid = WidgetDAOReadOnly::getTizenAppId(pkgid);
        _D("appid : %ls", appid.c_str());
    } Catch(WidgetDAOReadOnly::Exception::WidgetNotExist) {
        WrtDB::WrtDatabase::detachFromThread();
        return FALSE;
    }
    WrtDB::WrtDatabase::detachFromThread();
    return TRUE;
}

static int pkg_plugin_get_installed_apps_list(const char * /*category*/,
                                              const char * /*option*/,
                                              package_manager_pkg_info_t **list,
                                              int *count)
{
    _D("pkg_plugin_get_installed_apps_list() is called");

    package_manager_pkg_info_t *pkg_list = NULL;
    package_manager_pkg_info_t *pkg_last = NULL;

    WrtDB::WrtDatabase::attachToThreadRO();
    TizenAppIdList tizenAppIdList = WidgetDAOReadOnly::getTizenAppIdList();
    *count = 0;

    FOREACH(iterator, tizenAppIdList) {
        package_manager_pkg_info_t *pkg_info =
            static_cast<package_manager_pkg_info_t*>
            (malloc(sizeof(package_manager_pkg_info_t)));

        if (pkg_info == NULL) {
            continue;
        }

        if (NULL == pkg_list) {
            pkg_list = pkg_info;
            pkg_last = pkg_info;
        } else {
            pkg_last->next = pkg_info;
        }

        TizenAppId tzAppid = *iterator;
        WidgetDAOReadOnly widget(tzAppid);
        strncpy(pkg_info->pkg_type, "wgt", PKG_TYPE_STRING_LEN_MAX);
        snprintf(pkg_info->pkg_name, PKG_NAME_STRING_LEN_MAX, "%s",
                 DPL::ToUTF8String(tzAppid).c_str());

        DPL::OptionalString version = widget.getVersion();
        if (!!version) {
            strncpy(pkg_info->version,
                    DPL::ToUTF8String(*version).c_str(),
                    PKG_VERSION_STRING_LEN_MAX - 1);
        }

        (*count)++;
        pkg_last = pkg_info;
    }
    *list = pkg_list;
    WrtDB::WrtDatabase::detachFromThread();

    return TRUE;
}

static int pkg_plugin_get_app_detail_info(
    const char *pkg_name,
    package_manager_pkg_detail_info_t *
    pkg_detail_info)
{
    _D("pkg_plugin_get_app_detail_info() is called");

    WrtDB::WrtDatabase::attachToThreadRO();

    WrtDB::TizenAppId appid;
    Try {
        WrtDB::TizenPkgId pkgid(DPL::FromUTF8String(pkg_name));
        appid = WidgetDAOReadOnly::getTizenAppId(pkgid);
        _D("appid : %ls", appid.c_str());
    } Catch(WidgetDAOReadOnly::Exception::WidgetNotExist) {
        WrtDB::WrtDatabase::detachFromThread();
        return FALSE;
    }

    WidgetDAOReadOnly widget(appid);

    DPL::OptionalString version = widget.getVersion();
    DPL::OptionalString id = widget.getGUID();
    DPL::OptionalString locale = widget.getDefaultlocale();

    if (!!version) {
        strncpy(pkg_detail_info->version,
                DPL::ToUTF8String(*version).c_str(),
                PKG_VERSION_STRING_LEN_MAX - 1);
    }
    snprintf(pkg_detail_info->pkgid, PKG_NAME_STRING_LEN_MAX, "%s",
             pkg_name);
    snprintf(pkg_detail_info->optional_id, PKG_NAME_STRING_LEN_MAX, "%s",
             DPL::ToUTF8String(appid).c_str());
    WidgetLocalizedInfo localizedInfo;

    if (!locale) {
        _D("locale is NULL");
        DPL::String languageTag(L"");
        localizedInfo = widget.getLocalizedInfo(languageTag);
    } else {
        localizedInfo = widget.getLocalizedInfo(*locale);
    }
    DPL::OptionalString desc(localizedInfo.description);

    if (!!desc) {
        strncpy(pkg_detail_info->pkg_description,
                DPL::ToUTF8String(*desc).c_str(),
                PKG_VALUE_STRING_LEN_MAX - 1);
    }
    strncpy(pkg_detail_info->pkg_type, "wgt", PKG_TYPE_STRING_LEN_MAX);
    strncpy(pkg_detail_info->pkg_name, pkg_name, PKG_NAME_STRING_LEN_MAX - 1);

    std::string min_version = DPL::ToUTF8String((*widget.getMinimumWacVersion()));

    strncpy(pkg_detail_info->min_platform_version, min_version.c_str(),
            PKG_VERSION_STRING_LEN_MAX - 1);

    /* set installed time */
    pkg_detail_info->installed_time = widget.getInstallTime();

    /* set Widget size */
    DPL::String pkgName = DPL::FromUTF8String(pkg_name);
    std::string installPath = WidgetConfig::GetWidgetBasePath(pkgName);
    std::string persistentPath =
        WidgetConfig::GetWidgetPersistentStoragePath(pkgName);
    std::string tempPath =
        WidgetConfig::GetWidgetTemporaryStoragePath(pkgName);
    installPath += "/";
    tempPath += "/";
    persistentPath += "/";

    size_t installedSize = Utils::getFolderSize(installPath);
    size_t persistentSize = Utils::getFolderSize(persistentPath);
    size_t appSize = installedSize - persistentSize;
    size_t dataSize = persistentSize + Utils::getFolderSize(tempPath);

    pkg_detail_info->installed_size = GET_DIRECTORY_SIZE_KB(installedSize);
    pkg_detail_info->app_size = GET_DIRECTORY_SIZE_KB(appSize);
    pkg_detail_info->data_size = GET_DIRECTORY_SIZE_KB(dataSize);

    WrtDB::WrtDatabase::detachFromThread();
    return TRUE;
}

int getConfigParserData(const std::string &widgetPath, ConfigParserData& configInfo)
{
    const char* CONFIG_XML = "config.xml";
    const char* WITH_OSP_XML = "res/wgt/config.xml";

    Try {
        ParserRunner parser;

        std::unique_ptr<DPL::ZipInput> zipFile(
                new DPL::ZipInput(widgetPath));

        std::unique_ptr<DPL::ZipInput::File> configFile;

        // Open config.xml file
        Try {
            configFile.reset(zipFile->OpenFile(CONFIG_XML));
        }
        Catch(DPL::ZipInput::Exception::OpenFileFailed)
        {
            configFile.reset(zipFile->OpenFile(WITH_OSP_XML));
        }

        // Extract config
        DPL::BinaryQueue buffer;
        DPL::AbstractWaitableInputAdapter inputAdapter(configFile.get());
        DPL::AbstractWaitableOutputAdapter outputAdapter(&buffer);
        DPL::Copy(&inputAdapter, &outputAdapter);
        parser.Parse(&buffer,
                ElementParserPtr(
                    new RootParser<WidgetParser>(configInfo,
                        DPL::
                        FromUTF32String(
                            L"widget"))));
    }
    Catch(DPL::ZipInput::Exception::OpenFailed)
    {
        _E("Failed to open widget package");
        return FALSE;
    }
    Catch(DPL::ZipInput::Exception::OpenFileFailed)
    {
        _E("Failed to open config.xml file");
        return FALSE;
    }
    Catch(DPL::CopyFailed)
    {
        _E("Failed to extract config.xml file");
        return FALSE;
    }
    Catch(DPL::FileInput::Exception::OpenFailed)
    {
        _E("Failed to open config.xml file");
        return FALSE;
    }
    Catch(ElementParser::Exception::ParseError)
    {
        _E("Failed to parse config.xml file");
        return FALSE;
    }
    Catch(DPL::ZipInput::Exception::SeekFileFailed)
    {
        _E("Failed to seek widget archive - corrupted package?");
        return FALSE;
    }

    return TRUE;
}

char* getIconInfo(const std::string &widgetPath,
        const std::string &icon_name, int &icon_size)
{
    Try {
        std::unique_ptr<DPL::ZipInput> zipFile(
                new DPL::ZipInput(widgetPath));

        std::unique_ptr<DPL::ZipInput::File> iconFile;

        Try {
            iconFile.reset(zipFile->OpenFile(icon_name));
        }
        Catch(DPL::ZipInput::Exception::OpenFileFailed)
        {
            _D("This web app is hybrid web app");
            std::string hybrid_icon = "res/wgt/" + icon_name;
            iconFile.reset(zipFile->OpenFile(hybrid_icon));
        }

        DPL::BinaryQueue buffer;
        DPL::AbstractWaitableInputAdapter inputAdapter(iconFile.get());
        DPL::AbstractWaitableOutputAdapter outputAdapter(&buffer);
        DPL::Copy(&inputAdapter, &outputAdapter);
        icon_size = buffer.Size();
        char *getBuffer = (char*) calloc(1, (sizeof(char) * icon_size) + 1);
        buffer.Flatten(getBuffer, buffer.Size());
        return getBuffer;
    }
    Catch(DPL::ZipInput::Exception::OpenFailed)
    {
        _D("Failed to open widget package");
        return NULL;
    }
    Catch(DPL::ZipInput::Exception::OpenFileFailed)
    {
        _D("Not found icon file %s", icon_name.c_str());
        return NULL;
    }
}

char* getIconForLocale(const std::string& bp, const std::string& tag,
                                     const std::string& icon, int & size)
{
    std::string iconPath;
    if (!tag.empty()) {
        iconPath += std::string("locales/") + tag;
    }
    if (!iconPath.empty()) {
        iconPath += "/";
    }

    iconPath += icon;
    return getIconInfo(bp, iconPath, size);
}

char* getIcon(const std::string & basepath, const WrtDB::ConfigParserData & config, int & size)
{
    const std::list<std::string> defaultIcons{ "icon.svg", "icon.ico", "icon.png", "icon.gif", "icon.jpg" };
    LanguageTags tagsList =
        LanguageTagsProviderSingleton::Instance().getLanguageTags();

    char * result = NULL;

    //for each locale tag - searching for icon presence and returning raw data
    //first found is best as locale tags are ordered
    FOREACH(tag, tagsList)
    {
        FOREACH(icon, config.iconsList)
        {
            std::string src = DPL::ToUTF8String(icon->src);
            result = getIconForLocale(basepath, DPL::ToUTF8String(*tag), src, size);
            if(result) {
                return result;
            }
        }
        FOREACH(i, defaultIcons)
        {
            result = getIconForLocale(basepath, DPL::ToUTF8String(*tag), *i, size);
            if(result) {
                return result;
            }
        }
    }
    return NULL;
}

int getWidgetDetailInfoFromPackage(const char* pkgPath,
        package_manager_pkg_detail_info_t* pkg_detail_info)
{
    const std::string widget_path(pkgPath);
    ConfigParserData configInfo;

    if (FALSE == getConfigParserData(widget_path, configInfo)) {
        return FALSE;
    }

    strncpy(pkg_detail_info->pkg_type, "wgt", PKG_TYPE_STRING_LEN_MAX);
    if (!!configInfo.tizenPkgId) {
        strncpy(pkg_detail_info->pkgid,
                DPL::ToUTF8String(*configInfo.tizenPkgId).c_str(), PKG_TYPE_STRING_LEN_MAX - 1);
    }
    if (!!configInfo.tizenAppId) {
        strncpy(pkg_detail_info->pkg_name,
                DPL::ToUTF8String(*configInfo.tizenAppId).c_str(),
                PKG_NAME_STRING_LEN_MAX - 1);
    }
    if (!!configInfo.version) {
        strncpy(pkg_detail_info->version,
                DPL::ToUTF8String(*configInfo.version).c_str(),
                PKG_VERSION_STRING_LEN_MAX - 1);
    }

    DPL::OptionalString name;
    DPL::OptionalString desc;

    LanguageTags tags = LanguageTagsProviderSingleton::Instance().getLanguageTags();

    auto toLowerCase = [](const DPL::String & r)
    {
        DPL::String result;
        std::transform(r.begin(), r.end(), std::inserter(result, result.begin()), ::tolower);
        return result;
    };

    if (!!configInfo.defaultlocale)
    {
        Locale & dl = *configInfo.defaultlocale;
        configInfo.defaultlocale = toLowerCase(dl);
    }

    bool found = false;
    FOREACH(tag, tags)
    {
        *tag = toLowerCase(*tag);
        FOREACH(localizedData, configInfo.localizedDataSet)
        {
            Locale i = localizedData->first;
            i = toLowerCase(i);

            if (!!configInfo.defaultlocale && *configInfo.defaultlocale == i)
            {
                name = localizedData->second.name;
                desc = localizedData->second.description;
            }
            if (*tag == i)
            {
                name = localizedData->second.name;
                desc = localizedData->second.description;
                found = true;
                break;
            }
        }
        if(found) break;
    }

    if (!!name) {
        strncpy(pkg_detail_info->label, DPL::ToUTF8String(*name).c_str(),
                PKG_LABEL_STRING_LEN_MAX - 1);
    }

    if (!!desc) {
        strncpy(pkg_detail_info->pkg_description,
                DPL::ToUTF8String(*desc).c_str(),
                PKG_VALUE_STRING_LEN_MAX - 1);
    }

    if (!!configInfo.tizenMinVersionRequired) {
        strncpy(pkg_detail_info->min_platform_version,
                DPL::ToUTF8String(*configInfo.tizenMinVersionRequired).c_str(),
                PKG_VERSION_STRING_LEN_MAX - 1);
    }

    if (!!configInfo.authorName) {
        strncpy(pkg_detail_info->author,
                DPL::ToUTF8String(*configInfo.authorName).c_str(),
                PKG_VALUE_STRING_LEN_MAX - 1);
    }


    pkg_detail_info->privilege_list = NULL;
    FOREACH(it, configInfo.featuresList) {
        std::string featureInfo =  DPL::ToUTF8String(it->name);
        _D("privilege : %s", featureInfo.c_str());
        int length = featureInfo.size();
        char *privilege = (char*) calloc(1, (sizeof(char) * (length + 1)));
        if (privilege == NULL) {
            continue;
        }
        snprintf(privilege, length + 1, "%s", featureInfo.c_str());
        pkg_detail_info->privilege_list =
            g_list_append(pkg_detail_info->privilege_list, privilege);
    }

    char* icon_buf = getIcon(widget_path, configInfo, pkg_detail_info->icon_size);

    if (icon_buf) {
        _D("icon size : %d", pkg_detail_info->icon_size);
        pkg_detail_info->icon_buf = icon_buf;
    } else {
        _D("No icon");
        pkg_detail_info->icon_size = 0;
        pkg_detail_info->icon_buf = NULL;
    }

    return TRUE;
}

static int pkg_plugin_get_app_detail_info_from_package(
    const char * pkg_path,
    package_manager_pkg_detail_info_t * pkg_detail_info)
{
    _D("pkg_plugin_get_app_detail_info_from_package() is called");
    return getWidgetDetailInfoFromPackage(pkg_path, pkg_detail_info);
}

pkgmgr_info *pkgmgr_client_check_pkginfo_from_file(const char *pkg_path)
{
    _D("pkgmgr_client_check_pkginfo_from_file() is called");
    package_manager_pkg_detail_info_t *pkg_detail_info;
    pkg_detail_info = (package_manager_pkg_detail_info_t*)malloc(
            sizeof(package_manager_pkg_detail_info_t));
    if (pkg_detail_info == NULL) {
        _E("Failed to get memory alloc");
        return NULL;
    }
    int ret = getWidgetDetailInfoFromPackage(pkg_path, pkg_detail_info);
    if (FALSE == ret) {
        _E("Failed to get package detail info ");
        free(pkg_detail_info);
        return NULL;
    }
    return reinterpret_cast<pkgmgr_info*>(pkg_detail_info);
}

__attribute__ ((visibility("default")))
int pkg_plugin_on_load(pkg_plugin_set *set)
{
    DPL::Log::LogSystemSingleton::Instance().SetTag("WGT-BACKLIB");
    if (NULL == set) {
        return FALSE;
    }
    memset(set, 0x00, sizeof(pkg_plugin_set));

    set->plugin_on_unload = pkg_native_plugin_on_unload;
    set->pkg_is_installed = pkg_plugin_app_is_installed;
    set->get_installed_pkg_list = pkg_plugin_get_installed_apps_list;
    set->get_pkg_detail_info = pkg_plugin_get_app_detail_info;
    set->get_pkg_detail_info_from_package =
        pkg_plugin_get_app_detail_info_from_package;

    return TRUE;
}

#ifdef __cplusplus
}
#endif
