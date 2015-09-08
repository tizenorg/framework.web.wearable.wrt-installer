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
/*
 * @file    task_certify.cpp
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @version
 * @brief
 */

//SYSTEM INCLUDES
#include <cstring>
#include <string>
#include <sstream>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dpl/assert.h>
#include <pcrecpp.h>

//WRT INCLUDES
#include <widget_install/task_certify.h>
#include <widget_install/job_widget_install.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/wrt-dao-ro/widget_config.h>
#include "wac_widget_id.h"

#include <vcore/Certificate.h>
#include <vcore/SignatureReader.h>
#include <vcore/SignatureFinder.h>
#include <vcore/WrtSignatureValidator.h>
#include <pkgmgr-info.h>
#include <dpl/utils/wrt_global_settings.h>

#include <dpl/string.h>
#include <installer_log.h>

using namespace ValidationCore;
using namespace WrtDB;

namespace {
const int SIGNATURE_FILE_NUMBER_DISTRIBUTOR1 = 1;
const int SIGNATURE_FILE_NUMBER_DISTRIBUTOR2 = 2;

WidgetCertificateData toWidgetCertificateData(const SignatureData &data,
                                              bool root)
{
    WidgetCertificateData result;

    result.chainId = data.getSignatureNumber();
    _D("result.chainId : %d", result.chainId);

    result.owner = data.isAuthorSignature() ?
        WidgetCertificateData::AUTHOR : WidgetCertificateData::DISTRIBUTOR;

    if (data.isAuthorSignature()) {
        result.owner = WidgetCertificateData::AUTHOR;
    } else {
        if (SIGNATURE_FILE_NUMBER_DISTRIBUTOR1 == data.getSignatureNumber()) {
            result.owner = WidgetCertificateData::DISTRIBUTOR;
        } else if (SIGNATURE_FILE_NUMBER_DISTRIBUTOR2 ==
                data.getSignatureNumber()){
            result.owner = WidgetCertificateData::DISTRIBUTOR2;
        } else {
            result.owner = WidgetCertificateData::UNKNOWN;
        }
    }

    result.type = root ?
        WidgetCertificateData::ROOT : WidgetCertificateData::ENDENTITY;

    CertificatePtr certificate;

    if (root) {
        certificate = data.getRootCaCertificatePtr();
    } else {
        certificate = data.getEndEntityCertificatePtr();
    }

    AssertMsg(certificate && !certificate->getCommonName().empty(),
           "CommonName is Null");

    result.strCommonName = DPL::FromUTF8String(certificate->getCommonName());

    result.strMD5Fingerprint = std::string("md5 ") +
        Certificate::FingerprintToColonHex(
            certificate->getFingerprint(Certificate::FINGERPRINT_MD5));

    result.strSHA1Fingerprint = std::string("sha-1 ") +
        Certificate::FingerprintToColonHex(
            certificate->getFingerprint(Certificate::FINGERPRINT_SHA1));

    return result;
}

CertificatePtr getOldAuthorSignerCertificate(DPL::String appid)
{
    WidgetDAOReadOnly dao(appid);
    CertificateChainList chainList = dao.getWidgetCertificate(SIGNATURE_AUTHOR);

    FOREACH(it, chainList)
    {
        ValidationCore::CertificateCollection chain;
        if (false == chain.load(*it)) {
            _E("Chain is broken");
        }

        if (!chain.sort()) {
            _E("Chain failed at sorting");
        }

        ValidationCore::CertificateList list = chain.getCertificateList();

        FOREACH(cert, list)
        {
            if (!(*cert)->isRootCert() && !(*cert)->isCA()) {
                return *cert;
            }
        }
    }
    return CertificatePtr();
}
} // namespace anonymous

