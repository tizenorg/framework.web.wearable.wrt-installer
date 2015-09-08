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
 * @file        widget_location.cpp
 * @author      Iwanek Tomasz (t.iwanek@smasung.com)
 */
#include "widget_location.h"

#include <unistd.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/assert.h>
#include <dpl/sstream.h>
#include <dpl/localization/localization_utils.h>
#include <widget_install/task_commons.h>
#include <installer_log.h>

WidgetLocation::DirectoryDeletor::DirectoryDeletor(bool isReadOnly) :
    m_dirpath(Jobs::WidgetInstall::createTempPath(isReadOnly))
{}

WidgetLocation::DirectoryDeletor::DirectoryDeletor(std::string tempPath) :
        m_dirpath(tempPath)
{}

WidgetLocation::DirectoryDeletor::~DirectoryDeletor()
{
    _D("Removing widget installation temporary directory: %s", m_dirpath.c_str());
    std::string roPath = WrtDB::GlobalConfig::GetUserPreloadedWidgetPath();

    if (0 != m_dirpath.compare(0, roPath.length(), roPath)) {
        if (!WrtUtilRemove(m_dirpath)) {
            _W("Fail at removing directory: %s", m_dirpath.c_str());
        }
    }
}

std::string WidgetLocation::DirectoryDeletor::getTempPath() const
{
    return m_dirpath;
}

WidgetLocation::WidgetLocation() :
    m_temp(new WidgetLocation::DirectoryDeletor(true))
{}

WidgetLocation::WidgetLocation(const std::string & widgetname) :
    m_pkgid(widgetname),
    m_temp(new WidgetLocation::DirectoryDeletor(false))
{}

WidgetLocation::~WidgetLocation()
{}

WidgetLocation::WidgetLocation(const std::string & widgetname,
                               std::string sourcePath,
                               WrtDB::PackagingType t,
                               bool isReadonly,
                               InstallMode::ExtensionType eType) :
    m_pkgid(widgetname),
    m_widgetSource(sourcePath),
    m_type(t),
    m_temp(
        new WidgetLocation::DirectoryDeletor(isReadonly)),
    m_extensionType(eType)
{
    if (isReadonly) {
        m_installedPath += WrtDB::GlobalConfig::GetUserPreloadedWidgetPath();
    } else {
        m_installedPath += WrtDB::GlobalConfig::GetUserInstalledWidgetPath();
    }
    if (access(m_widgetSource.c_str(), F_OK) != 0) {
        m_widgetSource = m_installedPath + "/" + m_pkgid;
    }
}

WidgetLocation::WidgetLocation(const std::string & widgetname,
                               std::string sourcePath,
                               std::string dirPath,
                               WrtDB::PackagingType t,
                               bool isReadonly,
                               InstallMode::ExtensionType eType) :
    m_pkgid(widgetname),
    m_widgetSource(sourcePath),
    m_type(t),
    m_temp(new WidgetLocation::DirectoryDeletor(dirPath)),
    m_extensionType(eType)
{
    if (isReadonly) {
        m_installedPath += WrtDB::GlobalConfig::GetUserPreloadedWidgetPath();
    } else {
        m_installedPath += WrtDB::GlobalConfig::GetUserInstalledWidgetPath();
    }
    if (access(m_widgetSource.c_str(), F_OK) != 0) {
        m_widgetSource = m_installedPath + "/" + m_pkgid;
    }
}

// TODO cache all these paths
std::string WidgetLocation::getInstallationDir() const
{
    return m_installedPath;
}

std::string WidgetLocation::getPackageInstallationDir() const
{
    return m_installedPath + "/" + m_pkgid;
}

std::string WidgetLocation::getSourceDir() const
{
    return m_installedPath + "/"
           + m_pkgid + WrtDB::GlobalConfig::GetWidgetSrcPath();
}

std::string WidgetLocation::getBinaryDir() const
{
    return m_installedPath + "/"
           + m_pkgid + WrtDB::GlobalConfig::GetUserWidgetExecPath();
}

