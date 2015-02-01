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
 * @file    api_callbacks_translate.h
 * @author  Pawel Sikorski (p.sikorski@samsung.com)
 * @version 1.0
 * @brief   Header file for api callbacks translate functions
 */
#ifndef WRT_SRC_API_API_CALLBACKS_TRANSLATE_H_
#define WRT_SRC_API_API_CALLBACKS_TRANSLATE_H_

#include <string>

#include <wrt_common_types.h>
#include <widget_install/widget_install_errors.h>
#include <widget_uninstall/widget_uninstall_errors.h>
#include <plugin_install/plugin_installer_errors.h>
#include <job_base.h>
#include <wrt_type.h>
#include <wrt_install_mode.h>
#include <pkgmgr_signal_interface.h>

typedef void (*WrtInstallerInitCallback)(WrtErrStatus status,
                                         void *data);
typedef void (*WrtPluginInstallerStatusCallback)(WrtErrStatus status,
                                                 void *data);
typedef void (*WrtInstallerStatusCallback)(std::string tizenId,
                                           WrtErrStatus status,
                                           void *data);
typedef void (*WrtProgressCallback)(float percent,
                                    const char *description,
                                    void *data);


namespace InstallerCallbacksTranslate {
struct StatusCallbackStruct
{
    void* userdata;
    WrtInstallerStatusCallback status_callback;
    WrtProgressCallback progress_callback;

    StatusCallbackStruct(void* u,
                         WrtInstallerStatusCallback s,
                         WrtProgressCallback p) :
        userdata(u),
        status_callback(s),
        progress_callback(p)
    {}
};

struct PluginStatusCallbackStruct
{
    void* userdata;
    WrtPluginInstallerStatusCallback statusCallback;
    WrtProgressCallback progressCallback;

    PluginStatusCallbackStruct(void* u,
                               WrtPluginInstallerStatusCallback s,
                               WrtProgressCallback p) :
        userdata(u),
        statusCallback(s),
        progressCallback(p)
    {}
};

void installFinishedCallback(void *userParam,
                             std::string tizenId,
                             Jobs::Exceptions::Type status);

void uninstallFinishedCallback(void *userParam,
                               std::string tizenId,
                               Jobs::Exceptions::Type status);

void pluginInstallFinishedCallback(void *userParam,
                                   Jobs::Exceptions::Type status);

// callback for progress of install OR uninstall
void installProgressCallback(void *userParam,
                             ProgressPercent percent,
                             const ProgressDescription &description);
} //namespace

#endif /* WRT_SRC_API_API_CALLBACKS_TRANSLATE_H_ */