namespace Jobs {
namespace WidgetInstall {
TaskCertify::TaskCertify(InstallerContext &inCont) :
    DPL::TaskDecl<TaskCertify>(this),
    m_contextData(inCont)
{
    AddStep(&TaskCertify::StartStep);
    AddStep(&TaskCertify::stepSignature);
    // certi comparison determines whether the update.
    if (true == m_contextData.isUpdateMode) {
        AddStep(&TaskCertify::stepVerifyUpdate);
    }
    AddStep(&TaskCertify::stepCertifyLevel);
    AddStep(&TaskCertify::EndStep);
}

void TaskCertify::processDistributorSignature(const SignatureData &data)
{
    // this signature is verified -
    // no point in check domain WAC_ROOT and WAC_RECOGNIZED
    m_contextData.widgetSecurity.setDistributorSigned(true);

    CertificateCollection collection;
    collection.load(data.getCertList());
    AssertMsg(collection.sort(),
           "Certificate collection can't sort");

    AssertMsg(collection.isChain(),
           "Certificate collection is not able to create chain. "
           "It is not possible to verify this signature.");

    if (SIGNATURE_FILE_NUMBER_DISTRIBUTOR1 == data.getSignatureNumber()) {
        m_contextData.widgetSecurity.getCertificateChainListRef().push_back(
                collection);
    } else {
        m_contextData.widgetSecurity.getCertificateChainList2Ref().push_back(
                collection);
    }

    m_contextData.widgetSecurity.getCertificateListRef().push_back(
            toWidgetCertificateData(data, true));
    m_contextData.widgetSecurity.getCertificateListRef().push_back(
            toWidgetCertificateData(data, false));
}

void TaskCertify::processAuthorSignature(const SignatureData &data)
{
    using namespace ValidationCore;
    _D("DNS Identity match!");
    // this signature is verified or widget is distributor signed
    m_contextData.widgetSecurity.setAuthorCertificatePtr(data.getEndEntityCertificatePtr());
    CertificatePtr test = m_contextData.widgetSecurity.getAuthorCertificatePtr();

    m_contextData.widgetSecurity.getCertificateListRef().push_back(
        toWidgetCertificateData(data, true));
    m_contextData.widgetSecurity.getCertificateListRef().push_back(
        toWidgetCertificateData(data, false));

    // match widget_id with one from dns identity set
    WacWidgetId widgetId(m_contextData.widgetConfig.configInfo.widget_id);

    CertificatePtr cert = data.getEndEntityCertificatePtr();
    Assert(cert);
    Certificate::AltNameSet dnsIdentity = cert->getAlternativeNameDNS();

    CertificateCollection collection;
    collection.load(data.getCertList());
    collection.sort();
    AssertMsg(collection.isChain(),
           "Certificate collection is not able to create chain. "
           "It is not possible to verify this signature.");

    m_contextData.widgetSecurity.getAuthorsCertificateChainListRef().push_back(
        collection);

    FOREACH(it, dnsIdentity){
        if (widgetId.matchHost(DPL::FromASCIIString(*it))) {
            m_contextData.widgetSecurity.setRecognized(true);
            return;
        }
    }
}

void TaskCertify::getSignatureFiles(std::string path, SignatureFileInfoSet& file)
{
    _D("path : %s", path.c_str());
    SignatureFileInfoSet signatureFiles;
    SignatureFinder signatureFinder(path);
    if (SignatureFinder::NO_ERROR != signatureFinder.find(file)) {
        _E("Error in Signature Finder : %s", path.c_str());
        ThrowMsg(Exceptions::SignatureNotFound,
                "Error openig temporary widget directory");
    }
}

void TaskCertify::stepSignature()
{
    LOGD("================ Step: <<Signature>> ENTER ===============");

    std::string widgetPath;
    widgetPath = m_contextData.locations->getPackageInstallationDir() + "/";

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

            WrtSignatureValidator::Result result;

            WrtSignatureValidator validator(
                    WrtSignatureValidator::TIZEN,
                    !GlobalSettings::
                    OCSPTestModeEnabled(),
                    !GlobalSettings::
                    CrlTestModeEnabled(),
                    false);

            result = validator.check(data, widgetPath);

            if (m_contextData.mode.command == InstallMode::Command::RECOVERY
                    || m_contextData.mode.installTime == InstallMode::InstallTime::FOTA)
            {
                result = WrtSignatureValidator::SIGNATURE_VERIFIED;
            }

            if (result == WrtSignatureValidator::SIGNATURE_REVOKED) {
                _W("Certificate is REVOKED");
                ThrowMsg(Exceptions::CertificateExpired,
                         "Certificate is REVOKED");
            }

            if (result == WrtSignatureValidator::SIGNATURE_INVALID &&
                    iter->getFileNumber() <= 1) {
                _W("Signature is INVALID");
                // TODO change exception name
                ThrowMsg(Exceptions::SignatureInvalid,
                         "Invalid Package");
            }

            if (data.isAuthorSignature()) {
                if (result == WrtSignatureValidator::SIGNATURE_VERIFIED ) {
                    processAuthorSignature(data);
                }
            } else {
                if (result != WrtSignatureValidator::SIGNATURE_INVALID) {
                    processDistributorSignature(data);
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
            }
        } Catch(ParserSchemaException::Base) {
            _E("Error occured in ParserSchema.");
            // cannot rethrow ValidationCore::Exception as DPL::Exception.
            // let's just throw.
            ThrowMsg(Exceptions::SignatureInvalid,
                       "Error occured in ParserSchema.");
        } catch(...) {
            _E("Unknow exception occured in ParserSchema");
            ThrowMsg(Exceptions::SignatureInvalid,
                       "Error occured in ParserSchema.");
        }
        m_contextData.certLevel = level;
    }

