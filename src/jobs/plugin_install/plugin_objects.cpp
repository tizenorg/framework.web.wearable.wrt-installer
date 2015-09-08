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
 * @file    plugin_objects.h
 * @author  Grzegorz Krawczyk (g.krawczyk@samgsung.com)
 * @version
 * @brief
 */
#include <string>
#include <installer_log.h>
#include "plugin_objects.h"

namespace {
const char* SEPARATOR = ".";
const std::string GLOBAL_OBJECT_NAME = "GLOBAL_OBJECT";

std::string normalizeName(const std::string& objectName)
{
    if (objectName.empty()) {
        _E("Normalize name, name size is 0");
        return objectName;
    }

    if (!objectName.compare(0, GLOBAL_OBJECT_NAME.size(),
                            GLOBAL_OBJECT_NAME))
    {
        return objectName;
    }

    //each object in storage has name started from $GLOBAL_OBJECT_NAME$
    return GLOBAL_OBJECT_NAME + std::string(SEPARATOR) + objectName;
}

std::string normalizeName(const std::string& objectName,
                          const std::string& parentName)
{
    if (objectName.empty() || parentName.empty()) {
        _E("Normalize name, name size or parent name size is 0");
        return std::string();
    }

    std::string normalizedName;
    normalizedName = normalizeName(parentName) +
        std::string(SEPARATOR) + objectName;

    return normalizedName;
}
}

PluginObjects::PluginObjects()
{
    m_implemented = ObjectsPtr(new Objects());
    m_dependent = ObjectsPtr(new Objects());
}

PluginObjects::ObjectsPtr PluginObjects::getImplementedObject() const
{
    return m_implemented;
}

PluginObjects::ObjectsPtr PluginObjects::getDependentObjects() const
{
    return m_dependent;
}

void PluginObjects::addObjects(const std::string& parentName,
                               const std::string& name)
{
    addImplementedObject(normalizeName(name, parentName));
    addDependentObject(normalizeName(parentName));
}

void PluginObjects::addDependentObject(const std::string& value)
{
    if (!value.compare(GLOBAL_OBJECT_NAME)) {
        //dont add dependency to GLOBAL_OBJECT
        return;
    }
    m_dependent->insert(value);
}

bool PluginObjects::hasObject(const std::string& name) const
{
    return m_implemented->find(name) != m_implemented->end();
}

void PluginObjects::addImplementedObject(const std::string& value)
{
    m_implemented->insert(value);
}
