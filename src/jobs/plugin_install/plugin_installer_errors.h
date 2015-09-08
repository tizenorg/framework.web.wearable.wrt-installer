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
 * @file    plugin_installer_errors.h
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @author  Grzegorz Krawczyk (g.krawczyk@samgsung.com)
 * @version
 * @brief
 */

#ifndef \
    WRT_SRC_INSTALLER_CORE_PLUGIN_INSTALLER_TASKS_PLUGIN_INSTALLER_ERRORS_H_
#define \
    WRT_SRC_INSTALLER_CORE_PLUGIN_INSTALLER_TASKS_PLUGIN_INSTALLER_ERRORS_H_

#include <job_exception_base.h>
#include <job_exception_error.h>

using namespace Jobs::Exceptions;

namespace Jobs {
namespace PluginInstall {
namespace Exceptions {

DECLARE_JOB_EXCEPTION_BASE(JobExceptionBase, Base, ErrorUnknown)
DECLARE_JOB_EXCEPTION(Base, PluginPathFailed, ErrorPluginInstallationFailed)
DECLARE_JOB_EXCEPTION(Base, PluginMetafileFailed, ErrorPluginInstallationFailed)
DECLARE_JOB_EXCEPTION(Base, PluginAlreadyInstalled,
        ErrorPluginInstallationFailed)
DECLARE_JOB_EXCEPTION(Base, PluginLibraryError, ErrorPluginInstallationFailed)
DECLARE_JOB_EXCEPTION(Base, InstallationWaitingError,
        ErrorPluginInstallationFailed)
DECLARE_JOB_EXCEPTION(Base, UnknownError, ErrorUnknown)
} //namespace
} //namespace
} //namespace

#endif
//WRT_SRC_INSTALLER_CORE_PLUGIN_INSTALLER_TASKS_PLUGIN_INSTALLER_ERRORS_H_

