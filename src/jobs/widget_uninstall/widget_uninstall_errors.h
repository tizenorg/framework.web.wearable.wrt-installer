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
 * @file    widget_uninstall_errors.h
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @version
 * @brief
 */

#ifndef WIDGET_UNINSTALL_ERRORS_H_
#define WIDGET_UNINSTALL_ERRORS_H_

#include <job_exception_base.h>
#include <job_exception_error.h>

using namespace Jobs::Exceptions;

namespace Jobs {
namespace WidgetUninstall {
namespace Exceptions {

DECLARE_JOB_EXCEPTION_BASE(JobExceptionBase, Base, ErrorUnknown)

DECLARE_JOB_EXCEPTION(Base, DatabaseFailure, ErrorWidgetUninstallationFailed)
DECLARE_JOB_EXCEPTION(Base, AlreadyUninstalling,
        ErrorWidgetUninstallationFailed)
DECLARE_JOB_EXCEPTION(Base, AppIsRunning, ErrorWidgetUninstallationFailed)
DECLARE_JOB_EXCEPTION(Base, WidgetNotExist, ErrorWidgetUninstallationFailed)
DECLARE_JOB_EXCEPTION(Base, UninstallOspSvcFailed,
        ErrorWidgetUninstallationFailed)
DECLARE_JOB_EXCEPTION(Base, PlatformAPIFailure, ErrorWidgetUninstallationFailed)
DECLARE_JOB_EXCEPTION(Base, RemoveFileFailure, ErrorWidgetUninstallationFailed)
DECLARE_JOB_EXCEPTION(Base, Unremovable, ErrorWidgetUninstallationFailed)
} //namespace
} //namespace
} //namespace

#endif /* WIDGET_UNINSTALL_ERRORS_H_ */
