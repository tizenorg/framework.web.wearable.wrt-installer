/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
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
/*
 * @file    task_certify_level.cpp
 * @author  Jihoon Chung (jihoon.chung@samgsung.com)
 * @version
 * @brief
 */

//SYSTEM INCLUDES
#include <string>
#include <map>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

//WRT INCLUDES
#include <widget_install/task_certify_level.h>
#include <widget_install/job_widget_install.h>
#include <widget_install/widget_install_errors.h>
#include <widget_install/widget_install_context.h>
#include <dpl/assert.h>
#include <dpl/exception.h>
#include <dpl/string.h>
#include <dpl/foreach.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/wrt-dao-ro/widget_config.h>

#include <vcore/CertStoreType.h>
#include <vcore/SignatureReader.h>
#include <vcore/SignatureFinder.h>
#include <vcore/WrtSignatureValidator.h>
#include <dpl/utils/wrt_global_settings.h>

#include <installer_log.h>

using namespace ValidationCore;
using namespace WrtDB;

namespace Jobs {
namespace WidgetInstall {
TaskCertifyLevel::TaskCertifyLevel(InstallerContext &inCont) :
    DPL::TaskDecl<TaskCertifyLevel>(this),
    m_contextData(inCont)
{
    AddStep(&TaskCertifyLevel::StartStep);
    AddStep(&TaskCertifyLevel::stepCertifyLevel);
    AddStep(&TaskCertifyLevel::EndStep);
}

void TaskCertifyLevel::stepCertifyLevel()
{
    LOGI("================ Step: <<Certify Level>> ENTER ===============");
    if (!checkConfigurationLevel(getCertifyLevel())) {
        ThrowMsg(Exceptions::PrivilegeLevelViolation, "setting level violate");
    }
    LOGI("================ Step: <<Certify Level>> DONE ================");
}

void TaskCertifyLevel::getSignatureFiles(const std::string& path,
                                         SignatureFileInfoSet& file)
{
    SignatureFileInfoSet signatureFiles;
    SignatureFinder signatureFinder(path);
    if (SignatureFinder::NO_ERROR != signatureFinder.find(file)) {
        _E("Error in Signature Finder : %s", path.c_str());
        ThrowMsg(Exceptions::SignatureNotFound, "Signature not found");
    }
}

TaskCertifyLevel::Level TaskCertifyLevel::getCertifyLevel()
{
    std::string widgetPath = m_contextData.locations->getPackageInstallationDir() + "/";
    SignatureFileInfoSet signatureFiles;

    Try {
        getSignatureFiles(widgetPath, signatureFiles);

        if (signatureFiles.size() <= 0) {
            widgetPath += std::string(WrtDB::GlobalConfig::GetWidgetSrcPath())
                + "/";
            if (0 == access(widgetPath.c_str(), F_OK)) {
                getSignatureFiles(widgetPath, signatureFiles);
            }
        }
    } Catch(Exceptions::SignatureNotFound) {
        ReThrowMsg(Exceptions::SignatureNotFound, widgetPath);
    }

    SignatureFileInfoSet::reverse_iterator iter = signatureFiles.rbegin();
    _D("Number of signatures: %d", signatureFiles.size());

    Level level = Level::UNKNOWN;
    for (; iter != signatureFiles.rend(); ++iter) {
        _D("Checking signature with id=%d", iter->getFileNumber());
        SignatureData data(widgetPath + iter->getFileName(),
                           iter->getFileNumber());

        Try {
            SignatureReader xml;
            xml.initialize(data, GlobalConfig::GetSignatureXmlSchema());
            xml.read(data);

            WrtSignatureValidator validator(
                    WrtSignatureValidator::TIZEN,
                    !GlobalSettings::
                    OCSPTestModeEnabled(),
                    !GlobalSettings::
                    CrlTestModeEnabled(),
                    false);

            WrtSignatureValidator::Result result =
                validator.check(data, widgetPath);

            if (m_contextData.mode.installTime == InstallMode::InstallTime::PRELOAD
                    || m_contextData.mode.command == InstallMode::Command::RECOVERY
                    || m_contextData.mode.installTime == InstallMode::InstallTime::FOTA)
            {
                result = WrtSignatureValidator::SIGNATURE_VERIFIED;
            }

            if (result == WrtSignatureValidator::SIGNATURE_REVOKED) {
                ThrowMsg(Exceptions::CertificateExpired,
                         "Certificate is REVOKED");
            }

            if (result == WrtSignatureValidator::SIGNATURE_INVALID &&
                    iter->getFileNumber() <= 1)
            {
                ThrowMsg(Exceptions::SignatureInvalid, "Invalid Package");
            }

            if (data.isAuthorSignature()) {
                _D("Skip author signature");
            } else {
                Level currentCertLevel =
                    certTypeToLevel(data.getVisibilityLevel());
                if (currentCertLevel == Level::UNKNOWN) {
                    continue;
                }
                if (currentCertLevel > level) {
                    level = currentCertLevel;
                    _D("level %s", enumToString(level).c_str());
                }
            }
        } Catch(ParserSchemaException::Base) {
            _E("Error occured in ParserSchema.");
            ReThrowMsg(Exceptions::SignatureInvalid,
                       "Error occured in ParserSchema.");
        }
    }

    m_contextData.certLevel = level;
    return level;
}

bool TaskCertifyLevel::checkConfigurationLevel(
    TaskCertifyLevel::Level level)
{
    if (!checkSettingLevel(level)) {
        return false;
    }
    if (!checkAppcontrolHasDisposition(level)) {
        return false;
    }
    if(!checkNPRuntime(level)){
        return false;
    }
    if(!checkServiceLevel(level)){
        return false;
    }
    return true;
}

bool TaskCertifyLevel::checkSettingLevel(
    TaskCertifyLevel::Level level)
{
    secureSettingMap data = {
        {"sound-mode", Level::PARTNER},
        {"nodisplay", Level::PARTNER},
        {"background-vibration", Level::PARTNER}
    };
    FOREACH(it, m_contextData.widgetConfig.configInfo.settingsList) {
        secureSettingIter ret = data.find(DPL::ToUTF8String(it->m_name));
        if (ret != data.end()) {
            if (level < ret->second) {
                _E("\"%ls\" needs \"%s\" level", it->m_name.c_str(), enumToString(ret->second).c_str());
                return false;
            }
        }
    }
    return true;
}

bool TaskCertifyLevel::checkAppcontrolHasDisposition(
    TaskCertifyLevel::Level level)
{
    // tizen:disposition -> platform
    FOREACH(it, m_contextData.widgetConfig.configInfo.appControlList) {
        if (ConfigParserData::AppControlInfo::Disposition::UNDEFINE !=
            it->m_disposition)
        {
            if (level < Level::PLATFORM) {
                _E("\"tizen:disposition\" needs \"%s \" level", enumToString(Level::PLATFORM).c_str());
                return false;
            }
        }
    }
    return true;
}

bool TaskCertifyLevel::checkNPRuntime(
    TaskCertifyLevel::Level level)
{
    DPL::String& pkgid = m_contextData.widgetConfig.tzPkgid;
    std::string npruntime_path = WrtDB::WidgetConfig::GetWidgetNPRuntimePluginsPath(pkgid);
    DIR *npruntime_dir = opendir(npruntime_path.c_str());

    if (npruntime_dir == NULL) {
        return true;
    }

    struct dirent dEntry;
    struct dirent *dEntryResult;
    int ret = 0;
    bool result = true;
    std::string suffix = ".so";

    do {
        struct stat statInfo;
        ret = readdir_r(npruntime_dir, &dEntry, &dEntryResult);
        if (dEntryResult != NULL && ret == 0) {
            std::string fileName = dEntry.d_name;
            std::string fullName = npruntime_path + "/" + fileName;

            if (stat(fullName.c_str(), &statInfo) != 0) {
                closedir(npruntime_dir);
                return false;
            }

            if (S_ISDIR(statInfo.st_mode)) {
                if (("." == fileName) || (".." == fileName)) {
                    continue;
                }
            } else {
                if(fileName.size() >= suffix.size() &&
                   fileName.compare(fileName.size() - suffix.size(), suffix.size(), suffix) == 0) {
                    result = level >= Level::PARTNER;
                    break;
                }
            }
        }
    } while (dEntryResult != NULL && ret == 0);
    closedir(npruntime_dir);
    if (!result) {
        LOGE("npruntime needs partner level");
    }
    return result;
}

bool TaskCertifyLevel::checkServiceLevel(
    TaskCertifyLevel::Level level)
{
    if (m_contextData.widgetConfig.configInfo.serviceAppInfoList.size() > 0) {
        if (level < Level::PARTNER) {
            _E("\"tizen:service\" needs \"%s \" level", enumToString(Level::PARTNER).c_str());
            return false;
        }
    }
    return true;
}

std::string TaskCertifyLevel::enumToString(
    TaskCertifyLevel::Level level)
{
    switch (level) {
#define X(x, y) case x: return #y;
        X(Level::UNKNOWN, UNKNOWN)
        X(Level::PUBLIC, PUBLIC)
        X(Level::PARTNER, PARTNER)
        X(Level::PLATFORM, PLATFORM)
#undef X
    default:
        return "UNKNOWN";
    }
}

TaskCertifyLevel::Level TaskCertifyLevel::certTypeToLevel(
    CertStoreId::Type type)
{
    // CertStoreType.h (framework/security/cert-svc)
    // RootCA's visibility level : public
    // const Type VIS_PUBLIC = 1 << 6;
    // RootCA's visibility level : partner
    // const Type VIS_PARTNER = 1 << 7;
    // RootCA's visibility level : platform
    // const Type VIS_PLATFORM = 1 << 10;
    if (type == CertStoreId::VIS_PUBLIC) {
        return Level::PUBLIC;
    } else if (type == CertStoreId::VIS_PARTNER) {
        return Level::PARTNER;
    } else if (type == CertStoreId::VIS_PLATFORM) {
        return Level::PLATFORM;
    }
    return Level::UNKNOWN;
}

void TaskCertifyLevel::StartStep()
{
    LOGI("--------- <TaskCertifyLevel> : START ----------");
}

void TaskCertifyLevel::EndStep()
{
    LOGI("--------- <TaskCertifyLevel> : END ----------");

    m_contextData.job->UpdateProgress(
        InstallerContext::INSTALL_CERTIFY_LEVEL_CHECK,
        "Application Certificate level check Finished");
}
} //namespace WidgetInstall
} //namespace Jobs

