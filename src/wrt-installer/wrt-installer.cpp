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
/* @file    wrt-installer.cpp
 * @version 1.0
 * @brief   Implementation file for installer
 */

#include "wrt-installer.h"
#include "plugin_utils.h"

#include <cstdlib>
#include <cstring>
#include <map>

#include <device/power.h>
#include <dirent.h>
#include <sys/resource.h>

#include <libxml/parser.h>
#include <dpl/log/log.h>
#include <dpl/optional_typedefs.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/wrt-dao-ro/config_parser_data.h>
#include <dpl/abstract_waitable_input_adapter.h>
#include <dpl/abstract_waitable_output_adapter.h>
#include <dpl/binary_queue.h>
#include <dpl/copy.h>
#include <dpl/errno_string.h>
#include <dpl/localization/w3c_file_localization.h>
#include <dpl/optional_typedefs.h>
#include <dpl/utils/widget_version.h>
#include <dpl/utils/wrt_global_settings.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/wrt-dao-ro/config_parser_data.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/wrt-dao-ro/WrtDatabase.h>
#include <dpl/zip_input.h>

#include <wrt-commons/i18n-dao-ro/i18n_database.h>

#include <vcore/VCore.h>

#include <Elementary.h>

#include <installer_callbacks_translate.h>
#include <installer_controller.h>
#include <installer_log.h>
#include <installer_main_thread.h>
#include <language_subtag_rst_tree.h>
#include <parser_runner.h>
#include <pkg-manager/pkgmgr_signal_dummy.h>
#include <pkgmgr-info.h>
#include <root_parser.h>
#include <widget_parser.h>
#include <wrt_install_mode.h>
#include <command_parser.h>

using namespace WrtDB;

namespace { // anonymous
const char * const PKGMGR_INSTALL_MSG = "Install widget";
const char * const PKGMGR_UNINSTALL_MSG = "Uninstall widget";

const char * const CONFIG_XML = "config.xml";
const char * const HYBRID_CONFIG_XML = "res/wgt/config.xml";

const unsigned int NOFILE_CNT_FOR_INSTALLER = 9999;

struct free_deleter
{
    void operator()(void* x)
    {
        free(x);
    }
};

struct PluginInstallerData
{
    void* wrtInstaller;
    std::string pluginPath;
};

std::string cutOffFileName(const std::string& path)
{
    size_t found = path.find_last_of("/");
    if (found == std::string::npos) {
        return path;
    } else {
        return path.substr(0, found);
    }
}

bool checkPath(const std::string& path)
{
    struct stat st;
    if (0 == stat(path.c_str(), &st) && S_ISDIR(st.st_mode)) {
        return true;
    }
    _E("Cannot access directory [ %s ]", path.c_str());
    return false;
}

bool checkPaths()
{
    bool if_ok = true;

    if_ok &= (checkPath(cutOffFileName(GlobalConfig::GetWrtDatabaseFilePath())));
    if_ok &= (checkPath(GlobalConfig::GetDevicePluginPath()));
    if_ok &= (checkPath(GlobalConfig::GetUserInstalledWidgetPath()));
    if_ok &= (checkPath(GlobalConfig::GetUserPreloadedWidgetPath()));

    return if_ok;
}

} // namespace anonymous

WrtInstaller::WrtInstaller(int argc, char **argv) :
    Application(argc, argv, "backend", false),
    DPL::TaskDecl<WrtInstaller>(this),
    m_packagePath(),
    m_initialized(false),
    m_numPluginsToInstall(0),
    m_totalPlugins(0),
    m_returnStatus(-1),
    m_sendPkgSig(false),
    m_startupPluginInstallation(false)
{
    Touch();
    _D("App Created");
}

WrtInstaller::~WrtInstaller()
{
    _D("App Finished");
}

int WrtInstaller::getReturnStatus() const
{
    return m_returnStatus;
}

void WrtInstaller::OnStop()
{
    _D("Stopping Dummy Client");
}

