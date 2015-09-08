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
 * @file    wrt-installer.h
 * @version 1.0
 * @brief   Implementation file for installer
 */
#ifndef WRT_INSTALLER_H
#define WRT_INSTALLER_H

#include <dpl/application.h>
#include <dpl/generic_event.h>
#include <dpl/event/controller.h>
#include <dpl/task.h>
#include <dpl/string.h>
#include <string>
#include <map>
#include <boost/optional.hpp>
#include <wrt_install_mode.h>
#include <wrt_type.h>
#include <pkgmgr_signal.h>

namespace WRTInstallerNS { //anonymous
DECLARE_GENERIC_EVENT_0(QuitEvent)
DECLARE_GENERIC_EVENT_0(NextStepEvent)
DECLARE_GENERIC_EVENT_0(InstallPluginEvent)
}

typedef void (*ShowResultCallback)(void *data, Evas_Object *obj,
                                   void *event_info);
class WrtInstaller :
    public DPL::Application,
    private DPL::Event::Controller<DPL::TypeListDecl<
                                       WRTInstallerNS::QuitEvent,
                                       WRTInstallerNS::NextStepEvent,
                                       WRTInstallerNS::InstallPluginEvent>::
                                       Type>,
    public DPL::TaskDecl<WrtInstaller>
{
  public:
    WrtInstaller(int argc,
                 char **argv);
    virtual ~WrtInstaller();

    int getReturnStatus() const;

    virtual void OnStop();
    virtual void OnCreate();
    virtual void OnReset(bundle *b);
    virtual void OnTerminate();

  private:
    void         showHelpAndQuit();
    void         showArguments();

    // Events
    virtual void OnEventReceived(const WRTInstallerNS::QuitEvent &event);
    virtual void OnEventReceived(const WRTInstallerNS::NextStepEvent& event);
    virtual void OnEventReceived(
        const WRTInstallerNS::InstallPluginEvent& event);

    // Installation steps
    void initStep();
    void installStep();
    void installPluginsStep();
    void uninstallPkgNameStep();
    void unistallWgtFileStep();
    void removeUpdateStep();
    void shutdownStep();

    // Static callbacks
    static void staticWrtStatusCallback(std::string tizenId,
                                        WrtErrStatus status,
                                        void* userdata);
    static void staticWrtPluginInstallationCallback(WrtErrStatus status,
                                                    void* userdata);
    static void staticWrtPluginInstallProgressCb(float percent,
                                                 const char* description,
                                                 void* userdata);
    static void staticWrtInstallProgressCallback(float percent,
                                                 const char* description,
                                                 void* userdata);

    static void staticWrtUninstallProgressCallback(float percent,
                                                   const char* description,
                                                   void* userdata);

    static void staticWrtInitializeToPreloadCallback(std::string tizenId,
                                        WrtErrStatus status,
                                        void* userdata);

    static void staticWrtInitPreloadStatusCallback(std::string tizenId,
                                        WrtErrStatus status,
                                        void* userdata);

    void installNewPlugins();
    void showErrorMsg(WrtErrStatus status, std::string tizenId, std::string
            printMsg);

    void makeStatusOfWrtInit(WrtErrStatus status);
    void getRecoveryPackageId(std::string &pkgId);
    bool setInitialCSC(std::string cscPath);

    // Private data
    std::shared_ptr<PackageManager::IPkgmgrSignal> pkgmgrSignalInterface;
    InstallMode m_installMode;
    std::string m_packagePath;
    std::string m_name;
    bool m_initialized;
    size_t m_numPluginsToInstall;
    size_t m_totalPlugins;
    int m_returnStatus;
    bool m_sendPkgSig;
    bool m_startupPluginInstallation;

    typedef std::list<std::string> PluginPathList;
    boost::optional<PluginPathList> m_pluginsPaths;
};
#endif // WRT_INSTALLER_H
