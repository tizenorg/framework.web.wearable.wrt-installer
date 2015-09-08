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
 * @file        widget_location.h
 * @author      Iwanek Tomasz (t.iwanek@smasung.com)
 */
#ifndef WRT_INSTALLER_SRC_MISC_WIDGET_LOCATION_H
#define WRT_INSTALLER_SRC_MISC_WIDGET_LOCATION_H

#include <string>
#include <memory>

#include <dpl/wrt-dao-ro/common_dao_types.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <wrt_common_types.h>
#include <wrt_install_mode.h>

/**
 * @brief The WidgetLocation class
 *
 * Object that stores locations of several files/directories according
 * to package name
 *
 * Current package layout (of installed package) is following:
 *
 * /opt/apps/[package_name]
 *           \_____________ /data
 *           \_____________ /share
 *           \_____________ /bin
 *           \_____________ /bin/[id_of_installed_package]
 *           \_____________ /res/wgt/
 *                               \___ config.xml
 *                               \___ [widgets_archive_content]
 *
 * 1) Normal Widget
 *  Developer provides content of res/wgt directory (package contains that
 * directory as root).
 *
 * 2) For OSP Service Hybrid App is actually a bit different:
 *  Root is OSP Service directory, WebApp content is located in [root]/res/wgt
 *
 * Temporary directory is directory when widget is placed at the begining
 * of installation process. After parsing process of config.xml, destination
 * directory is created.
 */
class WidgetLocation
{
    class DirectoryDeletor
    {
      public:
        DirectoryDeletor();
        DirectoryDeletor(std::string tempPath);
        DirectoryDeletor(bool isPreload);

        ~DirectoryDeletor();
        std::string getTempPath() const;

      private:
        std::string m_dirpath;
    };

  public:
    DECLARE_EXCEPTION_TYPE(DPL::Exception, Base)
    DECLARE_EXCEPTION_TYPE(Base, NoTemporaryPath)
    /**
     * @brief WidgetLocation
     *
     * Creates useless object. Needed by optional type
     */
    WidgetLocation();
    /**
     * @brief WidgetLocation Builds paths for widget location during
     * uninstallation
     *
     * Uninstallation process needs only installed package directory.
     *
     * @param widgetname name of widget
     */
    explicit WidgetLocation(const std::string & widgetname);
    /**
     * @brief WidgetLocation Builds paths for widget location during
     * installation
     *
     * @param widgetname name of widget
     * @param sourcePath given source path
     * @param t declaraced type of widget if type is needed
     *
     * In destruction removes temporary directory
     */
    WidgetLocation(const std::string & widgetname, std::string sourcePath,
                   WrtDB::PackagingType t = WrtDB::PKG_TYPE_NOMAL_WEB_APP,
                   bool isReadonly = false,
                   InstallMode::ExtensionType eType =
                   InstallMode::ExtensionType::WGT);

    WidgetLocation(const std::string & widgetname, std::string sourcePath,
                   std::string dirPath,
                   WrtDB::PackagingType t = WrtDB::PKG_TYPE_NOMAL_WEB_APP,
                   bool isReadonly = false,
                   InstallMode::ExtensionType eType =
                   InstallMode::ExtensionType::WGT);

    ~WidgetLocation();

    // Installed paths
    std::string getInstallationDir() const; // /opt/apps or /usr/apps
    std::string getPackageInstallationDir() const; // /opt/apps/[package]
    std::string getSourceDir() const;  // /opt/apps/[package]/res/wgt
    std::string getBinaryDir() const;  // /opt/apps/[package]/bin or /usr/apps/[package]/bin
    std::string getUserBinaryDir() const;  // /opt/apps/[package]/bin
    std::string getExecFile() const;   // /opt/apps/[package]/bin/[package]
    std::string getBackupDir() const;  // /opt/apps/[package].backup
    std::string getBackupSourceDir() const; // /opt/apps/[pkg].backup/res/wgt
    std::string getBackupBinaryDir() const; // /opt/apps/[pkg].backup/bin
    std::string getBackupExecFile() const;  // /opt/apps/[pkg].backup/bin/[pkg]
    std::string getBackupPrivateDir() const;  // /opt/apps/[pkg].backup/data
    std::string getUserDataRootDir() const; // /opt/usr/apps/[package]
    std::string getPrivateStorageDir() const; // /opt/usr/apps/[package]/data
    std::string getPrivateTempStorageDir() const; // /opt/usr/apps/[package]/tmp
    std::string getSharedRootDir() const; // /opt/usr/apps/[package]/shared
    std::string getSharedResourceDir() const; // /opt/usr/apps/[package]/shared/res
    std::string getSharedDataDir() const; // /opt/usr/apps/[package]/shared/data
    std::string getSharedTrustedDir() const; // /opt/usr/apps/[package]/shared/trusted
    std::string getBackupSharedDir() const; // /opt/usr/apps/[package].backup/shared
    std::string getBackupSharedDataDir() const; // /opt/usr/apps/[package].backup/shared/data
    std::string getBackupSharedTrustedDir() const; // /opt/usr/apps/[package].backup/shared/trusted
    std::string getNPPluginsDir() const; // /opt/usr/apps/[package]/res/wgt/plugins
    std::string getNPPluginsExecFile() const; // /opt/usr/apps/[package]/bin/{execfile}

    // Temporary paths
    /**
     * @brief getTemporaryRootDir
     * @return value of root for developer's provide package (root of unpacked
     * .wgt file)
     */
    std::string getTemporaryPackageDir() const;
    /**
     * @brief getTemporaryRootDir
     *
     * Value of this will differs according to type of installed widget.
     *
     * @return value of root for content in temporary directory to be copied
     * into 'res/wgt'
     */
    std::string getTemporaryRootDir() const;

    //icons
    /**
     * @brief setIconTargetFilenameForLocale set installed ion path according to
     * locale
     * @param icon path of application icon
     */
    void setIconTargetFilenameForLocale(const std::string &icon);

    /**
     * @brief getIconTargetFilename gets icon full path
     * @param languageTag language tag
     * @return value of full path
     */
    std::string getInstalledIconPath() const;

    /**
     * @brief getWidgetSourcePath return widget's source path given to installer
     * @return value of source path
     */
    std::string getWidgetSource() const;
    /**
     * @brief pkgid Returns pkgid
     * @return pkgid
     */
    DPL::String getPkgId() const;

    //external files
    /**
     * @brief registerExternalFile Registers file for database registration
     * @param file
     *
     * Registered file will be stored in database and removed automatically a
     *
     * @return
     */
    void registerExternalLocation(const std::string & file);
    /**
     * @brief listExternalFile list all file to be registered
     */
    WrtDB::ExternalLocationList listExternalLocations() const;

    /*
     * @brief set appid
     */
    void registerAppid(const std::string & appid);
#ifdef SERVICE_ENABLED
    void registerServiceAppid(const std::string & svcAppid);
#endif

  private:
    std::string m_pkgid;                        //id of package
    std::string m_widgetSource;                   // Source widget zip
                                                  // file/widget url
    std::string m_appid;                        //id of app
#ifdef SERVICE_ENABLED
    std::string m_svcAppid;
#endif
    std::string m_iconPath;                       //installed icon path
    WrtDB::PackagingType m_type;
    std::shared_ptr<DirectoryDeletor> m_temp;      //directory
    WrtDB::ExternalLocationList m_externals;
    std::string m_installedPath;
    InstallMode::ExtensionType m_extensionType;
};

#endif // WRT_INSTALLER_SRC_MISC_WIDGET_LOCATION_H
