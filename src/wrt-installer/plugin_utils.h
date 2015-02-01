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
 * @file    plugin-utils.h
 * @author
 * @version 1.0
 * @brief   Header file for plugin util
 */
#ifndef PLUGIN_UTILS_H
#define PLUGIN_UTILS_H

#include <string>
#include <sys/stat.h>

namespace PluginUtils {
struct FileState
{
    enum Type
    {
        FILE_EXISTS,
        FILE_EXISTS_NOT_REGULAR,
        FILE_NOT_EXISTS,
        FILE_READ_DATA_ERROR
    };
};

bool lockPluginInstallation(bool isPreload);
bool unlockPluginInstallation(bool isPreload);
bool checkPluginInstallationRequired();
bool removeInstallationRequiredFlag();
FileState::Type checkFile(const std::string& filename);
bool removeFile(const std::string& filename);
}
#endif // PLUGIN_UTILS_H
