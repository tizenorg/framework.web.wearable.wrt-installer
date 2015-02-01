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
 * @file    task_remove_backup.cpp
 * @author  Soyoung kim(sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task backup files remove
 */
#include <widget_install/task_remove_backup.h>

#include <sys/stat.h>
#include <string>
#include <sstream>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/foreach.h>
#include <dpl/assert.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/wrt-dao-rw/widget_dao.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <widget_install/job_widget_install.h>
#include <widget_install/widget_install_errors.h>
#include <widget_install/widget_install_context.h>
#include <wrt-commons/widget-interface-dao/widget_interface_dao.h>
#include <ace_api_install.h>
#include <installer_log.h>

using namespace WrtDB;

namespace Jobs {
namespace WidgetInstall {
TaskRemoveBackupFiles::TaskRemoveBackupFiles(InstallerContext& context) :
    DPL::TaskDecl<TaskRemoveBackupFiles>(this),
    m_context(context)
{
    AddStep(&TaskRemoveBackupFiles::StartStep);
    if (m_context.mode.extension != InstallMode::ExtensionType::DIR)
    {
        AddStep(&TaskRemoveBackupFiles::StepRemoveBackupFiles);
    }
    AddStep(&TaskRemoveBackupFiles::StepDeleteBackupDB);
    AddStep(&TaskRemoveBackupFiles::StepDeleteBackupWidgetInterfaceDB);
    AddStep(&TaskRemoveBackupFiles::EndStep);
}

void TaskRemoveBackupFiles::StepRemoveBackupFiles()
{
    std::ostringstream backupDir;
    backupDir << m_context.locations->getBackupDir();

    if (WrtUtilRemove(backupDir.str())) {
        _D("Success to remove backup files : %s", backupDir.str().c_str());
    } else {
        _E("Failed to remove backup directory : %s", backupDir.str().c_str());
        ThrowMsg(Exceptions::RemoveBackupFailed,
                 "Error occurs during removing existing folder");
    }

    std::string tmp = m_context.locations->getTemporaryPackageDir();
    if (WrtUtilRemove(tmp)) {
        _D("Success to remove temp directory : %s", tmp.c_str());
    } else {
        _E("Failed to remove temp directory : %s", tmp.c_str());
    }
}

void TaskRemoveBackupFiles::StepDeleteBackupDB()
{
    _D("StepDeleteBackupDB");

    std::list<TizenAppId> idList = WidgetDAOReadOnly::getTzAppIdList(m_context.widgetConfig.tzPkgid);
    FOREACH( it, idList ){
        Try
        {
                DPL::String suffix = L".backup";
                if( it->size() >= suffix.size() && it->compare(it->size() - suffix.size() , suffix.size(), suffix) == 0)
                    WidgetDAO::unregisterWidget(*it);
        }
        Catch(WidgetDAOReadOnly::Exception::WidgetNotExist)
        {
            _E("Fail to delete old version db information");
        }
    }
}

void TaskRemoveBackupFiles::StepDeleteBackupWidgetInterfaceDB()
{
    _D("StepDeleteBackupWidgetInterfaceDB");
    using namespace WidgetInterfaceDB;
    using namespace WrtDB;

    DbWidgetHandle handle =
        WidgetDAOReadOnly::getHandle(m_context.widgetConfig.tzAppid);
    std::string backupDbPath = WidgetInterfaceDAO::databaseFileName(handle);
    backupDbPath += GlobalConfig::GetBackupDatabaseSuffix();

    // remove backup database
    if (remove(backupDbPath.c_str()) != 0) {
        _W("Fail to remove");
    }
}

void TaskRemoveBackupFiles::StartStep()
{
    LOGI("--------- <TaskRemoveBackupFiles> : START ----------");
}

void TaskRemoveBackupFiles::EndStep()
{
    LOGI("--------- <TaskRemoveBackupFiles> : END ----------");
}
} //namespace WidgetInstall
} //namespace Jobs