std::string WidgetLocation::getUserBinaryDir() const
{
    return getUserDataRootDir() + "/"
           + WrtDB::GlobalConfig::GetUserWidgetExecPath();
}

std::string WidgetLocation::getExecFile() const
{
    return getBinaryDir() + "/" + m_appid;
}

std::string WidgetLocation::getBackupDir() const
{
    return getPackageInstallationDir() + ".backup";
}

std::string WidgetLocation::getBackupSourceDir() const
{
    return getBackupDir() + WrtDB::GlobalConfig::GetWidgetSrcPath();
}

std::string WidgetLocation::getBackupBinaryDir() const
{
    return getBackupDir() + WrtDB::GlobalConfig::GetUserWidgetExecPath();
}

std::string WidgetLocation::getBackupExecFile() const
{
    return getBackupBinaryDir() + "/" + m_appid;
}

std::string WidgetLocation::getBackupPrivateDir() const
{
    return getBackupDir() + "/" +
        WrtDB::GlobalConfig::GetWidgetPrivateStoragePath();
}

std::string WidgetLocation::getUserDataRootDir() const
{
    return std::string(WrtDB::GlobalConfig::GetWidgetUserDataPath()) +
           "/" + m_pkgid;
}

std::string WidgetLocation::getPrivateStorageDir() const
{
    return getUserDataRootDir() + "/" +
           WrtDB::GlobalConfig::GetWidgetPrivateStoragePath();
}

std::string WidgetLocation::getPrivateTempStorageDir() const
{
    return getUserDataRootDir() + "/" +
           WrtDB::GlobalConfig::GetWidgetPrivateTempStoragePath();
}


std::string WidgetLocation::getTemporaryPackageDir() const
{
    return m_temp->getTempPath();
}

std::string WidgetLocation::getTemporaryRootDir() const
{
    if (m_extensionType == InstallMode::ExtensionType::DIR) {
        return getWidgetSource() + WrtDB::GlobalConfig::GetWidgetSrcPath();
    }
    return getSourceDir();
}

DPL::String WidgetLocation::getPkgId() const
{
    return DPL::FromUTF8String(m_pkgid);
}

std::string WidgetLocation::getInstalledIconPath() const
{
    return m_iconPath;
}

std::string WidgetLocation::getWidgetSource() const
{
    return m_widgetSource;
}

void WidgetLocation::setIconTargetFilenameForLocale(const std::string & icon)
{
    m_iconPath = icon;
}

void WidgetLocation::registerExternalLocation(const std::string & file)
{
    m_externals.push_back(file);
}

WrtDB::ExternalLocationList WidgetLocation::listExternalLocations() const
{
    return m_externals;
}

void WidgetLocation::registerAppid(const std::string & appid)
{
    m_appid = appid;
}

#ifdef SERVICE_ENABLED
void WidgetLocation::registerServiceAppid(const std::string & svcAppid)
{
    m_svcAppid = svcAppid;
}
#endif

std::string WidgetLocation::getSharedRootDir() const
{
    /* TODO : add wrt-commons*/
    return getUserDataRootDir() + "/shared";
}

std::string WidgetLocation::getSharedResourceDir() const
{
    return getSharedRootDir() + "/res";
}

std::string WidgetLocation::getSharedDataDir() const
{
    return getSharedRootDir() + "/data";
}

std::string WidgetLocation::getSharedTrustedDir() const
{
    return getSharedRootDir() + "/trusted";
}

std::string WidgetLocation::getBackupSharedDir() const
{
    return getBackupDir() + "/shared";
}

std::string WidgetLocation::getBackupSharedDataDir() const
{
    return getBackupSharedDir() + "/data";
}

std::string WidgetLocation::getBackupSharedTrustedDir() const
{
    return getBackupSharedDir() + "/trusted";
}

std::string WidgetLocation::getNPPluginsExecFile() const
{
    return getBinaryDir() + "/" + m_appid + ".npruntime";
}

std::string WidgetLocation::getNPPluginsDir() const
{
    return getSourceDir() + "/plugins";
}
