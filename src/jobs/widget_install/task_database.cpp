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
 * @file    task_new_db_insert.cpp
 * @author  Lukasz Wrzosek(l.wrzosek@samsung.com)
 * @author  Soyoung kim(sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task database updating for widget
 * update
 */
#include <unistd.h>
#include <cstdio>
#include <time.h>
#include <sys/stat.h>
#include <widget_install/task_database.h>
#include <widget_install/job_widget_install.h>
#include <widget_install/widget_install_errors.h>
#include <widget_install/widget_install_context.h>
#ifdef DBOX_ENABLED
#include <web_provider_livebox_info.h>
#endif
#include <dpl/wrt-dao-rw/widget_dao.h>
#include <dpl/foreach.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/assert.h>
#include <wrt-commons/security-origin-dao/security_origin_dao.h>
#include <wrt-commons/widget-interface-dao/widget_interface_dao.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/wrt-dao-ro/widget_dao_types.h>
#include <string>
#include <sstream>
#include <ace_api_install.h>
#include <ace_registration.h>
#include <errno.h>
#include <string.h>
#include <map>
#include <installer_log.h>

using namespace WrtDB;

namespace Jobs {
namespace WidgetInstall {
TaskDatabase::TaskDatabase(InstallerContext& context) :
    DPL::TaskDecl<TaskDatabase>(this),
    m_context(context)
{
    AddStep(&TaskDatabase::StartStep);
    AddStep(&TaskDatabase::StepRegisterExternalFiles);
    AddStep(&TaskDatabase::StepWrtDBInsert);
    AddStep(&TaskDatabase::StepAceDBInsert);
    AddStep(&TaskDatabase::StepSecurityOriginDBInsert);
    AddStep(&TaskDatabase::StepWidgetInterfaceDBInsert);
    AddStep(&TaskDatabase::StepRemoveExternalFiles);
#ifdef DBOX_ENABLED
    AddStep(&TaskDatabase::StepLiveboxDBInsert);
#endif
    AddStep(&TaskDatabase::EndStep);

    AddAbortStep(&TaskDatabase::StepAbortDBInsert);
    AddAbortStep(&TaskDatabase::StepAbortAceDBInsert);
    AddAbortStep(&TaskDatabase::StepAbortWidgetInterfaceDBInsert);
}

void TaskDatabase::StepWrtDBInsert()
{
    Try
    {
        /* Set install Time */
        time(&m_context.widgetConfig.installedTime);

        if (m_context.isUpdateMode) { //update
            _D("Registering widget... (update)");
            Try
            {
                std::list<TizenAppId> idList = WidgetDAOReadOnly::getTzAppIdList(m_context.widgetConfig.tzPkgid);
                FOREACH(it , idList ){
                    //installed AppId list, It need to delete ACE Database corresponding record
                    m_handleToRemoveList.push_back(WidgetDAOReadOnly::getHandle(*it));
                    WrtDB::TizenAppId backAppId = *it + L".backup";
                    m_backAppIdList.push_back(backAppId);
                    //Change all installed tzAppid to .backup
                    WidgetDAO::updateTizenAppId(*it, backAppId);
            }

            WidgetDAO::registerWidget(m_context.widgetConfig.tzAppid,
                                      m_context.widgetConfig,
                                      m_context.widgetSecurity);
            m_handleList.push_back(WidgetDAOReadOnly::getHandle(m_context.widgetConfig.tzAppid));

            FOREACH(iterator, m_context.widgetConfig.configInfo.serviceAppInfoList) {
                WrtDB::TizenAppId tizenAppId = iterator->serviceId;
                WidgetDAO::registerService(*iterator, m_context.widgetConfig,m_context.widgetSecurity);
                m_handleList.push_back(WidgetDAOReadOnly::getHandle(tizenAppId));
            }
            }
            Catch(WidgetDAOReadOnly::Exception::WidgetNotExist)
            {
                LogError(
                    "Given tizenId not found for update installation (Same GUID?)");
                ThrowMsg(Exceptions::DatabaseFailure,
                         "Given tizenId not found for update installation");
            }
        } else { //new installation
            _D("Registering widget...");
            WidgetDAO::registerWidget(
                m_context.widgetConfig.tzAppid,
                m_context.widgetConfig,
                m_context.widgetSecurity);

            m_handleList.push_back(WidgetDAOReadOnly::getHandle(m_context.widgetConfig.tzAppid));

            FOREACH(iterator, m_context.widgetConfig.configInfo.serviceAppInfoList) {
                    WidgetDAO::registerService(*iterator, m_context.widgetConfig,m_context.widgetSecurity);
                    m_handleList.push_back(WidgetDAOReadOnly::getHandle(iterator->serviceId));
            }
        }

        FOREACH(cap, m_context.staticPermittedDevCaps) {
            _D("staticPermittedDevCaps : %ls smack status: %d", cap->first.c_str(), cap->second);
        }

        _D("Widget registered");
    }
    Catch(WidgetDAO::Exception::DatabaseError)
    {
        _E("Database failure!");
        ReThrowMsg(Exceptions::InsertNewWidgetFailed, "Database failure!");
    }
    Catch(DPL::DB::SqlConnection::Exception::Base)
    {
        _E("Database failure!");
        ReThrowMsg(Exceptions::InsertNewWidgetFailed, "Database failure!");
    }
}

void TaskDatabase::StepAceDBInsert()
{
    FOREACH(iterHandleToRemove, m_handleToRemoveList)
    {
        if (INVALID_WIDGET_HANDLE != *iterHandleToRemove) {
            _D("Removing old insallation. Handle: %d", *iterHandleToRemove);
            if (ACE_OK != ace_unregister_widget(
                    static_cast<ace_widget_handle_t>(*iterHandleToRemove)))
            {
                _W("Error while removing ace entry for previous insallation");
            }
        }
    }

    FOREACH(iterHandle, m_handleList)
    {
        if (!AceApi::registerAceWidget(*iterHandle, m_context.widgetConfig,
                                   m_context.widgetSecurity.getCertificateList()))
        {
            _E("ace database insert failed");
            ThrowMsg(Exceptions::UpdateFailed,
                 "Update failure. ace_register_widget failed");
        }
        _D("Ace data inserted");
    }
}

void TaskDatabase::StepSecurityOriginDBInsert()
{
    _D("Create Security origin database");
    // automatically create security origin database
    using namespace SecurityOriginDB;
    using namespace WrtDB;

    try{
        SecurityOriginDAO dao(m_context.locations->getPkgId());
        // Checking privilege list for setting security origin exception data
        FOREACH(it, m_context.widgetConfig.configInfo.privilegeList) {
            std::map<std::string, Feature>::const_iterator result =
                g_W3CPrivilegeTextMap.find(DPL::ToUTF8String(it->name));
            if (result != g_W3CPrivilegeTextMap.end()) {
                if (result->second == FEATURE_FULLSCREEN_MODE) {
                    continue;
                } else {
                    dao.setPrivilegeSecurityOriginData(result->second);
                }
            }
        }
    }catch(const SecurityOriginDAO::Exception::DatabaseError& err){
        _E("error open SecurityOrigin db %s", err.GetMessage().c_str());
        ThrowMsg(Exceptions::UpdateFailed, "Cannot open SecurityOrigin DB");
    }
}

void TaskDatabase::StepWidgetInterfaceDBInsert()
{
    _D("Create Widget Interface database");
    using namespace WidgetInterfaceDB;
    using namespace WrtDB;

    DbWidgetHandle handle =
        WidgetDAOReadOnly::getHandle(m_context.widgetConfig.tzAppid);

    // backup database
    if (m_context.isUpdateMode) {
        std::string dbPath = WidgetInterfaceDAO::databaseFileName(handle);
        std::string backupDbPath = dbPath;
        backupDbPath += GlobalConfig::GetBackupDatabaseSuffix();
        _D("\"%s\" to \"%s\"", dbPath.c_str(), backupDbPath.c_str());
        if (0 != std::rename(dbPath.c_str(), backupDbPath.c_str())) {
            _E("widget interface database backup failed");
            ThrowMsg(Exceptions::UpdateFailed,
                     "widget interface database backup failed");
        }
    }

    Try
    {
        // automatically create widget interface database
        WidgetInterfaceDAO dao(handle);
    }
    Catch(WidgetInterfaceDAO::Exception::DatabaseError)
    {
        _E("widget interface database create failed");
        ThrowMsg(Exceptions::UpdateFailed,
                 "widget interface database create failed");
    }
}

void TaskDatabase::StepRegisterExternalFiles()
{
    WrtDB::ExternalLocationList externalLocationsUpdate =
        m_context.locations->listExternalLocations();
    if (m_context.isUpdateMode) { //update
        Try
        {
            WidgetDAOReadOnly dao(WidgetDAOReadOnly::getHandleByPkgId(m_context.widgetConfig.tzPkgid));
            WrtDB::ExternalLocationList externalLocationsDB =
                dao.getWidgetExternalLocations();
            FOREACH(file, externalLocationsDB)
            {
                if (std::find(externalLocationsUpdate.begin(),
                              externalLocationsUpdate.end(),
                              *file) == externalLocationsUpdate.end())
                {
                    m_externalLocationsToRemove.push_back(*file);
                }
            }
        }
        Catch(WidgetDAOReadOnly::Exception::WidgetNotExist)
        {
            _E("Given tizenId not found for update installation (Same GUID?)");
            ThrowMsg(Exceptions::UpdateFailed,
                     "Given tizenId not found for update installation");
        }
    }
    _D("Registering external files:");
    FOREACH(file, externalLocationsUpdate)
    {
        _D("  -> %s", (*file).c_str());
    }

    //set external locations to be registered
    m_context.widgetConfig.externalLocations = externalLocationsUpdate;
}

void TaskDatabase::StepRemoveExternalFiles()
{
    if (!m_externalLocationsToRemove.empty()) {
        _D("Removing external files:");
    }

    FOREACH(file, m_externalLocationsToRemove)
    {
        if (WrtUtilFileExists(*file)) {
            _D("  -> %s", (*file).c_str());
            if (-1 == remove(file->c_str())) {
                ThrowMsg(Exceptions::RemovingFileFailure,
                         "Failed to remove external file");
            }
        } else if (WrtUtilDirExists(*file)) {
            _D("  -> %s", (*file).c_str());
            if (!WrtUtilRemove(*file)) {
                ThrowMsg(Exceptions::RemovingFolderFailure,
                         "Failed to remove external directory");
            }
        } else {
            _W("  -> %s(no such a path)", (*file).c_str());
        }
    }
}

void TaskDatabase::StepAbortDBInsert()
{
    _W("[DB Update Task] Aborting... (DB Clean)");
    Try
    {
        WidgetDAO::unregisterWidget(m_context.widgetConfig.tzAppid);

        FOREACH(iter, m_context.widgetConfig.configInfo.serviceAppInfoList) {
            WidgetDAO::unregisterWidget(iter->serviceId);
        }

        if (m_context.isUpdateMode) {
            FOREACH(iter, m_backAppIdList) {
                unsigned pos = (*iter).find(L".backup");
                TizenAppId str = (*iter).substr(0,pos);
                WidgetDAO::updateTizenAppId(*iter,str);
            }
        }
        _D("Cleaning DB successful!");
    }
    Catch(DPL::DB::SqlConnection::Exception::Base)
    {
        _E("Failed to handle StepAbortDBClean!");
    }
}

void TaskDatabase::StepAbortAceDBInsert()
{
    _W("[DB Update Task] ACE DB Aborting... (DB Clean)");

    FOREACH(iter, m_handleList) {
        ace_unregister_widget(static_cast<ace_widget_handle_t>(*iter));
    }

    FOREACH(iter, m_handleToRemoveList) {
        // Remove also old one. If it was already updated nothing wrong will happen,
        // but if not old widget will be removed.
        if (INVALID_WIDGET_HANDLE != *iter) {
            ace_unregister_widget(static_cast<ace_widget_handle_t>(*iter));
        }

        if (!AceApi::registerAceWidgetFromDB(*iter))
        {
            _E("ace database restore failed");
        }
    }
    _D("Ace data inserted");
}

void TaskDatabase::StepAbortWidgetInterfaceDBInsert()
{
    _D("[DB Update Task] Widget interface Aborting...");
    using namespace WidgetInterfaceDB;
    using namespace WrtDB;

    DbWidgetHandle handle =
        WidgetDAOReadOnly::getHandle(m_context.widgetConfig.tzAppid);
    std::string dbPath = WidgetInterfaceDAO::databaseFileName(handle);

    // remove database
    if (remove(dbPath.c_str()) != 0) {
        _W("Fail to remove");
    }

    // rollback database
    if (m_context.isUpdateMode) {
        std::string backupDbPath = dbPath;
        backupDbPath += GlobalConfig::GetBackupDatabaseSuffix();
        _D("\"%s\" to \"%s\"", dbPath.c_str(), backupDbPath.c_str());
        if (0 != std::rename(backupDbPath.c_str(), dbPath.c_str())) {
            _W("Fail to rollback");
        }
    }
}

#ifdef DBOX_ENABLED
void TaskDatabase::StepLiveboxDBInsert()
{
    if (m_context.widgetConfig.configInfo.m_livebox.size() <= 0) {
        return;
    }

    std::string tizenId = DPL::ToUTF8String(m_context.widgetConfig.tzAppid);

    // insert specific information to web livebox db
    for (auto it = m_context.widgetConfig.configInfo.m_livebox.begin();
         it != m_context.widgetConfig.configInfo.m_livebox.end(); ++it)
    {
        std::string boxId = DPL::ToUTF8String((**it).m_liveboxId);
        std::string boxType;
        if ((**it).m_type.empty()) {
            boxType = web_provider_livebox_get_default_type();
        } else {
            boxType = DPL::ToUTF8String((**it).m_type);
        }
        _D("livebox id: %s", boxId.c_str());
        _D("livebox type: %s", boxType.c_str());

        int autoLaunch = (**it).m_autoLaunch == L"true" ? 1 : 0;
        _D("livebox auto-launch: %d", autoLaunch);

        int mouseEvent = (**it).m_boxInfo.m_boxMouseEvent == L"true" ? 1 : 0;
        _D("livebox mouse-event: %d", mouseEvent);

        int pdFastOpen = (**it).m_boxInfo.m_pdFastOpen == L"true" ? 1 : 0;
        _D("livebox pd fast-open: %d", pdFastOpen);

        if (m_context.isUpdateMode) {
            web_provider_livebox_delete_by_app_id(tizenId.c_str());
        }
        web_provider_livebox_insert_box_info(
                boxId.c_str(), tizenId.c_str(), boxType.c_str(),
                autoLaunch, mouseEvent, pdFastOpen);
    }
}
#endif

void TaskDatabase::StartStep()
{
    LOGI("--------- <TaskDatabase> : START ----------");
}

void TaskDatabase::EndStep()
{
    LOGI("--------- <TaskDatabase> : END ----------");
}
} //namespace WidgetInstall
} //namespace Jobs