void WrtInstaller::OnCreate()
{
    _D("Creating DummyClient");

    //pm lock
    device_power_request_lock(POWER_LOCK_CPU, 0);

    showArguments();

    AddStep(&WrtInstaller::initStep);

    std::string arg = m_argv[0];

    using namespace PackageManager;

    auto pkgmgrSignal = std::shared_ptr<PackageManager::PkgmgrSignal>(new PackageManager::PkgmgrSignal());

    pkgmgrSignalInterface =
        std::static_pointer_cast<PackageManager::IPkgmgrSignal>(
                std::shared_ptr<PackageManager::PkgmgrSignalDummy>(
                    new PackageManager::PkgmgrSignalDummy()));

    if (arg.empty()) {
        return showHelpAndQuit();
    }

    installNewPlugins();

    if (arg.find("wrt-installer") != std::string::npos) {
        if (m_argc <= 1) {
            return showHelpAndQuit();
        }

        arg = m_argv[1];
        if (arg == "-h" || arg == "--help") {
            if (m_argc != 2) {
                return showHelpAndQuit();
            }

            // Just show help
            return showHelpAndQuit();
        } else if (arg == "-p" || arg == "--install-plugins") {
            if (m_argc != 2) {
                return showHelpAndQuit();
            }

            if (!m_startupPluginInstallation) {
                AddStep(&WrtInstaller::installPluginsStep);
            } else {
                _D("Plugin installation alredy started");
            }
        } else if (arg == "-i" || arg == "--install") {
            if (m_argc != 3) {
                return showHelpAndQuit();
            }

            struct stat info;
            if (-1 != stat(m_argv[2], &info) && S_ISDIR(info.st_mode)) {
                _D("Installing package directly from directory");
                m_installMode.extension = InstallMode::ExtensionType::DIR;
            } else {
                _D("Installing from regular location");
                m_installMode.extension = InstallMode::ExtensionType::WGT;
            }
            m_packagePath = m_argv[2];
            m_sendPkgSig = true;
            pkgmgrSignal->initialize(m_argc, m_argv);
            pkgmgrSignalInterface = std::static_pointer_cast<PackageManager::IPkgmgrSignal>(pkgmgrSignal);
            AddStep(&WrtInstaller::installStep);
        } else if (arg == "-ip" || arg == "--install-preload") {
            _D("Install preload web app");
            if (m_argc != 3) {
                return showHelpAndQuit();
            }
            m_packagePath = m_argv[2];
            m_installMode.installTime = InstallMode::InstallTime::PRELOAD;
            m_installMode.rootPath = InstallMode::RootPath::RO;
            m_installMode.removable = false;
            AddStep(&WrtInstaller::installStep);
        } else if (arg == "-ipw" || arg == "--install-preload-writable") {
            _D("Install preload web application to writable storage");
            if (m_argc != 3) {
                return showHelpAndQuit();
            }
            m_packagePath = m_argv[2];
            m_installMode.installTime = InstallMode::InstallTime::PRELOAD;
            m_installMode.rootPath = InstallMode::RootPath::RW;
            m_installMode.removable = true;
            AddStep(&WrtInstaller::installStep);
        } else if (arg == "-c" || arg == "--csc-update") {
            // "path=/opt/system/csc/Ozq2iEG15R-2.0.0-arm.wgt:op=install:removable=true"
            _D("Install & uninstall by csc configuration");
            if (m_argc != 3) {
                return showHelpAndQuit();
            }
            m_installMode.installTime = InstallMode::InstallTime::CSC;
            std::string configuration = m_argv[2];

            CommandParser::CscOption option;
            if (!CommandParser::CscCommandParser(configuration, option)) {
                return showHelpAndQuit();
            }

            if (0 == option.operation.compare(Command::VALUE_INSTALL)) {
                m_installMode.extension = InstallMode::ExtensionType::WGT;
                m_packagePath = option.path;
                m_installMode.removable = option.removable;
                m_installMode.cscPath = m_argv[2];
                _D("operation = %s", option.operation.c_str());
                _D("path      = %s", m_packagePath.c_str());
                _D("removable = %d", m_installMode.removable);
                _D("csc Path  = %s", m_installMode.cscPath.c_str());
                AddStep(&WrtInstaller::installStep);
            } else if (0 == option.operation.compare(Command::VALUE_UNINSTALL)){
                _D("operation = %s", option.operation.c_str());
                _D("path      = %s", option.path.c_str());
                m_name = option.path;
                AddStep(&WrtInstaller::uninstallPkgNameStep);
            } else {
                _E("Unknown operation : %s", option.operation.c_str());
                return showHelpAndQuit();
            }
        } else if (arg == "-un" || arg == "--uninstall-name") {
            if (m_argc != 3) {
                return showHelpAndQuit();
            }
            m_name = m_argv[2];
            m_sendPkgSig = true;
            m_argv[1] = (char*)"-d";
            pkgmgrSignal->initialize(m_argc, m_argv);
            pkgmgrSignalInterface = std::static_pointer_cast<PackageManager::IPkgmgrSignal>(pkgmgrSignal);
            AddStep(&WrtInstaller::uninstallPkgNameStep);
        } else if (arg == "-up" || arg == "--uninstall-packagepath") {
            if (m_argc != 3) {
                return showHelpAndQuit();
            }
            m_packagePath = m_argv[2];
            AddStep(&WrtInstaller::unistallWgtFileStep);
        } else if (arg == "-r" || arg == "--reinstall") {
            if (m_argc != 3) {
                return showHelpAndQuit();
            }
            _D("Installing package directly from directory");
            m_installMode.command = InstallMode::Command::REINSTALL;
            m_installMode.extension = InstallMode::ExtensionType::DIR;
            m_packagePath = m_argv[2];
            m_sendPkgSig = true;
            pkgmgrSignal->initialize(m_argc, m_argv);
            pkgmgrSignalInterface = std::static_pointer_cast<PackageManager::IPkgmgrSignal>(pkgmgrSignal);
            AddStep(&WrtInstaller::installStep);
        } else if (arg == "-f" || arg == "--fota") {
            // "path=8HPzsUYyNZ:op=install"
            _D("Install & uninstall by fota");
            if (m_argc != 3) {
                return showHelpAndQuit();
            }
            std::string configuration = m_argv[2];
            CommandParser::FotaOption option;
            if (!CommandParser::FotaCommandParser(configuration, option)) {
                return showHelpAndQuit();
            }

            if ((0 == option.operation.compare(Command::VALUE_INSTALL)) ||
                    (0 == option.operation.compare(Command::VALUE_UPGRADE)) ||
                    (0 == option.operation.compare(Command::VALUE_UPDATE))) {
                _D("FOTA ... Installation");
                m_name = option.pkgId;
                m_packagePath +=
                    std::string(WrtDB::GlobalConfig::GetUserPreloadedWidgetPath())
                    + "/" + option.pkgId;
                _D("package id   = %s", m_name.c_str());
                _D("operation    = %s", option.operation.c_str());
                _D("package path = %s", m_packagePath.c_str());

                m_installMode.installTime = InstallMode::InstallTime::FOTA;
                m_installMode.rootPath = InstallMode::RootPath::RO;
                m_installMode.extension = InstallMode::ExtensionType::DIR;
                AddStep(&WrtInstaller::installStep);
            } else if (0 == option.operation.compare(Command::VALUE_UNINSTALL)){
                _D("FOTA ... Uninstallation");
                m_name = option.pkgId;
                _D("package id   = %s", m_name.c_str());
                AddStep(&WrtInstaller::uninstallPkgNameStep);
            } else {
                _E("Unknown operation : %s", option.operation.c_str());
                return showHelpAndQuit();
            }
        } else if (arg == "-b" || arg == "--recovery") {
            getRecoveryPackageId(m_name);
            _D("m_name : %s", m_name.c_str());

            if (!m_name.empty()) {
                pkgmgrinfo_pkginfo_h handle = NULL;
                if (0 == pkgmgrinfo_pkginfo_get_pkginfo(m_name.c_str(), &handle)) {
                    m_installMode.command = InstallMode::Command::RECOVERY;
                    m_installMode.extension = InstallMode::ExtensionType::DIR;
                    AddStep(&WrtInstaller::installStep);
                } else {
                    _D("package id   = %s", m_name.c_str());
                    AddStep(&WrtInstaller::uninstallPkgNameStep);
                }
            }
        } else {
            return showHelpAndQuit();
        }
    } else if (arg.find("backend") != std::string::npos) {
        m_sendPkgSig = true;
        pkgmgrSignal->initialize(m_argc, m_argv);
        PkgmgrSignal::RequestType reqType = pkgmgrSignal->getRequestedType();
        pkgmgrSignalInterface = std::static_pointer_cast<PackageManager::IPkgmgrSignal>(pkgmgrSignal);

        switch (reqType) {
        case PackageManager::PkgmgrSignal::RequestType::INSTALL:
            m_packagePath = m_argv[4];
            if (6 < m_argc) {
                m_name = std::string(m_argv[6]);
            }

            struct stat info;
            if (-1 != stat(m_argv[4], &info) && S_ISDIR(info.st_mode)) {
                _D("Installing package directly from directory");
                m_installMode.extension = InstallMode::ExtensionType::DIR;
            } else {
                _D("Installing from regular location");
                m_installMode.extension = InstallMode::ExtensionType::WGT;
            }
            AddStep(&WrtInstaller::installStep);
            break;
        case PackageManager::PkgmgrSignal::RequestType::UNINSTALL:
        {
            m_name = m_argv[4];
            pkgmgrinfo_pkginfo_h handle = NULL;
            bool preload = false;
            bool system = false;
            bool removable = true;
            bool update = false;
            char *cscPath = NULL;

            if (0 == pkgmgrinfo_pkginfo_get_pkginfo(m_name.c_str(), &handle)) {
                if (0 > pkgmgrinfo_pkginfo_is_preload(handle, &preload)) {
                    _E("Can't get package information : %s", m_name.c_str());
                }
                if (0 > pkgmgrinfo_pkginfo_is_system(handle, &system)) {
                    _E("Can't get package information : %s", m_name.c_str());
                }
                if (0 > pkgmgrinfo_pkginfo_is_removable(handle, &removable)) {
                    _E("Can't get package information : %s", m_name.c_str());
                }
                if (0 > pkgmgrinfo_pkginfo_is_update(handle, &update)) {
                    _E("Can't get package information about update : %s", m_name.c_str());
                }
                if (0 > pkgmgrinfo_pkginfo_get_csc_path(handle, &cscPath)) {
                    _E("Can't get package information about update : %s", m_name.c_str());
                }
            }

            _D("preload app : %d", preload);
            _D("system app : %d", system);
            _D("removable app : %d", removable);
            _D("update : %d", update);
            _D("csc path : %s", cscPath);

            if (preload && update) {
                if (setInitialCSC(cscPath)) {
                    if (system) {
                        AddStep(&WrtInstaller::uninstallPkgNameStep);
                        AddStep(&WrtInstaller::installStep);
                    } else {
                        AddStep(&WrtInstaller::uninstallPkgNameStep);
                    }
                } else if (system) {
                    AddStep(&WrtInstaller::removeUpdateStep);
                } else if (removable) {
                    AddStep(&WrtInstaller::uninstallPkgNameStep);
                }
            } else {
                AddStep(&WrtInstaller::uninstallPkgNameStep);
            }
            pkgmgrinfo_pkginfo_destroy_pkginfo(handle);
            break;
        }
        case PackageManager::PkgmgrSignal::RequestType::REINSTALL:
            m_packagePath = m_argv[4];
            m_installMode.command = InstallMode::Command::REINSTALL;
            m_installMode.extension = InstallMode::ExtensionType::DIR;
            AddStep(&WrtInstaller::installStep);
            break;
        default:
            _D("Not available type");
            break;
        }
    }

    AddStep(&WrtInstaller::shutdownStep);
    DPL::Event::ControllerEventHandler<WRTInstallerNS::NextStepEvent>::
        PostEvent(
        WRTInstallerNS::NextStepEvent());
}

