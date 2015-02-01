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

#ifndef PLUGIN_PATH_H
#define PLUGIN_PATH_H

#include <string>
#include <dpl/string.h>
#include <dpl/utils/path.h>

class PluginPath: public DPL::Utils::Path{
private:
    std::string m_library;

public:
    PluginPath(const DPL::Utils::Path& fullPath);
    PluginPath(const std::string& fullPath);
    PluginPath(const DPL::String& fullPath);
    PluginPath();

    //getMetafile() this function adds metafile to current path.
    PluginPath getMetaFile() const;

    //setLibraryCombinedName This function creates name for library by adding
    //prefix and suffix to PluginPath object filename.
    void setLibraryCombinedName(const std::string& prefix, const std::string& sufix)
    {
      this->m_library = prefix + this->Filename() + sufix;
    }

    //getLibraryName returns library name
    const std::string& getLibraryName() const
    {
        return m_library;
    }
    //getLibraryPath returns full path to the library
    const PluginPath getLibraryPath() const
    {
        return this->operator /(m_library);
    }
};

#endif // PLUGIN_PATH_H
