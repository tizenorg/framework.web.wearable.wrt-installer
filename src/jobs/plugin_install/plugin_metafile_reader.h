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
 * @file        plugin_metafile_reader.h
 * @author      Grzegorz Krawczyk(g.krawczyk@samsung.com)
 * @version     1.0
 * @brief
 */

#ifndef WRT_SRC_INSTALLER_CORE_PLUGIN_INSTALLER_TASKS_PLUGIN_METAFILE_READER_H_
#define WRT_SRC_INSTALLER_CORE_PLUGIN_INSTALLER_TASKS_PLUGIN_METAFILE_READER_H_

#include <dpl/wrt-dao-ro/common_dao_types.h>
#include <vcore/ParserSchema.h>

class PluginPath;

class PluginMetafileReader
{
  public:
    PluginMetafileReader();

    void initialize(const PluginPath &filename);

    void read(WrtDB::PluginMetafileData &data);

  private:
    void blankFunction(WrtDB::PluginMetafileData &data);

    void tokenEndLibraryName(WrtDB::PluginMetafileData &data);
    void tokenEndApiFeature(WrtDB::PluginMetafileData &data);
    void tokenEndName(WrtDB::PluginMetafileData &data);
    void tokenEndDeviceCapability(WrtDB::PluginMetafileData &data);

    WrtDB::PluginMetafileData::Feature m_feature;

    ValidationCore::ParserSchema<PluginMetafileReader,
                                 WrtDB::PluginMetafileData> m_parserSchema;
};

#endif