void WrtInstaller::OnReset(bundle* /*b*/)
{
    _D("OnReset");
}

void WrtInstaller::OnTerminate()
{
    _D("Wrt Shutdown now");

    //pm unlock
    device_power_release_lock(POWER_LOCK_CPU);

    PluginUtils::unlockPluginInstallation(
        m_installMode.installTime == InstallMode::InstallTime::PRELOAD);
    if (m_initialized) {
        try {
            _D("DEINITIALIZING WRT INSTALLER...");
            // Installer termination
            CONTROLLER_POST_SYNC_EVENT(
                Logic::InstallerController,
                InstallerControllerEvents::
                    TerminateEvent());
            InstallerMainThreadSingleton::Instance().DetachDatabases();
            I18n::DB::Interface::detachDatabase();

            // This must be done after DetachDatabase
            ValidationCore::VCoreDeinit();

            // Global deinit check
            _D("Cleanup libxml2 global values.");
            xmlCleanupParser();
        } catch (const DPL::Exception& ex) {
            _E("Internal Error during Shutdown:");
            DPL::Exception::DisplayKnownException(ex);
        }
    }
}

void WrtInstaller::showHelpAndQuit()
{
    printf("Usage: wrt-installer [OPTION]... [WIDGET: ID/NAME/PATH]...\n"
           "Operate with WebRuntime daemon: install, uninstall"
           " and launch widgets.\n"
           "Query list of installed widgets and setup up debugging support.\n"
           "\n"
           "Exactly one option must be selected.\n"
           "Mandatory arguments to long options are mandatory for short "
           "options too.\n"
           "  -h,    --help                                 show this help\n"
           "  -p,    --install-plugins                      install plugins\n"
           "  -i,    --install                              "
           "install or update widget package for given path\n"
           "  -c,    --csc-update                           "
           "install or uninstall by CSC configuration \n"
           "  -un,   --uninstall-name                       "
           "uninstall widget for given package name\n"
           "  -up,   --uninstall-packagepath                "
           "uninstall widget for given package file path\n"
           "  -r,    --reinstall                            "
           "reinstall mode for sdk (this is NOT normal reinstallation/update)\n"
           "\n");

    Quit();
}

