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
 * @file       task_commons.cpp
 * @author     Krzysztof Jackiewicz (k.jackiewicz@samsung.com)
 * @version    1.0
 */

#include "task_commons.h"
#include <unistd.h>
#include <sstream>
#include <ftw.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/exception.h>
#include <dpl/errno_string.h>
#include <dpl/utils/wrt_utility.h>
#include <widget_install/widget_install_errors.h>
#include <installer_log.h>

namespace Jobs {
namespace WidgetInstall {
namespace {
const char * const TEMPORARY_PATH_POSTFIX = "temp";
const mode_t TEMPORARY_PATH_MODE = 0775;
} // namespace

std::string createTempPath(bool preload)
{
    _D("Step: Creating temporary path");

    // Temporary path
    std::ostringstream tempPathBuilder;

    if (preload) {
        tempPathBuilder << WrtDB::GlobalConfig::GetUserPreloadedWidgetPath();
    } else {
        tempPathBuilder << WrtDB::GlobalConfig::GetUserInstalledWidgetPath();
    }
    tempPathBuilder << WrtDB::GlobalConfig::GetTmpDirPath();
    tempPathBuilder << "/";
    tempPathBuilder << TEMPORARY_PATH_POSTFIX;
    tempPathBuilder << "_";

    timeval tv;
    gettimeofday(&tv, NULL);
    tempPathBuilder <<
    (static_cast<unsigned long long>(tv.tv_sec) * 1000000ULL +
     static_cast<unsigned long long>(tv.tv_usec));

    std::string tempPath = tempPathBuilder.str();

    // Remove old path if any
    struct stat fileInfo;

    if (stat(tempPath.c_str(), &fileInfo) == 0) {
        if (!WrtUtilRemove(tempPath)) {
            ThrowMsg(Exceptions::RemovingFolderFailure,
                     "Failed to to remove temporary directory");
        }
    }
    // Create new path
    if (!WrtUtilMakeDir(tempPath, TEMPORARY_PATH_MODE)) {
        ThrowMsg(Exceptions::FileOperationFailed,
                 "Failed to create temporary directory");
    }

    return tempPath;
}

void createTempPath(const std::string& path)
{
    if (!WrtUtilMakeDir(path, TEMPORARY_PATH_MODE)) {
        ThrowMsg(Exceptions::FileOperationFailed,
                 "Failed to create temporary directory");
    }
}
} // WidgetInstall
} // Jobs
