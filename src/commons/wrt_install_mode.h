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
 * @file    wrt_install_mode.h
 * @author  Jihoon Chung (jihoon.chung@samgsung.com)
 * @version
 * @brief   Definition file of widget install mode class
 */

#ifndef WRT_INSTALL_MODE_H
#define WRT_INSTALL_MODE_H

class InstallMode
{
  public:
    enum class Command
    {
        INSTALL,
        REINSTALL,
        RECOVERY
    };
    enum class Location
    {
        INTERNAL,
        EXTERNAL
    };
    enum class RootPath
    {
        RW,
        RO
    };
    enum class ExtensionType
    {
        WGT,
        DIR
    };
    enum class InstallTime
    {
        NORMAL,
        CSC,
        PRELOAD,
        FOTA,
    };

    InstallMode(Command cmd = Command::INSTALL,
                Location lo = Location::INTERNAL,
                RootPath root = RootPath::RW,
                ExtensionType extensionType = ExtensionType::WGT,
                InstallTime time = InstallTime::NORMAL) :
        command(cmd),
        location(lo),
        rootPath(root),
        extension(extensionType),
        installTime(time),
        removable(true)
    {};

    Command command;
    Location location;
    RootPath rootPath;
    ExtensionType extension;
    InstallTime installTime;
    bool removable;
    std::string cscPath;
};

#endif // WRT_INSTALL_MODE_H