void WrtInstaller::showArguments()
{
    fprintf(stderr,
            "===========================================================\n");
    fprintf(stderr, "# wrt-installer #\n");
    fprintf(stderr, "# argc [%d]\n", m_argc);
    for (int i = 0; i < m_argc; i++) {
        fprintf(stderr, "# argv[%d] = [%s]\n", i, m_argv[i]);
    }
    fprintf(stderr,
            "===========================================================\n");
    // for dlog
    _D("===========================================================");
    _D("# wrt-installer #");
    _D("# argc %d", m_argc);
    for (int i = 0; i < m_argc; i++) {
        _D("# argv[%d] = %s", i, m_argv[i]);
    }
    _D("===========================================================");

}

void WrtInstaller::OnEventReceived(const WRTInstallerNS::QuitEvent& /*event*/)
{
    _D("Quiting");

    if (m_initialized) {
        _D("Wrt Shutdown now");
        SwitchToStep(&WrtInstaller::shutdownStep);
        DPL::Event::ControllerEventHandler<WRTInstallerNS::NextStepEvent>::
            PostEvent(
            WRTInstallerNS::NextStepEvent());
    } else {
        _D("Quiting application");
        return Quit();
    }
}

void WrtInstaller::OnEventReceived(
    const WRTInstallerNS::NextStepEvent& /*event*/)
{
    _D("Executing next step");
    NextStep();
}

void WrtInstaller::OnEventReceived(
    const WRTInstallerNS::InstallPluginEvent& /*event*/)
{
    PluginInstallerData* privateData = new PluginInstallerData;
    privateData->wrtInstaller = this;

    if (!(*m_pluginsPaths).empty()) {
        privateData->pluginPath = (*m_pluginsPaths).front();
        (*m_pluginsPaths).pop_front();

        _D("INSTALL PLUGIN: %s", privateData->pluginPath.c_str());
        //Private data for status callback
        //Resource is free in pluginInstallFinishedCallback
        InstallerCallbacksTranslate::PluginStatusCallbackStruct*
        callbackStruct =
            new InstallerCallbacksTranslate::PluginStatusCallbackStruct(
                privateData, &staticWrtPluginInstallationCallback, &staticWrtPluginInstallProgressCb);

        CONTROLLER_POST_EVENT(
            Logic::InstallerController,
            InstallerControllerEvents::InstallPluginEvent(
                privateData->pluginPath,
                PluginInstallerStruct(
                    InstallerCallbacksTranslate::
                        pluginInstallFinishedCallback,
                    InstallerCallbacksTranslate::
                        installProgressCallback, callbackStruct)));
    } else {
        delete privateData;
    }
}

void WrtInstaller::initStep()
{
    try {
        _D("INITIALIZING WRT INSTALLER...");

        // Touch InstallerController Singleton
        InstallerMainThreadSingleton::Instance().TouchArchitecture();

        // Check paths
        if (!checkPaths()) {
            makeStatusOfWrtInit(WRT_INSTALLER_ERROR_FATAL_ERROR);
            return;
        }

        // Initialize ValidationCore - this must be done before AttachDatabases
        ValidationCore::VCoreInit(
            std::string(GlobalConfig::GetFingerprintListFile()),
            std::string(GlobalConfig::GetFingerprintListSchema()),
            std::string(GlobalConfig::GetVCoreDatabaseFilePath()));

        InstallerMainThreadSingleton::Instance().AttachDatabases();

        _D("Prepare libxml2 to work in multithreaded program.");
        xmlInitParser();

        // Initialize Language Subtag registry
        LanguageSubtagRstTreeSingleton::Instance().Initialize();

        // Installer init
        CONTROLLER_POST_SYNC_EVENT(
            Logic::InstallerController,
            InstallerControllerEvents::
                InitializeEvent());

        makeStatusOfWrtInit(WRT_SUCCESS);
    } catch (const DPL::Exception& ex) {
        _E("Internal Error during Init:");
        DPL::Exception::DisplayKnownException(ex);
        makeStatusOfWrtInit(WRT_INSTALLER_ERROR_FATAL_ERROR);
    }
}

void WrtInstaller::installStep()
{
    std::unique_ptr<char, free_deleter> packagePath(canonicalize_file_name(
                                                        m_packagePath.c_str()));

    if (InstallMode::InstallTime::PRELOAD == m_installMode.installTime) {
        DPL::Log::OldStyleLogProvider *oldStyleProvider =
            new DPL::Log::OldStyleLogProvider(false, false, false, true,
                    false, true);
        DPL::Log::LogSystemSingleton::Instance().AddProvider(oldStyleProvider);
    }

    std::string path = packagePath ? packagePath.get() : m_packagePath.c_str();
    _D("INSTALL WIDGET: %s", path.c_str());
    // Post installation event
    CONTROLLER_POST_EVENT(
        Logic::InstallerController,
        InstallerControllerEvents::InstallWidgetEvent(
            path, m_name.c_str(),
            Jobs::WidgetInstall::WidgetInstallationStruct(
                InstallerCallbacksTranslate::installFinishedCallback,
                InstallerCallbacksTranslate::installProgressCallback,
                new InstallerCallbacksTranslate::StatusCallbackStruct(
                    this, &staticWrtStatusCallback, (m_sendPkgSig)
                            ? &staticWrtInstallProgressCallback : NULL),
                m_installMode, pkgmgrSignalInterface)));
}

