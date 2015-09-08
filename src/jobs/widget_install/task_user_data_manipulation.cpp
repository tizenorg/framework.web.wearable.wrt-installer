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
 * @file    task_user_data_manipulation.cpp
 * @author  Soyoung Kim (sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task user data folder 
 */
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <widget_install/task_user_data_manipulation.h>
#include <widget_install/job_widget_install.h>
#include <widget_install/widget_install_errors.h>
#include <widget_install/widget_install_context.h>
#include <widget_install/directory_api.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/assert.h>
#include <dpl/errno_string.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <string>
#include <installer_log.h>

#define WEBAPP_DEFAULT_UID  5000
#define WEBAPP_DEFAULT_GID  5000

namespace {
const mode_t PRIVATE_STORAGE_MODE = 0700;
const mode_t SHARED_STORAGE_MODE = 0755;
}

using namespace WrtDB;

namespace {
void changeOwnerForDirectory(std::string storagePath, mode_t mode) {
    if (euidaccess(storagePath.c_str(), F_OK) != 0) {
        if (!WrtUtilMakeDir(storagePath, mode)) {
            _E("Failed to create directory : %s", storagePath.c_str());
            ThrowMsg(Jobs::WidgetInstall::Exceptions::FileOperationFailed,
                     "Failed to create directory : " << storagePath);
        }
        // '5000' is default uid, gid for applications.
        // So installed applications should be launched as process of uid
        // '5000'.
        // the process can access private directory 'data' of itself.
        if (chown(storagePath.c_str(),
                  WEBAPP_DEFAULT_UID,
                  WEBAPP_DEFAULT_GID) != 0)
        {
            ThrowMsg(Jobs::WidgetInstall::Exceptions::FileOperationFailed,
                     "Chown to invaild user");
        }
    } else if (euidaccess(storagePath.c_str(), W_OK | R_OK | X_OK) == 0) {
        _D("%s already exists.", storagePath.c_str());
        // Even if private directory already is created, private dircetory
        // should change owner (recursively).
        if (chown(storagePath.c_str(),
                  WEBAPP_DEFAULT_UID,
                  WEBAPP_DEFAULT_GID) != 0)
        {
            ThrowMsg(Jobs::WidgetInstall::Exceptions::FileOperationFailed,
                     "Chown to invaild user");
        }
        if (chmod(storagePath.c_str(), mode) != 0) {
            ThrowMsg(Jobs::WidgetInstall::Exceptions::FileOperationFailed,
                     "chmod to 0700");
        }
    } else {
        ThrowMsg(Jobs::WidgetInstall::Exceptions::FileOperationFailed,
                 "No access to private storage.");
    }
}
}

namespace Jobs {
namespace WidgetInstall {
TaskUserDataManipulation::TaskUserDataManipulation(InstallerContext& context) :
    DPL::TaskDecl<TaskUserDataManipulation>(this),
    m_context(context)
{
    AddStep(&TaskUserDataManipulation::StartStep);
    AddStep(&TaskUserDataManipulation::StepCreatePrivateStorageDir);
    AddStep(&TaskUserDataManipulation::StepCreateSharedFolder);
    AddStep(&TaskUserDataManipulation::StepLinkForPreload);
    AddStep(&TaskUserDataManipulation::EndStep);
}

void TaskUserDataManipulation::StepCreatePrivateStorageDir()
{

    if (m_context.mode.installTime == InstallMode::InstallTime::FOTA
        && m_context.isUpdateMode)
    {
        return;
    }

    if (m_context.mode.installTime == InstallMode::InstallTime::PRELOAD
            || m_context.mode.installTime == InstallMode::InstallTime::FOTA) {
        std::string userWidgetDir = m_context.locations->getUserDataRootDir();
        _D("Create user data directory : %s", userWidgetDir.c_str());
        WrtUtilMakeDir(userWidgetDir);
    }
    std::string storagePath = m_context.locations->getPrivateStorageDir();
    _D("Create private storage directory : %s", m_context.locations->getPrivateStorageDir().c_str());

    changeOwnerForDirectory(storagePath, PRIVATE_STORAGE_MODE);

    if (m_context.isUpdateMode) { //update
        std::string backData = m_context.locations->getBackupPrivateDir();
        _D("copy private storage %s to %s", backData.c_str(), storagePath.c_str());
        if (!DirectoryApi::DirectoryCopy(backData, storagePath)) {
            _E("Failed to rename %s to %s", backData.c_str(), storagePath.c_str());
            ThrowMsg(Exceptions::BackupFailed,
                    "Error occurs copy private strage files");
        }
    }

    std::string tempStoragePath = m_context.locations->getPrivateTempStorageDir();
    _D("Create temp private storage directory : %s", tempStoragePath.c_str());
    changeOwnerForDirectory(tempStoragePath, PRIVATE_STORAGE_MODE);

    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_CREATE_PRIVATE_STORAGE,
        "Create private storage for user");
}

void TaskUserDataManipulation::StepLinkForPreload()
{
    /* link for resource directory  */
    if (m_context.mode.rootPath == InstallMode::RootPath::RO) {
        std::string optRes = m_context.locations->getUserDataRootDir() +
            WrtDB::GlobalConfig::GetWidgetResPath();
        std::string usrRes = m_context.locations->getPackageInstallationDir() +
            WrtDB::GlobalConfig::GetWidgetResPath();

        if (0 == access(optRes.c_str(), F_OK)) {
            if (!WrtUtilRemove(optRes.c_str())) {
                _E("Failed to remove %s", optRes.c_str());
            }
        }
        _D("Make symbolic name for preload app %s to %s", usrRes.c_str(), optRes.c_str());
        if (symlink(usrRes.c_str(), optRes.c_str()) != 0)
        {
            int error = errno;
            if (error)
                _E("Failed to make a symbolic name for a file [%s]", (DPL::GetErrnoString(error)).c_str());
            ThrowMsg(Exceptions::FileOperationFailed,
                    "Symbolic link creating is not done.");
        }

        /* link for data directory */
        std::string storagePath = m_context.locations->getPrivateStorageDir();
        std::string dataDir = m_context.locations->getPackageInstallationDir() +
            "/" + WrtDB::GlobalConfig::GetWidgetPrivateStoragePath();
        if (0 != access(dataDir.c_str(), F_OK)) {
            _D("Make symbolic name for preload app %s to %s", storagePath.c_str(), dataDir.c_str());
            if (symlink(storagePath.c_str(), dataDir.c_str()) != 0)
            {
                int error = errno;
                if (error)
                    _E("Failed to make a symbolic name for a file [%s]", (DPL::GetErrnoString(error)).c_str());
                ThrowMsg(Exceptions::FileOperationFailed,
                        "Symbolic link creating is not done.");
            }
            changeOwnerForDirectory(dataDir, PRIVATE_STORAGE_MODE);
        }

        /* link for bin directory  */
        if (m_context.widgetConfig.packagingType != PKG_TYPE_HYBRID_WEB_APP) {
            std::string widgetBinPath = m_context.locations->getBinaryDir();
            std::string userBinPath = m_context.locations->getUserBinaryDir();

            if (0 == access(userBinPath.c_str(), F_OK)) {
                if (!WrtUtilRemove(userBinPath.c_str())) {
                    _E("Failed To rename %s", userBinPath.c_str());
                }
            }
            _D("Make symbolic link for preload app %s to %s", widgetBinPath.c_str(), userBinPath.c_str());
            if (symlink(widgetBinPath.c_str(), userBinPath.c_str()) != 0)
            {
                int error = errno;
                if (error)
                    _E("Failed to make a symbolic name for a file [%s]", (DPL::GetErrnoString(error)).c_str());
                ThrowMsg(Exceptions::FileOperationFailed,
                        "Symbolic link creating is not done.");
            }

        }
    }
}

void TaskUserDataManipulation::StepCreateSharedFolder()
{
    if (m_context.mode.installTime == InstallMode::InstallTime::FOTA
        && m_context.isUpdateMode)
    {
        return;
    }

    _D("StepCreateSharedFolder");
    std::string sharedPath = m_context.locations->getSharedRootDir();
    _D("Create shared directory : %s", m_context.locations->getSharedRootDir().c_str());

    WrtUtilMakeDir(sharedPath);
    WrtUtilMakeDir(m_context.locations->getSharedResourceDir());

    changeOwnerForDirectory(m_context.locations->getSharedDataDir(),
            SHARED_STORAGE_MODE);
    changeOwnerForDirectory(m_context.locations->getSharedTrustedDir(),
            SHARED_STORAGE_MODE);


    // additional check for rootPath installation
    // If this app is preloaded, "shared" diretory is already on place and do not needs to be moved
    // TODO: why "shared" is on RW partion always but "data" and "tmp" are linked
    if (m_context.isUpdateMode
            && !(m_context.mode.rootPath == InstallMode::RootPath::RO
            && m_context.mode.installTime == InstallMode::InstallTime::PRELOAD)) {

        /* Restore /shared/data */
        _D("copy %s to %s", m_context.locations->getBackupSharedDataDir().c_str(), m_context.locations->getSharedDataDir().c_str());
        if (!DirectoryApi::DirectoryCopy(
                    m_context.locations->getBackupSharedDataDir(),
                    m_context.locations->getSharedDataDir())) {
                _E("Failed to rename %s to %s", m_context.locations->getBackupSharedDataDir().c_str(), m_context.locations->getSharedDataDir().c_str());
                ThrowMsg(Exceptions::BackupFailed,
                        "Error occurs copy shared strage files");
            }

        /* Restore /shared/trusted */
        _D("copy %s to %s", m_context.locations->getBackupSharedTrustedDir().c_str(), m_context.locations->getSharedTrustedDir().c_str());
        if (!DirectoryApi::DirectoryCopy(
                    m_context.locations->getBackupSharedTrustedDir(),
                    m_context.locations->getSharedTrustedDir())) {
            _E("Failed to rename %s to %s", m_context.locations->getBackupSharedTrustedDir().c_str(), m_context.locations->getSharedTrustedDir().c_str());
            ThrowMsg(Exceptions::BackupFailed,
                    "Error occurs copy shared strage files");
        }
    }
}

void TaskUserDataManipulation::StartStep()
{
    LOGI("--------- <TaskUserDataManipulation> : START ----------");
}

void TaskUserDataManipulation::EndStep()
{
    LOGI("--------- <TaskUserDataManipulation> : END ----------");
}
} //namespace WidgetInstall
} //namespace Jobs
