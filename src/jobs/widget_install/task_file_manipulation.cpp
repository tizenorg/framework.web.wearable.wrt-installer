/*
 * Copyright (c) 2010 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file    task_db_update.cpp
 * @author  Lukasz Wrzosek(l.wrzosek@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task database updating
 */
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <fstream>
#include <vconf.h>

#include <widget_install/task_file_manipulation.h>
#include <widget_install/job_widget_install.h>
#include <widget_install/widget_install_errors.h>
#include <widget_install/widget_install_context.h>
#include <widget_install/directory_api.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/foreach.h>
#include <dpl/assert.h>
#include <dpl/errno_string.h>
#include <dpl/utils/folder_size.h>
#include <dpl/wrt-dao-ro/global_config.h>

#include <widget_install_to_external.h>
#include <installer_log.h>
#include <widget_unzip.h>

#define WEBAPP_DEFAULT_UID  5000
#define WEBAPP_DEFAULT_GID  5000

namespace {
const mode_t PRIVATE_STORAGE_MODE = 0700;
const mode_t SHARED_STORAGE_MODE = 0755;
}

using namespace WrtDB;

namespace {
const char* GLIST_RES_DIR = "res";

bool _FolderCopy(std::string source, std::string dest)
{
    DIR* dir = opendir(source.c_str());
    if (NULL == dir) {
        return false;
    }

    struct dirent dEntry;
    struct dirent *dEntryResult;
    int return_code;

    do {
        struct stat statInfo;
        return_code = readdir_r(dir, &dEntry, &dEntryResult);
        if (dEntryResult != NULL && return_code == 0) {
            std::string fileName = dEntry.d_name;
            std::string fullName = source + "/" + fileName;

            if (stat(fullName.c_str(), &statInfo) != 0) {
                closedir(dir);
                return false;
            }

            if (S_ISDIR(statInfo.st_mode)) {
                if (("." == fileName) || (".." == fileName)) {
                    continue;
                }
                std::string destFolder = dest + "/" + fileName;
                WrtUtilMakeDir(destFolder);

                if (!_FolderCopy(fullName, destFolder)) {
                    closedir(dir);
                    return false;
                }
            }

            std::string destFile = dest + "/" + fileName;
            std::ifstream infile(fullName);
            std::ofstream outfile(destFile);
            outfile << infile.rdbuf();
            outfile.close();
            infile.close();
        }
    } while (dEntryResult != NULL && return_code == 0);
    closedir(dir);
    return true;
}
}

namespace Jobs {
namespace WidgetInstall {
TaskFileManipulation::TaskFileManipulation(InstallerContext& context) :
    DPL::TaskDecl<TaskFileManipulation>(this),
    m_context(context),
    m_extHandle(NULL)
{
    AddStep(&TaskFileManipulation::StartStep);
    AddStep(&TaskFileManipulation::StepCheckInstallLocation);
    AddStep(&TaskFileManipulation::StepPrepareRootDirectory);
    if (m_context.mode.extension != InstallMode::ExtensionType::DIR)
    {
        AddStep(&TaskFileManipulation::StepUnzipWgtFile);
    }
    AddStep(&TaskFileManipulation::EndStep);

    AddAbortStep(&TaskFileManipulation::StepAbortPrepareRootDirectory);
}

void TaskFileManipulation::StepCheckInstallLocation()
{
    _D("StepCheckInstallLocation");
    if (m_context.mode.rootPath == InstallMode::RootPath::RO) {
        m_context.locationType = INSTALL_LOCATION_TYPE_INTERNAL_ONLY;
        return;
    }

    // If webapp is hybrid app, it should be installed to internal storage.
    // Because Service app should be installed to internal.
    if (m_context.widgetConfig.packagingType == PKG_TYPE_HYBRID_WEB_APP) {
        m_context.locationType = INSTALL_LOCATION_TYPE_INTERNAL_ONLY;
        return;
    }

    std::string installedPath = WrtDB::GlobalConfig::GetUserInstalledWidgetPath();
    WidgetUnzip wgtUnzip(m_context.requestedPath);

    if (m_context.locationType == INSTALL_LOCATION_TYPE_AUTO ||
        m_context.locationType == INSTALL_LOCATION_TYPE_UNKNOWN) {
        int storage = 0;
        // vconf_get_int(VCONFKEY_SETAPPL_DEFAULT_MEM_INSTALL_APPLICATIONS_INT)
        // 0 : phone internal memory
        // 1 : SD card
        if (vconf_get_int(VCONFKEY_SETAPPL_DEFAULT_MEM_INSTALL_APPLICATIONS_INT,
                    &storage)) {
            _E("vconf_get_int(VCONFKEY_SETAPPL_DEFAULT_MEM_INSTALL_APPLICATIONS_INT) \
                    failed.");
        }
        _D("default setting : storage [%d]", storage);
        if (storage) {
            m_context.locationType = INSTALL_LOCATION_TYPE_PREFER_EXTERNAL;
        } else {
            m_context.locationType = INSTALL_LOCATION_TYPE_INTERNAL_ONLY;
            if(!wgtUnzip.checkAvailableSpace(installedPath)) {
                m_context.locationType = INSTALL_LOCATION_TYPE_PREFER_EXTERNAL;
            }
        }
    }

    if (m_context.locationType == INSTALL_LOCATION_TYPE_PREFER_EXTERNAL) {
        int mmcStatus;
        if (vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &mmcStatus)) {
            _E("vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS) failed.");
            mmcStatus = VCONFKEY_SYSMAN_MMC_INSERTED_NOT_MOUNTED;
        }

        if (VCONFKEY_SYSMAN_MMC_MOUNTED != mmcStatus) {
            _D("mmcStatus is MMC_REMOVED or NOT_MOUNTED.");
            m_context.locationType = INSTALL_LOCATION_TYPE_INTERNAL_ONLY;
        }
    }