void WrtInstaller::installPluginsStep()
{
    _D("Installing plugins ...");
    fprintf(stderr, "Installing plugins ...\n");

    if (m_startupPluginInstallation) {
        _D("Plugin installation started because new plugin package found");
    } else if (!PluginUtils::lockPluginInstallation(
        m_installMode.installTime == InstallMode::InstallTime::PRELOAD))
    {
        _E("Failed to open plugin installation lock file"
                 " Plugins are currently installed by other process");
        staticWrtPluginInstallationCallback(WRT_INSTALLER_ERROR_PLUGIN_INSTALLATION_FAILED,
                                            this);
        return;
    }

    std::string PLUGIN_PATH = std::string(GlobalConfig::GetDevicePluginPath());

    DIR *dir;
    dir = opendir(PLUGIN_PATH.c_str());

    if (!dir) {
        return;
    }

    _D("Plugin DIRECTORY IS %s", PLUGIN_PATH.c_str());

    std::list<std::string> pluginsPaths;
    struct dirent libdir;
    struct dirent *result;
    int return_code;
    errno = 0;
    for (return_code = readdir_r(dir, &libdir, &result);
            result != NULL && return_code == 0;
            return_code = readdir_r(dir, &libdir, &result))
    {
        if (strcmp(libdir.d_name, ".") == 0 ||
            strcmp(libdir.d_name, "..") == 0)
        {
            continue;
        }

        std::string path = PLUGIN_PATH;
        path += "/";
        path += libdir.d_name;

        struct stat tmp;

        if (stat(path.c_str(), &tmp) == -1) {
            _E("Failed to open file %s", path.c_str());
            continue;
        }

        if (!S_ISDIR(tmp.st_mode)) {
            _E("Not a directory %s", path.c_str());
            continue;
        }

        pluginsPaths.push_back(path);
    }

    if (return_code != 0 || errno != 0) {
        _E("readdir_r() failed with %s", DPL::GetErrnoString().c_str());
    }

    //set nb of plugins to install
    //this value indicate how many callbacks are expected
    m_numPluginsToInstall = pluginsPaths.size();
    _D("Plugins to install: %d", m_numPluginsToInstall);
    m_pluginsPaths = pluginsPaths;

    m_totalPlugins = m_numPluginsToInstall;
    DPL::Event::ControllerEventHandler<WRTInstallerNS::InstallPluginEvent>
        ::PostEvent(WRTInstallerNS::InstallPluginEvent());

    if (-1 == closedir(dir)) {
        _E("Failed to close dir: %s with error: %s", PLUGIN_PATH.c_str(), DPL::GetErrnoString().c_str());
    }
}

void WrtInstaller::uninstallPkgNameStep()
{
    _D("Package name : %s", m_name.c_str());

    _D("UNINSTALL WIDGET: %s", m_name.c_str());
    // Post uninstallation event
    CONTROLLER_POST_EVENT(
        Logic::InstallerController,
        InstallerControllerEvents::UninstallWidgetEvent(
            m_name,
            WidgetUninstallationStruct(
                InstallerCallbacksTranslate::uninstallFinishedCallback,
                InstallerCallbacksTranslate::installProgressCallback,
                new InstallerCallbacksTranslate::StatusCallbackStruct(
                    this, &staticWrtStatusCallback,
                    (m_sendPkgSig) ? &staticWrtUninstallProgressCallback : NULL),
                pkgmgrSignalInterface)
            )
        );
}

void WrtInstaller::removeUpdateStep()
{
    _D("This web app need to initialize preload app");
    _D("Package name : %s", m_name.c_str());

    _D("UNINSTALL WIDGET: %s", m_name.c_str());
    // Post uninstallation event
    CONTROLLER_POST_EVENT(
        Logic::InstallerController,
        InstallerControllerEvents::UninstallWidgetEvent(
            m_name,
            WidgetUninstallationStruct(
                InstallerCallbacksTranslate::uninstallFinishedCallback,
                InstallerCallbacksTranslate::installProgressCallback,
                new InstallerCallbacksTranslate::StatusCallbackStruct(
                    this, &staticWrtInitializeToPreloadCallback, (m_sendPkgSig)
                            ? &staticWrtUninstallProgressCallback : NULL),
                pkgmgrSignalInterface
                )
            )
        );
}

bool WrtInstaller::setInitialCSC(std::string cscPath)
{
    _D("This web app need to initialize initial csc app");
    _D("UNINSTALL WIDGET: %s", m_name.c_str());
    _D("csc path: %s", cscPath.c_str());
    char* originPath = strdup(cscPath.c_str());
    if (originPath == NULL) {
        _E("strdup() is failed");
        return false;
    }

    m_installMode.installTime = InstallMode::InstallTime::CSC;

    CommandParser::CscOption option;
    if (!CommandParser::CscCommandParser(cscPath, option)) {
        _E("Failure command parser");
        free(originPath);
        return false;
    }

    if (0 == option.operation.compare(Command::VALUE_INSTALL)) {
        m_installMode.extension = InstallMode::ExtensionType::WGT;
        m_packagePath = option.path;
        m_installMode.removable = option.removable;
        m_installMode.cscPath = originPath;
        _D("operation = %s", option.operation.c_str());
        _D("path      = %s", m_packagePath.c_str());
        _D("removable      = %d", m_installMode.removable);
        _D("csc Path = %s", m_installMode.cscPath.c_str());
    } else {
        _E("Unknown operation : %s", option.operation.c_str());
        free(originPath);
        return false;
    }
    free(originPath);
    return true;
}

