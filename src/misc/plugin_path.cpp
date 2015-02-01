/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file    plugin_path_builder.cpp
 * @author  Kamil Nować (k.nowac@partner.samgsung.com)
 * @version
 * @brief
 */

#include <plugin_path.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dlfcn.h>

using namespace DPL::Utils;

PluginPath::PluginPath(const Path& fullPath) : Path(fullPath.Fullpath())
{
    setLibraryCombinedName(
            WrtDB::GlobalConfig::GetPluginPrefix(),
            WrtDB::GlobalConfig::GetPluginSuffix());
};
PluginPath::PluginPath(const std::string& fullPath) : Path(fullPath)
{
    setLibraryCombinedName(
            WrtDB::GlobalConfig::GetPluginPrefix(),
            WrtDB::GlobalConfig::GetPluginSuffix());
};
PluginPath::PluginPath(const DPL::String& fullPath) : Path(fullPath)
{
    setLibraryCombinedName(
            WrtDB::GlobalConfig::GetPluginPrefix(),
            WrtDB::GlobalConfig::GetPluginSuffix());
};
PluginPath::PluginPath(){}

PluginPath PluginPath::getMetaFile() const
{
    PluginPath metaFile = *this;
    return metaFile /= WrtDB::GlobalConfig::GetPluginMetafileName();
}