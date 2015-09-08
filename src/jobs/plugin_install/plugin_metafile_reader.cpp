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
 * @file        plugin_metafile_reader.cpp
 * @author      Grzegorz Krawczyk(g.krawczyk@samsung.com)
 * @version     1.0
 * @brief
 */

#include "plugin_metafile_reader.h"
#include <plugin_path.h>

using namespace WrtDB;

namespace {
const std::string XML_NAMESPACE = "";

const std::string TOKEN_LIBRARY_NAME = "library-name";
const std::string TOKEN_API_FEATURE = "api-feature";
const std::string TOKEN_NAME = "name";
const std::string TOKEN_DEVICECAPABILITY = "device-capability";
}

PluginMetafileReader::PluginMetafileReader() : m_parserSchema(this)
{
    m_parserSchema.addEndTagCallback(
        TOKEN_LIBRARY_NAME,
        XML_NAMESPACE,
        &PluginMetafileReader::tokenEndLibraryName);

    m_parserSchema.addEndTagCallback(
        TOKEN_API_FEATURE,
        XML_NAMESPACE,
        &PluginMetafileReader::tokenEndApiFeature);

    m_parserSchema.addEndTagCallback(
        TOKEN_NAME,
        XML_NAMESPACE,
        &PluginMetafileReader::tokenEndName);

    m_parserSchema.addEndTagCallback(
        TOKEN_DEVICECAPABILITY,
        XML_NAMESPACE,
        &PluginMetafileReader::tokenEndDeviceCapability);
}

void PluginMetafileReader::initialize(const PluginPath &filename)
{
    m_parserSchema.initialize(filename.Fullpath(),
                              true,
                              ValidationCore::SaxReader::VALIDATION_DTD,
                              std::string());
}

void PluginMetafileReader::read(WrtDB::PluginMetafileData &data)
{
    m_parserSchema.read(data);
}

void PluginMetafileReader::blankFunction(PluginMetafileData & /* data */)
{}

void PluginMetafileReader::tokenEndLibraryName(PluginMetafileData &data)
{
    data.m_libraryName = m_parserSchema.getText();
}

void PluginMetafileReader::tokenEndApiFeature(PluginMetafileData &data)
{
    data.m_featureContainer.insert(m_feature);
    m_feature.m_deviceCapabilities.clear();
}

void PluginMetafileReader::tokenEndName(PluginMetafileData & /* data */)
{
    m_feature.m_name = m_parserSchema.getText();
}

void PluginMetafileReader::tokenEndDeviceCapability(PluginMetafileData& /*data*/)
{
    m_feature.m_deviceCapabilities.insert(m_parserSchema.getText());
}