void WrtInstaller::unistallWgtFileStep()
{
    _D("Uninstalling widget ...");

    Try {
        // Parse config
        ParserRunner parser;
        ConfigParserData configInfo;

        // Open zip file
        std::unique_ptr<DPL::ZipInput> zipFile(
            new DPL::ZipInput(m_packagePath));
        std::unique_ptr<DPL::ZipInput::File> configFile;

        Try {
            // Open config.xml file
            configFile.reset(zipFile->OpenFile(CONFIG_XML));
        }
        Catch(DPL::ZipInput::Exception::OpenFileFailed)
        {
            // Open config.xml file for hybrid
            configFile.reset(zipFile->OpenFile(HYBRID_CONFIG_XML));
        }

        // Extract config
        DPL::BinaryQueue buffer;
        DPL::AbstractWaitableInputAdapter inputAdapter(configFile.get());
        DPL::AbstractWaitableOutputAdapter outputAdapter(&buffer);
        DPL::Copy(&inputAdapter, &outputAdapter);
        parser.Parse(&buffer,
                     ElementParserPtr(
                         new RootParser<WidgetParser>(configInfo,
                                                      DPL::FromUTF32String(
                                                          L"widget"))));

        DPL::OptionalString pkgId = configInfo.tizenPkgId;
        if (!!pkgId) {
            _D("Pkgid from packagePath : %ls", (*pkgId).c_str());
            _D("UNINSTALL WIDGET: %s", DPL::ToUTF8String(*pkgId).c_str());
            // Post uninstallation event
            CONTROLLER_POST_EVENT(
                Logic::InstallerController,
                InstallerControllerEvents::UninstallWidgetEvent(
                    DPL::ToUTF8String(*pkgId),
                    WidgetUninstallationStruct(
                        InstallerCallbacksTranslate::uninstallFinishedCallback,
                        InstallerCallbacksTranslate::installProgressCallback,
                        new InstallerCallbacksTranslate::StatusCallbackStruct(
                            this, &staticWrtStatusCallback, !m_sendPkgSig
                            ? &staticWrtUninstallProgressCallback : NULL),
                            pkgmgrSignalInterface)
                    )
                );

        } else {
            _E("Fail to uninstalling widget... ");
            m_returnStatus = -1;
            DPL::Event::ControllerEventHandler<WRTInstallerNS::QuitEvent>::
                PostEvent(
                WRTInstallerNS::QuitEvent());
        }
    }
    Catch(DPL::ZipInput::Exception::OpenFailed)
    {
        _E("Failed to open widget package");
        printf("failed: widget package does not exist\n");
        m_returnStatus = -1;
        DPL::Event::ControllerEventHandler<WRTInstallerNS::QuitEvent>::
            PostEvent(
            WRTInstallerNS::QuitEvent());
    }
    Catch(DPL::ZipInput::Exception::OpenFileFailed)
    {
        printf("failed: widget config file does not exist\n");
        _E("Failed to open config.xml file");
        m_returnStatus = -1;
        DPL::Event::ControllerEventHandler<WRTInstallerNS::QuitEvent>::
            PostEvent(
            WRTInstallerNS::QuitEvent());
    }
    Catch(ElementParser::Exception::ParseError)
    {
        printf("failed: can not parse config file\n");
        _E("Failed to parse config.xml file");
        m_returnStatus = -1;
        DPL::Event::ControllerEventHandler<WRTInstallerNS::QuitEvent>::
            PostEvent(
            WRTInstallerNS::QuitEvent());
    }
    Catch(DPL::Exception)
    {
        printf("Unknown DPL exception\n");
        _E("Unknown DPL exception");
        m_returnStatus = -1;
        DPL::Event::ControllerEventHandler<WRTInstallerNS::QuitEvent>::
            PostEvent(
            WRTInstallerNS::QuitEvent());
    }
}

void WrtInstaller::shutdownStep()
{
    _D("Closing Wrt connection ...");
    if (m_initialized) {
        try {
            _D("DEINITIALIZING WRT INSTALLER...");
            // Installer termination
            CONTROLLER_POST_SYNC_EVENT(
                Logic::InstallerController,
                InstallerControllerEvents::
                    TerminateEvent());

            InstallerMainThreadSingleton::Instance().DetachDatabases();

            // This must be done after DetachDatabase
            ValidationCore::VCoreDeinit();

            // Global deinit check
            _D("Cleanup libxml2 global values.");
            xmlCleanupParser();
        } catch (const DPL::Exception& ex) {
            _E("Internal Error during Shutdown:");
            DPL::Exception::DisplayKnownException(ex);
        }
        m_initialized = false;
        DPL::Event::ControllerEventHandler<WRTInstallerNS::QuitEvent>::
            PostEvent(
            WRTInstallerNS::QuitEvent());
    }
}

void WrtInstaller::makeStatusOfWrtInit(WrtErrStatus status)
{
    if (status == WRT_SUCCESS) {
        _D("Init succesfull");
        m_initialized = true;
        m_returnStatus = 0;

        DPL::Event::ControllerEventHandler<WRTInstallerNS::NextStepEvent>
            ::PostEvent(WRTInstallerNS::NextStepEvent());
    } else {
        _E("Init unsuccesfull");
        m_returnStatus = -1;
        DPL::Event::ControllerEventHandler<WRTInstallerNS::QuitEvent>::
            PostEvent(
            WRTInstallerNS::QuitEvent());
    }
}

void WrtInstaller::staticWrtInitializeToPreloadCallback(std::string tizenId, WrtErrStatus
        status, void* userdata)
{
    WrtInstaller *This = static_cast<WrtInstaller*>(userdata);
    Assert(This);

    std::string printMsg = "uninstallation";

    if (WRT_SUCCESS != status) {
        // Failure
        _E("Step failed");
        This->m_returnStatus = 1;

        This->showErrorMsg(status, tizenId, printMsg);

        This->DPL::Event::ControllerEventHandler<WRTInstallerNS::QuitEvent>
            ::PostEvent(WRTInstallerNS::QuitEvent());
    } else {
        InstallMode mode;
        mode.extension = InstallMode::ExtensionType::DIR;
        mode.installTime = InstallMode::InstallTime::PRELOAD;
        mode.rootPath = InstallMode::RootPath::RO;
        std::string packagePath =
            std::string(WrtDB::GlobalConfig::GetUserPreloadedWidgetPath())
                + "/" + This->m_name;

        if (InstallMode::InstallTime::PRELOAD == This->m_installMode.installTime) {
            DPL::Log::OldStyleLogProvider *oldStyleProvider =
                new DPL::Log::OldStyleLogProvider(false, false, false, true,
                        false, true);
            DPL::Log::LogSystemSingleton::Instance().AddProvider(oldStyleProvider);
        }

        _D("INSTALL WIDGET: %s", packagePath.c_str());
        // Post installation event
        CONTROLLER_POST_EVENT(
            Logic::InstallerController,
            InstallerControllerEvents::InstallWidgetEvent(
                packagePath, tizenId.c_str(), Jobs::WidgetInstall::WidgetInstallationStruct(
                    InstallerCallbacksTranslate::installFinishedCallback,
                    InstallerCallbacksTranslate::installProgressCallback,
                    new InstallerCallbacksTranslate::StatusCallbackStruct(
                        This, &staticWrtInitPreloadStatusCallback, NULL),
                    mode,
                    This->pkgmgrSignalInterface)));
    }
}