    if (m_context.locationType == INSTALL_LOCATION_TYPE_INTERNAL_ONLY) {
        if(!wgtUnzip.checkAvailableSpace(installedPath)) {
            ThrowMsg(Exceptions::OutOfStorageFailed, "There is no space for installation");
        }
    }
}

void TaskFileManipulation::StepPrepareRootDirectory()
{
    if (m_context.locationType == INSTALL_LOCATION_TYPE_PREFER_EXTERNAL) {
        prepareExternalDir();
    } else {
        std::string widgetPath = m_context.locations->getPackageInstallationDir();
        std::string widgetBinPath = m_context.locations->getBinaryDir();
        std::string widgetSrcPath = m_context.locations->getSourceDir();

        WrtUtilMakeDir(widgetPath);

        _D("Create resource directory");
        WrtUtilMakeDir(widgetBinPath);
        WrtUtilMakeDir(widgetSrcPath);
    }

    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_DIR_CREATE,
        "Widget Directory Created");
}

void TaskFileManipulation::StepUnzipWgtFile()
{
    if (m_context.widgetConfig.packagingType != PKG_TYPE_HOSTED_WEB_APP) {
        std::string instDir;
        if (m_context.widgetConfig.packagingType == PKG_TYPE_HYBRID_WEB_APP) {
            instDir = m_context.locations->getPackageInstallationDir();
        } else {
            instDir = m_context.locations->getSourceDir();
        }

        _D("unzip file to %s", instDir.c_str());

        WidgetUnzip wgtUnzip(m_context.requestedPath);
        wgtUnzip.unzipWgtFile(instDir);
    } else {
        _D("From browser installation - unzip is not done");
    }

    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_UNZIP_WGT,
        "Unzip Wgt file");
}

void TaskFileManipulation::StepAbortPrepareRootDirectory()
{
    _D("[Create Root Directory]  Aborting.... (Rename path)");
    if (m_context.locationType == INSTALL_LOCATION_TYPE_PREFER_EXTERNAL) {
        if (m_context.isUpdateMode) {
            WidgetInstallToExtSingleton::Instance().postUpgrade(false);
        } else {
            WidgetInstallToExtSingleton::Instance().postInstallation(false);
        }
        WidgetInstallToExtSingleton::Instance().deinitialize();
    } else {
        std::string widgetPath;
        widgetPath = m_context.locations->getPackageInstallationDir();
        if (!WrtUtilRemove(widgetPath)) {
            _E("Error occurs during removing existing folder");
        }
        // Remove user data directory if preload web app.
        std::string userData = m_context.locations->getUserDataRootDir();
        if (0 == access(userData.c_str(), F_OK)) {
            if (!WrtUtilRemove(userData)) {
                _E("Error occurs during removing user data directory");
            }
        }
    }
}

void TaskFileManipulation::prepareExternalDir()
{
    _D("Step prepare to install in exernal directory");
    Try {
        std::string pkgid =
            DPL::ToUTF8String(m_context.widgetConfig.tzPkgid);

        WidgetInstallToExtSingleton::Instance().initialize(pkgid);

        std::unique_ptr<DPL::ZipInput> zipFile(new
                DPL::ZipInput(m_context.requestedPath));
        double unzipSize = zipFile->GetTotalUncompressedSize();
        int folderSize = (int)(unzipSize / (1024 * 1024)) + 1;

        GList *list = NULL;
        app2ext_dir_details* dirDetail = NULL;

        dirDetail = (app2ext_dir_details*) calloc(1,
                sizeof(
                    app2ext_dir_details));
        if (NULL == dirDetail) {
            ThrowMsg(Exceptions::ErrorExternalInstallingFailure,
                    "error in app2ext");
        }
        dirDetail->name = strdup(GLIST_RES_DIR);
        dirDetail->type = APP2EXT_DIR_RO;
        list = g_list_append(list, dirDetail);

        if (m_context.isUpdateMode) {
            WidgetInstallToExtSingleton::Instance().preUpgrade(list,
                                                               folderSize);
        } else {
            WidgetInstallToExtSingleton::Instance().preInstallation(list,
                                                                    folderSize);
        }
        free(dirDetail);
        g_list_free(list);

        /* make bin directory */
        std::string widgetBinPath = m_context.locations->getBinaryDir();
        WrtUtilMakeDir(widgetBinPath);
        std::string sourceDir = m_context.locations->getSourceDir();
        WrtUtilMakeDir(sourceDir);
    }
    Catch(DPL::ZipInput::Exception::OpenFailed) {
        ReThrowMsg(Exceptions::ErrorExternalInstallingFailure,
                   "Error during \
                create external folder ");
    }
    Catch(WidgetInstallToExt::Exception::ErrorInstallToExt)
    {
        ReThrowMsg(Exceptions::ErrorExternalInstallingFailure,
                   "Error during create external folder ");
    }
}

void TaskFileManipulation::StartStep()
{
    LOGI("--------- <TaskFileManipulation> : START ----------");
}

void TaskFileManipulation::EndStep()
{
    LOGI("--------- <TaskFileManipulation> : END ----------");
}
} //namespace WidgetInstall
} //namespace Jobs
