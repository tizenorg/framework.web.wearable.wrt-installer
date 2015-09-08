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
 * @file        plugin_objects.h
 * @author      Grzegorz Krawczyk(g.krawczyk@samsung.com)
 * @version     1.0
 * @brief
 */

#ifndef WRT_SRC_INSTALLER_CORE_PLUGIN_INSTALLER_TASKS_PLUGIN_OBJECTS_H_
#define WRT_SRC_INSTALLER_CORE_PLUGIN_INSTALLER_TASKS_PLUGIN_OBJECTS_H_

#include <memory>
#include <string>
#include <set>
#include <list>

#include <dpl/wrt-dao-ro/common_dao_types.h>
//TODO TO BE MOVED SOMEWHERE ELSE
// AS OTHER MODULES (LIKE DAO) USE IT

class PluginObjects : public WrtDB::PluginObjectsDAO
{
  public:
    explicit PluginObjects();

    //getters for objects from library
    ObjectsPtr getImplementedObject() const;
    ObjectsPtr getDependentObjects() const;

    //add object declaration
    void addObjects(const std::string& parentName,
                    const std::string& name);

    //check if library implemements object given as name
    bool hasObject(const std::string& name) const;

  private:
    void addImplementedObject(const std::string& value);
    void addDependentObject(const std::string& value);
};

typedef std::shared_ptr<PluginObjects> PluginObjectsPtr;

#endif