void WrtInstaller::staticWrtInitPreloadStatusCallback(std::string tizenId,
                                           WrtErrStatus status,
                                           void* userdata)
{
    WrtInstaller *This = static_cast<WrtInstaller*>(userdata);
    Assert(This);

    std::string printMsg = "initialization";

    if (WRT_SUCCESS != status) {
        // Failure
        _E("Step failed");
        This->m_returnStatus = status;

        This->showErrorMsg(status, tizenId, printMsg);

        This->DPL::Event::ControllerEventHandler<WRTInstallerNS::QuitEvent>
            ::PostEvent(WRTInstallerNS::QuitEvent());
    } else {
        fprintf(stderr,
                "## wrt-installer : %s %s was successful.\n",
                tizenId.c_str(),
                printMsg.c_str());
        _D("Status succesfull");
        This->m_returnStatus = 0;

        This->DPL::Event::ControllerEventHandler<WRTInstallerNS::
                                                     NextStepEvent>
            ::PostEvent(WRTInstallerNS::NextStepEvent());
    }
}

void WrtInstaller::staticWrtStatusCallback(std::string tizenId,
                                           WrtErrStatus status,
                                           void* userdata)
{
    WrtInstaller *This = static_cast<WrtInstaller*>(userdata);
    Assert(This);

    Step current = This->GetCurrentStep();
    std::string printMsg;

    if (current == &WrtInstaller::installStep) {
        printMsg = "installation";
    } else if (current == &WrtInstaller::uninstallPkgNameStep ||
               current == &WrtInstaller::unistallWgtFileStep)
    {
        printMsg = "uninstallation";
    }

    if (WRT_SUCCESS != status) {
        // Failure
        _E("Step failed");
        This->m_returnStatus = status;

        This->DPL::Event::ControllerEventHandler<WRTInstallerNS::QuitEvent>
            ::PostEvent(WRTInstallerNS::QuitEvent());

        This->showErrorMsg(status, tizenId, printMsg);
    } else {
        fprintf(stderr,
                "## wrt-installer : %s %s was successful.\n",
                tizenId.c_str(),
                printMsg.c_str());
        _D("Status succesfull");
        This->m_returnStatus = 0;

        if (This->m_installMode.installTime == InstallMode::InstallTime::PRELOAD &&
                !This->m_packagePath.empty())
        {
            _D("This widget is preloaded so it will be removed : %s", This->m_packagePath.c_str());
            if (!WrtUtilRemove(This->m_packagePath)) {
                _E("Failed to remove %s", This->m_packagePath.c_str());
            }
        }

        This->DPL::Event::ControllerEventHandler<WRTInstallerNS::
                                                     NextStepEvent>
            ::PostEvent(WRTInstallerNS::NextStepEvent());
    }
}