    if (signatureFiles.empty()) {
        _D("No signature files has been found.");
        m_contextData.certLevel = level;
    }

    LOGD("================ Step: <<Signature>> DONE ================");

    m_contextData.job->UpdateProgress(
        InstallerContext::INSTALL_DIGSIG_CHECK,
        "Widget Signature checked");
}

std::string TaskCertify::enumToString(
    TaskCertify::Level level)
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

TaskCertify::Level TaskCertify::certTypeToLevel(
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

bool TaskCertify::isTizenWebApp() const
{
    bool ret = FALSE;

    if (m_contextData.widgetConfig.webAppType.appType == WrtDB::AppType::APP_TYPE_TIZENWEBAPP)
    {
        ret = TRUE;
    }

    return ret;
}

void TaskCertify::stepVerifyUpdate()
{
    LOGD("================ Step: <<VerifyUpdate>> ENTER ===============");

    std::string oldAuthorCert;

    int ret = 0;
    pkgmgrinfo_certinfo_h handle;
    const char *authorCert = NULL;
    ret = pkgmgrinfo_pkginfo_create_certinfo(&handle);
    if (PMINFO_R_OK == ret) {
        ret = pkgmgrinfo_pkginfo_load_certinfo(DPL::ToUTF8String(
                    m_contextData.widgetConfig.tzPkgid).c_str(), handle);
        if (PMINFO_R_OK == ret) {
            ret = pkgmgrinfo_pkginfo_get_cert_value(handle,
                    PMINFO_AUTHOR_SIGNER_CERT, &authorCert);
            if (PMINFO_R_OK == ret) {
                oldAuthorCert = (NULL != authorCert)? authorCert : "";
            }
        }
        pkgmgrinfo_pkginfo_destroy_certinfo(handle);
    }

    ValidationCore::CertificatePtr certPtr = m_contextData.widgetSecurity.getAuthorCertificatePtr();

    if (oldAuthorCert.empty()) {
        _D("Old cert is empty.");
        if (certPtr != NULL) {
            ThrowMsg(Exceptions::NotMatchedCertification,
                    "Author signer certificates doesn't match \
                    between old widget and installing widget");
        }
    } else {
        if (certPtr == NULL) {
            ThrowMsg(Exceptions::NotMatchedCertification, "No author certificates");
        }

        std::string newAuthorPublicKeyStr = certPtr->getPublicKeyString();
        //compare with public key
        ValidationCore::Certificate installedCert(oldAuthorCert , ValidationCore::Certificate::FORM_BASE64);
        std::string installedPublicKeyStr = installedCert.getPublicKeyString();
        if (0 != installedPublicKeyStr.compare(newAuthorPublicKeyStr)) {
            _D("old widget's author public key : %ls",
                    installedPublicKeyStr.c_str());
            _D("new widget's author public key: %ls",
                    newAuthorPublicKeyStr.c_str());
            ThrowMsg(Exceptions::NotMatchedCertification,
                    "Author signer certificates doesn't match \
                    between old widget and installing widget");
        }
    }
    LOGD("================ Step: <<VerifyUpdate>> DONE ===============");
}

void TaskCertify::stepCertifyLevel()
{
    LOGI("================ Step: <<Certify Level>> ENTER ===============");
    if (!checkConfigurationLevel(static_cast<TaskCertify::Level>(m_contextData.certLevel))) {
        ThrowMsg(Exceptions::PrivilegeLevelViolation, "setting level violate");
    }
    LOGI("================ Step: <<Certify Level>> DONE ================");
}

bool TaskCertify::checkConfigurationLevel(
    TaskCertify::Level level)
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

bool TaskCertify::checkSettingLevel(
    TaskCertify::Level level)
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

bool TaskCertify::checkAppcontrolHasDisposition(
    TaskCertify::Level level)
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

bool TaskCertify::checkNPRuntime(
    TaskCertify::Level level)
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

bool TaskCertify::checkServiceLevel(
    TaskCertify::Level level)
{
    if (m_contextData.widgetConfig.configInfo.serviceAppInfoList.size() > 0) {
        if (level < Level::PARTNER) {
            _E("\"tizen:service\" needs \"%s \" level", enumToString(Level::PARTNER).c_str());
            return false;
        }
    }
    return true;
}

void TaskCertify::StartStep()
{
    LOGI("--------- <TaskCertify> : START ----------");
}

void TaskCertify::EndStep()
{
    LOGD("Step: <<CERTYFYING DONE>>");

    m_contextData.job->UpdateProgress(
        InstallerContext::INSTALL_CERT_CHECK,
        "Widget Certification Check Finished");

    LOGI("--------- <TaskCertify> : END ----------");
}
} //namespace WidgetInstall
} //namespace Jobs

