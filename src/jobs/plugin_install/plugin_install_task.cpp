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
 * @file    install_one_task.cpp
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @author  Grzegorz Krawczyk (g.krawczyk@samgsung.com)
 * @version
 * @brief
 */

//SYSTEM INCLUDES
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>

//WRT INCLUDES
#include <dpl/foreach.h>
#include <job.h>
#include "plugin_install_task.h"
#include "job_plugin_install.h"
#include "plugin_installer_errors.h"
#include "plugin_metafile_reader.h"
#include <dpl/wrt-dao-ro/global_config.h>
//#include <plugin.h>
#include <wrt_common_types.h>
#include <dpl/wrt-dao-rw/feature_dao.h>
#include <dpl/wrt-dao-rw/plugin_dao.h>
#include "plugin_objects.h"
#include <wrt_plugin_export.h>
#include <plugin_path.h>
#include <installer_log.h>

using namespace WrtDB;

#define SET_PLUGIN_INSTALL_PROGRESS(step, desc)              \
    m_context->installerTask->UpdateProgress(               \
        PluginInstallerContext::step, desc);

#define DISABLE_IF_PLUGIN_WITHOUT_LIB()        \
    if (m_pluginInfo.m_libraryName.empty()) \
    {                                          \
        _W("Plugin without library."); \
        return;                                \
    }

