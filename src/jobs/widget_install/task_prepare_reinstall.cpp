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
 * @file    task_prepare_reinstall.cpp
 * @author  Jihoon Chung(jihoon.chung@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task prepare reinstalling
 */

#include "task_prepare_reinstall.h"

#include <stdio.h>
#include <fstream>
#include <unistd.h>

#include <privilege-control.h>

#include <dpl/task.h>
#include <dpl/string.h>
#include <dpl/foreach.h>
#include <dpl/utils/wrt_utility.h>

#include <widget_install/widget_install_context.h>
#include <widget_install/widget_install_errors.h>
#include <widget_install/job_widget_install.h>

#include <installer_log.h>

namespace Jobs {
namespace WidgetInstall {
namespace {
const char* const KEY_DELETE = "#delete";
const char* const KEY_ADD = "#add";
const char* const KEY_MODIFY = "#modify";
std::list<std::string> keyList = {KEY_DELETE, KEY_ADD, KEY_MODIFY};

void verifyFile(const std::string &filePath)
{
    if (access(filePath.c_str(), F_OK) != 0) {
        ThrowMsg(Exceptions::RDSDeltaFailure, "File is missed " << filePath);
    }
}

std::string parseSubPath(const std::string& filePath)
{
    std::string subPath("");
    size_t pos = filePath.find_last_of('/') + 1;

    if (pos != std::string::npos) {
        subPath = filePath.substr(0, pos);
    }
    return subPath;
}

void createDir(const std::string& path)
{
    if (WrtUtilMakeDir(path)) {
        _D("Create directory : %s", path.c_str());
    } else {
        ThrowMsg(Exceptions::RDSDeltaFailure, "Fail to create dir" << path);
    }
}
} // namespace anonymous

TaskPrepareReinstall::TaskPrepareReinstall(InstallerContext& context) :
    DPL::TaskDecl<TaskPrepareReinstall>(this),
    m_context(context)
{
    AddStep(&TaskPrepareReinstall::StartStep);
    AddStep(&TaskPrepareReinstall::StepPrepare);
    AddStep(&TaskPrepareReinstall::StepParseRDSDelta);
    AddStep(&TaskPrepareReinstall::StepVerifyRDSDelta);
    AddStep(&TaskPrepareReinstall::StepAddFile);
    AddStep(&TaskPrepareReinstall::StepDeleteFile);
    AddStep(&TaskPrepareReinstall::StepModifyFile);
    AddStep(&TaskPrepareReinstall::StepUpdateSmackLabel);
    AddStep(&TaskPrepareReinstall::EndStep);
}

void TaskPrepareReinstall::StepPrepare()
{
    _D("Prepare");
    m_sourcePath = m_context.locations->getTemporaryPackageDir();
    m_sourcePath += "/";

    m_installedPath = m_context.locations->getPackageInstallationDir();
    m_installedPath += "/";
}

void TaskPrepareReinstall::StepParseRDSDelta()
{
    _D("parse RDS delta");
    std::string rdsDeltaPath = m_sourcePath;
    rdsDeltaPath += ".rds_delta";
    std::ifstream delta(rdsDeltaPath);

    if (!delta.is_open()) {
        ThrowMsg(Exceptions::RDSDeltaFailure, "rds_delta file is missed");
        return;
    }

    std::string line;
    std::string key;
    while (std::getline(delta, line) &&!delta.eof()) {
        FOREACH(keyIt, keyList) {
            if (line == *keyIt) {
                _D("find key = [%s]", line.c_str());
                key = line;
                break;
            }
        }
        if (key == line || line.empty() || line == "\n") {
            continue;
        }
        if (key == KEY_DELETE) {
            m_deleteFileList.push_back(line);
            _D("line = [%s]", line.c_str());
        } else if (key == KEY_ADD) {
            m_addFileList.push_back(line);
            _D("line = [%s]", line.c_str());
        } else if (key == KEY_MODIFY) {
            m_modifyFileList.push_back(line);
            _D("line = [%s]", line.c_str());
        }
    }
}

void TaskPrepareReinstall::StepVerifyRDSDelta()
{
    _D("verify RDS delta");
    // Verify ADD file
    FOREACH(file, m_addFileList) {
        std::string addFilePath = m_sourcePath;
        addFilePath += *file;
        verifyFile(addFilePath);
    }
    // Verify DELETE file
    FOREACH(file, m_deleteFileList) {
        std::string deleteFilePath = m_installedPath;
        deleteFilePath += *file;
        verifyFile(deleteFilePath);
    }
    // Verify MODIFY file
    FOREACH(file, m_modifyFileList) {
        std::string newFilePath = m_sourcePath;
        newFilePath += *file;
        verifyFile(newFilePath);

        std::string existingFilePath = m_installedPath;
        existingFilePath += *file;
        verifyFile(existingFilePath);
    }
    _D("Finished veify RDS Delta");

    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_RDS_DELTA_CHECK,
        "RDS delta verify finished");
}

void TaskPrepareReinstall::StepAddFile()
{
    _D("Add file");
    FOREACH(file, m_addFileList) {
        std::string newfile = m_sourcePath;
        newfile += *file;
        std::string destPath = m_installedPath;
        destPath += *file;

        if (WrtUtilDirExists(newfile)) {
            // In case of a new directory
            createDir(destPath);
        } else {
            // In case of a new file

            // Parse directory and file separately
            std::string subPath = parseSubPath(destPath);
            if (subPath.empty()) {
                ThrowMsg(Exceptions::RDSDeltaFailure,
                         "Invalid path given" << destPath);
            }

            // Create a new directory
            createDir(subPath);

            // Add file
            if (rename(newfile.c_str(), destPath.c_str()) != 0) {
                ThrowMsg(Exceptions::RDSDeltaFailure,
                        "Fail to add file " << newfile);
            }
            _D("Add %s to %s", newfile.c_str(), destPath.c_str());
        }
    }
}

void TaskPrepareReinstall::StepDeleteFile()
{
    _D("Delete file");
    FOREACH(file, m_deleteFileList) {
        std::string deleteFilePath = m_installedPath;
        deleteFilePath += *file;
        if (remove(deleteFilePath.c_str()) != 0) {
            ThrowMsg(Exceptions::RDSDeltaFailure,
                "Fail to DELETE file " << deleteFilePath);
        }
        _D("Delete %s", deleteFilePath.c_str());
    }
}

void TaskPrepareReinstall::StepModifyFile()
{
    _D("Modify  file");
    FOREACH(file, m_modifyFileList) {
        std::string destPath = m_installedPath;
        destPath += *file;
        if (remove(destPath.c_str()) != 0) {
            ThrowMsg(Exceptions::RDSDeltaFailure,
                "Fail to delete existing file " << destPath);
        }

        std::string newfile = m_sourcePath;
        newfile += *file;
        if (rename(newfile.c_str(), destPath.c_str()) != 0) {
            ThrowMsg(Exceptions::RDSDeltaFailure,
                "Fail to move new file" << destPath);
        }
        _D("Replace %s to %s", newfile.c_str(), destPath.c_str());
    }

    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_RDS_PREPARE,
        "RDS prepare finished");
}

void TaskPrepareReinstall::StepUpdateSmackLabel()
{
    /* res directory */
    std::string resDir = m_installedPath + "res";
    std::string pkgId = DPL::ToUTF8String(m_context.widgetConfig.tzPkgid);
    if (PC_OPERATION_SUCCESS != perm_app_setup_path(pkgId.c_str(), resDir.c_str(),
                APP_PATH_PRIVATE)) {
        _W("Add label to %s", resDir.c_str());
    }
}

void TaskPrepareReinstall::StartStep()
{
    LOGI("---------- <TaskPrepareReinstall> : START ----------");
}

void TaskPrepareReinstall::EndStep()
{
    LOGI("---------- <TaskPrepareReinstall> : END ----------");
    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_END,
        "End RDS update");
}

} //namespace WidgetInstall
} //namespace Jobs