void WrtInstaller::showErrorMsg(WrtErrStatus status, std::string tizenId,
        std::string printMsg)
{
    switch (status) {
        case WRT_INSTALLER_ERROR_PACKAGE_NOT_FOUND:
            fprintf(stderr, "## wrt-installer : %s %s has failed - widget package does not exist\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_PACKAGE_INVALID:
            fprintf(stderr, "## wrt-installer : %s %s has failed - invalid widget package\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_PACKAGE_LOWER_VERSION:
            fprintf(stderr, "## wrt-installer : %s %s has failed - given"
                    " version is lower than existing version\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_MANIFEST_NOT_FOUND:
            fprintf(stderr, "## wrt-installer : %s %s has failed - manifest"
                    " file doesn't find in package.\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_MANIFEST_INVALID:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "invalid manifestx.xml\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_CONFIG_NOT_FOUND:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "config.xml does not exist\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_CONFIG_INVALID:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "invalid config.xml\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_SIGNATURE_NOT_FOUND:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "signature doesn't exist in package.\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_SIGNATURE_INVALID:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "invalid signature.\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_SIGNATURE_VERIFICATION_FAILED:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "signature verification failed.\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_ROOT_CERTIFICATE_NOT_FOUND:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "root certificate could not find.\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_CERTIFICATION_INVAID:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "invalid certification.\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_CERTIFICATE_CHAIN_VERIFICATION_FAILED:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "certificate chain verification failed.\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_CERTIFICATE_EXPIRED:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "certificate expired.\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_INVALID_PRIVILEGE:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "invalid privilege\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_PRIVILEGE_LEVEL_VIOLATION:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "privilege level violation\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_MENU_ICON_NOT_FOUND:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "menu icon could not find\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_FATAL_ERROR:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "fatal error\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_OUT_OF_STORAGE:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "out of storage\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_OUT_OF_MEMORY:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "out of memory\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_ARGUMENT_INVALID:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "invalid argument\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_PACKAGE_ALREADY_INSTALLED:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "package already installed\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_ACE_CHECK_FAILED:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "ace check failure\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_MANIFEST_CREATE_FAILED:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "to create manifest failed\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_ENCRYPTION_FAILED:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "encryption of resource failed\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_INSTALL_OSP_SERVCIE:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "installation of osp service failed\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        case WRT_INSTALLER_ERROR_UNINSTALLATION_FAILED:
            fprintf(stderr, "## wrt-installer : %s %s has failed - "
                    "widget uninstallation failed\n",
                    tizenId.c_str(), printMsg.c_str());
            break;


        case WRT_INSTALLER_ERROR_UNKNOWN:
            fprintf(stderr,"## wrt-installer : %s %s has failed - unknown error\n",
                    tizenId.c_str(), printMsg.c_str());
            break;

        default:
            break;
    }

}

void WrtInstaller::staticWrtPluginInstallationCallback(WrtErrStatus status,
                                                       void* userdata)
{
    Assert(userdata);

    PluginInstallerData* data = static_cast<PluginInstallerData*>(userdata);

    WrtInstaller *This = static_cast<WrtInstaller*>(data->wrtInstaller);

    std::string path = std::string(data->pluginPath);
    delete data;

    This->m_numPluginsToInstall--;
    _D("Plugins to install: %d", This->m_numPluginsToInstall);

    if (This->m_numPluginsToInstall < 1) {
        _D("All plugins installation completed");
        fprintf(stderr, "All plugins installation completed.\n");

        //remove installation request
        if (!PluginUtils::removeInstallationRequiredFlag()) {
            _D("Failed to remove file initializing plugin installation");
        }

        //remove lock file
        if (!PluginUtils::unlockPluginInstallation(
            This->m_installMode.installTime == InstallMode::InstallTime::PRELOAD))
        {
            _D("Failed to remove installation lock");
        }

        This->DPL::Event::ControllerEventHandler<WRTInstallerNS::NextStepEvent>
            ::PostEvent(WRTInstallerNS::NextStepEvent());
    } else {
        This->DPL::Event::ControllerEventHandler<WRTInstallerNS::
                                                     InstallPluginEvent>::
            PostEvent(
            WRTInstallerNS::InstallPluginEvent());
    }

    if (WRT_SUCCESS == status) {
        This->m_returnStatus = 0;
        fprintf(stderr,
                "## wrt-installer : plugin installation successfull [%s]\n",
                path.c_str());
        _D("One plugin Installation succesfull: %s", path.c_str());
        return;
    }

    // Failure
    _W("One of the plugins installation failed!: %s", path.c_str());

    switch (status) {
    case WRT_INSTALLER_ERROR_PLUGIN_INSTALLATION_FAILED:
        _E("failed: plugin installation failed\n");
        break;

    case WRT_INSTALLER_ERROR_UNKNOWN:
        _E("failed: unknown error\n");
        break;

    default:
        break;
    }
}

void WrtInstaller::staticWrtPluginInstallProgressCb(float percent,
                                                    const char* description,
                                                    void* userdata)
{
    PluginInstallerData* data = static_cast<PluginInstallerData*>(userdata);

    std::string path = std::string(data->pluginPath);

    _D("Plugin Installation: %s progress: %2.0f description: %s", path.c_str(), percent, description);
}

void WrtInstaller::staticWrtInstallProgressCallback(float percent,
                                                    const char* description,
                                                    void* /*userdata*/)
{
    //WrtInstaller *This = static_cast<WrtInstaller*>(userdata);
    _D(" progress: %2.0f description: %s", percent, description);
}
void WrtInstaller::staticWrtUninstallProgressCallback(float percent,
                                                      const char* description,
                                                      void* /*userdata*/)
{
    //WrtInstaller *This = static_cast<WrtInstaller*>(userdata);
    _D(" progress: %2.0f description: %s", percent, description);
}

void WrtInstaller::installNewPlugins()
{
    _D("Install new plugins");

    if (!PluginUtils::lockPluginInstallation(
        m_installMode.installTime == InstallMode::InstallTime::PRELOAD))
    {
        _D("Lock NOT created");
        return;
    }

    if (!PluginUtils::checkPluginInstallationRequired()) {
        _D("Plugin installation not required");
        PluginUtils::unlockPluginInstallation(
            m_installMode.installTime == InstallMode::InstallTime::PRELOAD);
        return;
    }

    m_startupPluginInstallation = true;
    AddStep(&WrtInstaller::installPluginsStep);
}

void WrtInstaller::getRecoveryPackageId(std::string &pkgId)
{
    _D("getRecoveryPackageId");
    std::string folderPath =
        std::string(WrtDB::GlobalConfig::GetTempInstallInfoPath()) + "/";

    DIR* dir = opendir(folderPath.c_str());
    if (NULL == dir) {
        return;
    }

    struct dirent dEntry;
    struct dirent *dEntryResult;
    int return_code;

    do {
        struct stat statInfo;
        return_code = readdir_r(dir, &dEntry, &dEntryResult);
        if (dEntryResult != NULL && return_code == 0) {
            std::string fileName = dEntry.d_name;
            std::string fullName = folderPath + "/" + fileName;

            if (stat(fullName.c_str(), &statInfo) != 0) {
                closedir(dir);
                return;
            }

            if (S_ISDIR(statInfo.st_mode)) {
                if (("." == fileName) || (".." == fileName)) {
                    continue;
                }
            } else {
                pkgId = fileName;
                if (0 != unlink(fullName.c_str())) {
                    _E("Fail to delete : %s", fullName.c_str());
                }
            }
        }
    } while (dEntryResult != NULL && return_code == 0);
    closedir(dir);
}

#if 0
void shell(const char* cmd) {
    char buf[256];
    FILE *fp;

    _E("### %s ###", cmd);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        _E("error: fp is NULL");
    } else {
        while(fgets(buf, 256, fp) != NULL) {
            _E("%s", buf);
        }
        pclose(fp);
    }
}
#endif

int main(int argc, char *argv[])
{
    UNHANDLED_EXCEPTION_HANDLER_BEGIN
    {
        DPL::Log::LogSystemSingleton::Instance().SetTag("WRT_INSTALLER");

        // Output on stdout will be flushed after every newline character,
        // even if it is redirected to a pipe. This is useful for running
        // from a script and parsing output.
        // (Standard behavior of stdlib is to use full buffering when
        // redirected to a pipe, which means even after an end of line
        // the output may not be flushed).
        setlinebuf(stdout);

        // Check and re-set the file open limitation
        struct rlimit rlim;
        if (getrlimit(RLIMIT_NOFILE, &rlim) != -1) {
            _D("RLIMIT_NOFILE sft(%d)", rlim.rlim_cur);
            _D("RLIMIT_NOFILE hrd(%d)", rlim.rlim_max);

            if (rlim.rlim_cur < NOFILE_CNT_FOR_INSTALLER) {
                rlim.rlim_cur = NOFILE_CNT_FOR_INSTALLER;
                rlim.rlim_max = NOFILE_CNT_FOR_INSTALLER;
                if (setrlimit(RLIMIT_NOFILE, &rlim) == -1) {
                    _E("setrlimit is fail!!");
                }
            }
        } else {
            _E("getrlimit is fail!!");
        }

        WrtInstaller app(argc, argv);
        int ret = app.Exec();
        // In FOTA environment, appcore will return -1 due to /tmp is read-only.
        if (ret != 0) {
            app.OnCreate();
            elm_run();
            app.OnTerminate();
        }
        _D("App returned: %d", ret);
        ret = app.getReturnStatus();
        _D("WrtInstaller returned: %d", ret);
        return ret;
    }
    UNHANDLED_EXCEPTION_HANDLER_END
}
