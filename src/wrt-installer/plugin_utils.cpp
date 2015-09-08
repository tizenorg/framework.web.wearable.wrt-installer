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
 * @file    plugin-utils.cpp
 * @author
 * @version 1.0
 * @brief   Header file for plugin util
 */

#include <unistd.h>
#include "plugin_utils.h"
#include <dpl/exception.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <sys/file.h>
#include <installer_log.h>

using namespace WrtDB;

namespace PluginUtils {
const char* PLUGIN_INSTALL_LOCK_FILE = "/tmp/.wrt_plugin_install_lock";

static int s_plugin_install_lock_fd = -1;

bool lockPluginInstallation(bool isPreload)
{
    if (isPreload) {
        fprintf(stderr, "Skip create lock file.. \n");
        return true;
    }

    int ret = 0;

    _D("Try to lock for plugins installation.");

    s_plugin_install_lock_fd =
        open(PLUGIN_INSTALL_LOCK_FILE, O_RDONLY | O_CREAT, 0666);

    if (s_plugin_install_lock_fd == -1) {
        _E("Lock file open failed!");

        return false;
    }

    ret = flock(s_plugin_install_lock_fd, LOCK_EX); //lock with waiting

    if (ret == -1) {
        _E("Lock failed!");

        close(s_plugin_install_lock_fd);
        s_plugin_install_lock_fd = -1;

        return false;
    }

    return true;
}

bool unlockPluginInstallation(bool isPreload)
{
    _D("Unlock for plugins installation.");
    if (isPreload) {
        fprintf(stderr, "Skip plugin unlock.. \n");
        return true;
    }

    if (s_plugin_install_lock_fd != -1) {
        int ret = 0;

        ret = flock(s_plugin_install_lock_fd, LOCK_UN); //unlock

        if (ret == -1) {
            _E("Unlock failed!");
        }

        close(s_plugin_install_lock_fd);
        s_plugin_install_lock_fd = -1;

        return true;
    } else {
        _E("Lock file was not created!");
    }

    return false;
}

bool checkPluginInstallationRequired()
{
    std::string installRequest =
        std::string(GlobalConfig::GetPluginInstallInitializerName());

    FileState::Type installationRequest =
        checkFile(installRequest);

    switch (installationRequest) {
    case FileState::FILE_EXISTS:
        return true;
    case FileState::FILE_NOT_EXISTS:
        return false;
    default:
        _W("Opening installation request file failed");
        return false;
    }
}

bool removeInstallationRequiredFlag()
{
    std::string installRequest =
        std::string(GlobalConfig::GetPluginInstallInitializerName());

    return removeFile(installRequest);
}

//checks if file exists and is regular file
FileState::Type checkFile(const std::string& filename)
{
    struct stat tmp;

    if (-1 == stat(filename.c_str(), &tmp)) {
        if (ENOENT == errno) {
            return FileState::FILE_NOT_EXISTS;
        }
        return FileState::FILE_READ_DATA_ERROR;
    } else if (!S_ISREG(tmp.st_mode)) {
        return FileState::FILE_EXISTS_NOT_REGULAR;
    }
    return FileState::FILE_EXISTS;
}

bool removeFile(const std::string& filename)
{
    if (0 != unlink(filename.c_str())) {
        return false;
    }

    return true;
}
} //namespace PluginUtils