namespace Jobs {
namespace PluginInstall {
PluginInstallTask::PluginInstallTask(PluginInstallerContext *inCont) :
    DPL::TaskDecl<PluginInstallTask>(this),
    m_context(inCont),
    m_pluginHandle(0),
    m_dataFromConfigXML(true)
{
    AddStep(&PluginInstallTask::stepCheckPluginPath);
    AddStep(&PluginInstallTask::stepParseConfigFile);
    AddStep(&PluginInstallTask::stepFindPluginLibrary);
    AddStep(&PluginInstallTask::stepCheckIfAlreadyInstalled);
    AddStep(&PluginInstallTask::stepLoadPluginLibrary);
    AddStep(&PluginInstallTask::stepRegisterPlugin);
    AddStep(&PluginInstallTask::stepRegisterFeatures);
    AddStep(&PluginInstallTask::stepRegisterPluginObjects);
    AddStep(&PluginInstallTask::stepResolvePluginDependencies);

    SET_PLUGIN_INSTALL_PROGRESS(START, "Installation initialized");
}

PluginInstallTask::~PluginInstallTask()
{}

void PluginInstallTask::stepCheckPluginPath()
{
    _D("Plugin installation: step CheckPluginPath");

    if(!m_context->pluginFilePath.Exists()){
        ThrowMsg(Exceptions::PluginPathFailed,
                 "No such path");
    }

    SET_PLUGIN_INSTALL_PROGRESS(PLUGIN_PATH, "Path to plugin verified");
}

void PluginInstallTask::stepParseConfigFile()
{
    _D("Plugin installation: step parse config file");

    if(!m_context->pluginFilePath.getMetaFile().Exists()){
        m_dataFromConfigXML = false;
        return;
    }

    _D("Plugin Config file::%s", m_context->pluginFilePath.getMetaFile().Fullpath().c_str());

    Try
    {
        PluginMetafileReader reader;
        reader.initialize(m_context->pluginFilePath.getMetaFile());
        reader.read(m_pluginInfo);

        FOREACH(it, m_pluginInfo.m_featureContainer)
        {
            _D("Parsed feature : %s", it->m_name.c_str());
            FOREACH(devCap, it->m_deviceCapabilities) {
                _D("  |  DevCap : %s", (*devCap).c_str());
            }
        }

        SET_PLUGIN_INSTALL_PROGRESS(PLUGIN_PATH, "Config file analyzed");
    }
    Catch(ValidationCore::ParserSchemaException::Base)
    {
        _E("Error during file processing %s", m_context->pluginFilePath.getMetaFile().Fullpath().c_str());
        ThrowMsg(Exceptions::PluginMetafileFailed,
                 "Metafile error");
    }
}

void PluginInstallTask::stepFindPluginLibrary()
{
    if (m_dataFromConfigXML) {
        return;
    }
    _D("Plugin installation: step find plugin library");
    _D("Plugin .so: %s", m_context->pluginFilePath.getLibraryName().c_str());
    m_pluginInfo.m_libraryName = m_context->pluginFilePath.getLibraryName();
}

void PluginInstallTask::stepCheckIfAlreadyInstalled()
{
    if (PluginDAO::isPluginInstalled(m_pluginInfo.m_libraryName)) {
        ThrowMsg(Exceptions::PluginAlreadyInstalled,
                 "Plugin already installed");
    }

    SET_PLUGIN_INSTALL_PROGRESS(PLUGIN_EXISTS_CHECK, "Check if plugin exist");
}

void PluginInstallTask::stepLoadPluginLibrary()
{
    _D("Plugin installation: step load library");

    DISABLE_IF_PLUGIN_WITHOUT_LIB()

    _D("Loading plugin: %s", m_context->pluginFilePath.getLibraryName().c_str());

    fprintf(stderr, " - Try to dlopen() : [%s] ", m_context->pluginFilePath.getLibraryPath().Fullpath().c_str());

    void *dlHandle = dlopen( m_context->pluginFilePath.getLibraryPath().Fullpath().c_str(), RTLD_LAZY);
    if (dlHandle == NULL) {
        const char* error = (const char*)dlerror();
        fprintf(stderr,
                "-> Failed!\n   %s\n",
                (error != NULL ? error : "unknown"));
        _E("Failed to load plugin: %s. Reason: %s",
            m_context->pluginFilePath.getLibraryName().c_str(), (error != NULL ? error : "unknown"));
        ThrowMsg(Exceptions::PluginLibraryError, "Library error");
    }

    fprintf(stderr, "-> Done.\n");

    const js_entity_definition_t *rawEntityList = NULL;
    get_widget_entity_map_proc *getWidgetEntityMapProcPtr = NULL;

    getWidgetEntityMapProcPtr =
        reinterpret_cast<get_widget_entity_map_proc *>(dlsym(dlHandle,
                                                             PLUGIN_GET_CLASS_MAP_PROC_NAME));

    if (getWidgetEntityMapProcPtr) {
        rawEntityList = (*getWidgetEntityMapProcPtr)();
    } else {
        rawEntityList =
            static_cast<const js_entity_definition_t *>(dlsym(dlHandle,
                                                              PLUGIN_CLASS_MAP_NAME));
    }

    if (rawEntityList == NULL) {
        dlclose(dlHandle);
        _E("Failed to read class name %s", m_context->pluginFilePath.getLibraryName().c_str());
        ThrowMsg(Exceptions::PluginLibraryError, "Library error");
    }

    if (!m_dataFromConfigXML) {
        on_widget_init_proc *onWidgetInitProc =
            reinterpret_cast<on_widget_init_proc *>(
                dlsym(dlHandle, PLUGIN_WIDGET_INIT_PROC_NAME));

        if (NULL == onWidgetInitProc) {
            dlclose(dlHandle);
            _E("Failed to read onWidgetInit symbol %s", m_context->pluginFilePath.getLibraryName().c_str());
            ThrowMsg(Exceptions::PluginLibraryError, "Library error");
        }

        // obtain feature -> dev-cap mapping
        feature_mapping_interface_t mappingInterface = { NULL, NULL, NULL };
        (*onWidgetInitProc)(&mappingInterface);

        if (!mappingInterface.featGetter || !mappingInterface.release ||
            !mappingInterface.dcGetter)
        {
            _E("Failed to obtain mapping interface from .so");
            ThrowMsg(Exceptions::PluginLibraryError, "Library error");
        }

        feature_mapping_t* devcapMapping = mappingInterface.featGetter();

        _D("Getting mapping from features to device capabilities");

        for (size_t i = 0; i < devcapMapping->featuresCount; ++i) {
            PluginMetafileData::Feature feature;
            feature.m_name = devcapMapping->features[i].feature_name;

            _D("Feature: %s", feature.m_name.c_str());

            const devcaps_t* dc =
                mappingInterface.dcGetter(
                    devcapMapping,
                    devcapMapping->features[i].
                        feature_name);

            if (dc) {
                _D("devcaps count: %d", dc->devCapsCount);

                for (size_t j = 0; j < dc->devCapsCount; ++j) {
                    _D("devcap: %s", dc->deviceCaps[j]);
                    feature.m_deviceCapabilities.insert(dc->deviceCaps[j]);
                }
            }

            m_pluginInfo.m_featureContainer.insert(feature);
        }

        mappingInterface.release(devcapMapping);
    }

    m_libraryObjects = PluginObjectsPtr(new PluginObjects());
    const js_entity_definition_t *rawEntityListIterator = rawEntityList;

    _D("#####");
    _D("##### Plugin: %s supports new plugin API",
        m_context->pluginFilePath.getLibraryName().c_str());
    _D("#####");

    while (rawEntityListIterator->parent_name != NULL &&
           rawEntityListIterator->object_name != NULL)
    {
        _D("#####     [%s]: ", rawEntityListIterator->object_name);
        _D("#####     Parent: %s", rawEntityListIterator->parent_name);
        _D("#####");

        m_libraryObjects->addObjects(rawEntityListIterator->parent_name,
                                     rawEntityListIterator->object_name);

        ++rawEntityListIterator;
    }

    // Unload library
    if (dlclose(dlHandle) != 0) {
        _E("Cannot close plugin handle");
    } else {
        _D("Library is unloaded");
    }

    // Load export table
    _D("Library successfuly loaded and parsed");

    SET_PLUGIN_INSTALL_PROGRESS(LOADING_LIBRARY, "Library loaded and analyzed");
}

void PluginInstallTask::stepRegisterPlugin()
{
    _D("Plugin installation: step register Plugin");

    m_pluginHandle =
        PluginDAO::registerPlugin(m_pluginInfo, m_context->pluginFilePath.Fullpath());

    SET_PLUGIN_INSTALL_PROGRESS(REGISTER_PLUGIN, "Plugin registered");
}

void PluginInstallTask::stepRegisterFeatures()
{
    _D("Plugin installation: step register features");

    FOREACH(it, m_pluginInfo.m_featureContainer)
    {
        _D("PluginHandle: %d", m_pluginHandle);
        FeatureDAO::RegisterFeature(*it, m_pluginHandle);
    }
    SET_PLUGIN_INSTALL_PROGRESS(REGISTER_FEATURES, "Features registered");
}

void PluginInstallTask::stepRegisterPluginObjects()
{
    _D("Plugin installation: step register objects");

    DISABLE_IF_PLUGIN_WITHOUT_LIB()

    //register implemented objects
    PluginObjects::ObjectsPtr objects =
        m_libraryObjects->getImplementedObject();

    FOREACH(it, *objects)
    {
        PluginDAO::registerPluginImplementedObject(*it, m_pluginHandle);
    }

    //register requiredObjects
    objects = m_libraryObjects->getDependentObjects();

    FOREACH(it, *objects)
    {
        if (m_libraryObjects->hasObject(*it)) {
            _D("Dependency from the same library. ignored");
            continue;
        }

        PluginDAO::registerPluginRequiredObject(*it, m_pluginHandle);
    }

    SET_PLUGIN_INSTALL_PROGRESS(REGISTER_OBJECTS, "Plugin Objects registered");
}

void PluginInstallTask::stepResolvePluginDependencies()
{
    _D("Plugin installation: step resolve dependencies ");

    //DISABLE_IF_PLUGIN_WITHOUT_LIB
    if (m_pluginInfo.m_libraryName.empty()) {
        PluginDAO::setPluginInstallationStatus(
            m_pluginHandle,
            PluginDAO::
                INSTALLATION_COMPLETED);
        //Installation completed
        m_context->pluginHandle = m_pluginHandle;
        m_context->installationCompleted = true;
        _W("Plugin without library.");
        return;
    }

    PluginHandleSetPtr handles = PluginHandleSetPtr(new PluginHandleSet);

    DbPluginHandle handle = INVALID_PLUGIN_HANDLE;

    //register requiredObjects
    FOREACH(it, *(m_libraryObjects->getDependentObjects()))
    {
        if (m_libraryObjects->hasObject(*it)) {
            _D("Dependency from the same library. ignored");
            continue;
        }

        handle = PluginDAO::getPluginHandleForImplementedObject(*it);
        if (handle == INVALID_PLUGIN_HANDLE) {
            _E("Library implementing: %s NOT FOUND", (*it).c_str());
            PluginDAO::setPluginInstallationStatus(
                m_pluginHandle,
                PluginDAO::INSTALLATION_WAITING);
            return;
        }

        handles->insert(handle);
    }

    PluginDAO::registerPluginLibrariesDependencies(m_pluginHandle, handles);

    PluginDAO::setPluginInstallationStatus(m_pluginHandle,
                                           PluginDAO::INSTALLATION_COMPLETED);

    //Installation completed
    m_context->pluginHandle = m_pluginHandle;
    m_context->installationCompleted = true;

    SET_PLUGIN_INSTALL_PROGRESS(RESOLVE_DEPENDENCIES, "Dependencies resolved");
}

#undef SET_PLUGIN_INSTALL_PROGRESS
} //namespace Jobs
} //namespace PluginInstall
