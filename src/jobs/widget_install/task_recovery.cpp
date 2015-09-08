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
 * @file    task_recovery.cpp
 * @author  Soyoung Kim (sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task recovery
 */
#include "task_recovery.h"

#include <dpl/log/log.h>
#include <dpl/errno_string.h>
#include <dpl/foreach.h>

#include <dpl/wrt-dao-ro/widget_config.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/wrt-dao-rw/widget_dao.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/utils/path.h>
#include <ace_api_install.h>
#include <ace_registration.h>
#include <ace-common/ace_api_common.h>

#include <widget_install/job_widget_install.h>
#include <widget_install/widget_install_context.h>
#include <widget_install/widget_install_errors.h>
#include <installer_log.h>

using namespace WrtDB;

namespace {
const std::string BACKUP_ID = ".backup";
}

namespace Jobs {
namespace WidgetInstall {
TaskRecovery::TaskRecovery(InstallerContext& context) :
    DPL::TaskDecl<TaskRecovery>(this),
    m_context(context)
{
    AddStep(&TaskRecovery::StartStep);
    AddStep(&TaskRecovery::StepRecoveryDirectory);
    AddStep(&TaskRecovery::StepRecoveryDatabase);
    AddStep(&TaskRecovery::EndStep);
}

void TaskRecovery::StepRecoveryDirectory()
{
    _D("StepRecoveryDirectory ...");
    // check backup folder
    DPL::Utils::Path installedPath(WrtDB::GlobalConfig::GetUserInstalledWidgetPath());
    installedPath /= m_context.widgetConfig.tzPkgid;

    DPL::Utils::Path backupPath(WrtDB::GlobalConfig::GetUserInstalledWidgetPath());
    backupPath /= DPL::ToUTF8String(m_context.widgetConfig.tzPkgid) + BACKUP_ID;

    _D("installedPath : %s", installedPath.Fullpath().c_str());
    _D("backupPath : %s", backupPath.Fullpath().c_str());

    if (backupPath.Exists()) {
        DPL::Utils::TryRemove(installedPath);

        DPL::Utils::Rename(backupPath, installedPath);
    }
}

void TaskRecovery::StepRecoveryDatabase()
{
    _D("StepRecoveryDatabase ... %s", m_context.widgetConfig.tzPkgid.c_str());
    Try {
        std::string backupId, deleteId;

        TizenAppId dbAppId = WidgetDAOReadOnly::getTizenAppId(m_context.widgetConfig.tzPkgid);
        _D("Get appid : %ls", dbAppId.c_str());
        std::string appId = DPL::ToUTF8String(dbAppId);

        if (0 == appId.compare(appId.size() - BACKUP_ID.length(), BACKUP_ID.length(), BACKUP_ID)) {
            backupId = appId;
            deleteId = backupId.substr(0, backupId.length() -
                    BACKUP_ID.length());
        } else {
            backupId = appId + BACKUP_ID;
            deleteId = appId;
        }

        if (WrtDB::WidgetDAOReadOnly::isWidgetInstalled(DPL::FromUTF8String(backupId))) {
            _D("Recovery Database...");
            _D("backupId %s " , backupId.c_str());
            _D("deleteId %s " , deleteId.c_str());

            // remove ace
            ace_unregister_widget(static_cast<ace_widget_handle_t>(
                        WidgetDAOReadOnly::getHandle(DPL::
                            FromUTF8String(deleteId))));
            ace_unregister_widget(static_cast<ace_widget_handle_t>(
                        WidgetDAOReadOnly::getHandle(DPL::
                            FromUTF8String(backupId))));

            WidgetDAO::unregisterWidget(DPL::FromUTF8String(deleteId));
            WidgetDAO::updateTizenAppId(DPL::FromUTF8String(backupId),
                    DPL::FromUTF8String(deleteId));

            if(!AceApi::registerAceWidgetFromDB(WidgetDAOReadOnly::getHandle(
                            DPL::FromUTF8String(deleteId)))) {
                _E("ace database restore failed");
            }
        }

        WidgetDAOReadOnly dao(DPL::FromUTF8String(deleteId));
        m_context.requestedPath =
            DPL::ToUTF8String(*dao.getWidgetInstalledPath());
    }
    Catch(WidgetDAOReadOnly::Exception::WidgetNotExist)
    {
        ThrowMsg(Exceptions::RecoveryFailed, "[WidgetNotExist] Failure in recovery db");
    }
    Catch(WidgetDAO::Exception::DatabaseError)
    {
        ThrowMsg(Exceptions::RecoveryFailed, "[DatabaseError] Failure in recovery db");
    }
}

void TaskRecovery::StartStep()
{
    LOGI("--------- <TaskRecovery> : START ----------");
}

void TaskRecovery::EndStep()
{
    LOGI("--------- <TaskRecovery> : END ----------");
}
} //namespace RecoveryInstall
} //namespace Jobs
