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
 * @author      Yunchan Cho (yunchan.cho@samsung.com)
 * @version     0.1
 * @brief
 */

#include <dpl/lexical_cast.h>
#include <dpl/wrt-dao-ro/global_config.h>

#include <pkgmgr_installer.h>
#include <pkg-manager/pkgmgr_signal.h>
#include <installer_log.h>

namespace {
// package type sent in every signal
const char PKGMGR_WEBAPP_TYPE[] = "wgt";

// notification about opoeration start
const char PKGMGR_START_KEY[] = "start";

// value for new installation
const char PKGMGR_START_INSTALL[] = "install";

// value for update installation
const char PKGMGR_START_UPDATE[] = "update";

// value for uninstallation
const char PKGMGR_START_UNINSTALL[] = "uninstall";

// notification about progress of installation with percentage number
const char PKGMGR_PROGRESS_KEY[] = "install_percent";

// notification about icon path for installation frontend
const char PKGMGR_ICON_PATH[] = "icon_path";

// notification about error before end with given error code
// (currently, same as backend exit status)
const char PKGMGR_ERROR[] = "error";

// notification about end of installation with status
const char PKGMGR_END_KEY[] = "end";

// success value of end of installation
const char PKGMGR_END_SUCCESS[] = "ok";

// failure value of end of installation
const char PKGMGR_END_FAILURE[] = "fail";
}

namespace PackageManager {
PkgmgrSignal::PkgmgrSignal() :
    m_initialized(false),
    m_handle(NULL),
    m_reqType(RequestType::UNSUPPORTED),
    m_percent(0)
{}

PkgmgrSignal::~PkgmgrSignal()
{
    deinitialize();
}

bool PkgmgrSignal::initialize(int argc, char* argv[])
{
    if (m_handle) {
        _D("Release already allocated pkgmgr handle");
        pkgmgr_installer_free(m_handle);
        m_handle = NULL;
    }

    m_handle = pkgmgr_installer_new();
    if (!m_handle) {
        _E("Fail to get pkgmgr installer handle");
        return false;
    }

    // set information from pkgmgr
    if (!pkgmgr_installer_receive_request(
            m_handle, argc, argv))
    {
        auto pkgmgrtype = pkgmgr_installer_get_request_type(m_handle);
        switch(pkgmgrtype)
        {
            case PKGMGR_REQ_INSTALL:
                m_reqType = RequestType::INSTALL;
                break;
            case PKGMGR_REQ_UNINSTALL:
                m_reqType = RequestType::UNINSTALL;
                break;
            case PKGMGR_REQ_REINSTALL:
                m_reqType = RequestType::REINSTALL;
                break;
            default:
                m_reqType = RequestType::UNSUPPORTED;
                break;
        }

        if (m_reqType == RequestType::UNSUPPORTED)
        {
            _E("Fail to get request type of pkgmgr");
            pkgmgr_installer_free(m_handle);
            m_handle = NULL;
            return false;
        }
        const char *callerId = pkgmgr_installer_get_caller_pkgid(m_handle);
        if(callerId)
            m_callerId = callerId;

    } else {
        _E("Fail to get information of pkgmgr's request");
        pkgmgr_installer_free(m_handle);
        m_handle = NULL;
        return false;
    }

    m_type = PKGMGR_WEBAPP_TYPE;
    m_initialized = true;
    return true;
}

bool PkgmgrSignal::deinitialize()
{
    if (!m_initialized) {
        _E("PkgmgrSingal not yet intialized");
        return false;
    }

    if (!m_recoveryFile.empty() && (0 != unlink(m_recoveryFile.c_str()))) {
        _E("Failed to remove %s", m_recoveryFile.c_str());
    }

    if (m_handle) {
        pkgmgr_installer_free(m_handle);
    }

    m_handle = NULL;
    m_initialized = false;
    return true;
}

bool PkgmgrSignal::setPkgname(const std::string& name)
{
    if (!m_initialized) {
        _E("PkgmgrSingal not yet intialized");
        return false;
    }

    if (name.empty()) {
        _E("name is empty");
        return false;
    }

    m_pkgname = name;
    _D("Success to set tizen package name: %s", m_pkgname.c_str());
    setRecoveryFile();

    return true;
}

void PkgmgrSignal::setRecoveryFile()
{
    std::string filePath = WrtDB::GlobalConfig::GetTempInstallInfoPath();
    filePath += "/" + m_pkgname;

    m_recoveryFile = filePath;
    _D("SetRecoveryFile... %s", filePath.c_str());
    if (access(filePath.c_str(), F_OK) != 0) {
        FILE *file = fopen(filePath.c_str(), "w");
        if (file != NULL) {
            fclose(file);
        }
    } else {
        _D("Recovery File : %s is already exist", filePath.c_str());
    }
}

bool PkgmgrSignal::startJob(Jobs::InstallationType type)
{
    switch(type)
    {
        case Jobs::InstallationType::NewInstallation:
            sendSignal(PKGMGR_START_KEY, PKGMGR_START_INSTALL);
            break;
        case Jobs::InstallationType::UpdateInstallation:
            sendSignal(PKGMGR_START_KEY, PKGMGR_START_UPDATE);
            break;
        case Jobs::InstallationType::Uninstallation:
            sendSignal(PKGMGR_START_KEY, PKGMGR_START_UNINSTALL);
            break;
        default:
            _E("Trying to send unknown installation type to pkgmgr");
            return false;
    }
    return true;
}

bool PkgmgrSignal::endJob(Jobs::Exceptions::Type ecode)
{
    if(ecode == Jobs::Exceptions::Type::Success)
    {
        return sendSignal(PKGMGR_END_KEY, PKGMGR_END_SUCCESS);
    }
    else
    {
        sendSignal(PKGMGR_ERROR, DPL::lexical_cast<std::string>(ecode));
        return sendSignal(PKGMGR_END_KEY, PKGMGR_END_FAILURE);
    }
}

bool PkgmgrSignal::sendProgress(int percent)
{
    if (m_percent == percent) {
        return true;
    }

    m_percent = percent;
    return sendSignal(PKGMGR_PROGRESS_KEY, DPL::lexical_cast<std::string>(percent));
}

bool PkgmgrSignal::sendIconPath(const std::string & iconpath)
{
    return sendSignal(PKGMGR_ICON_PATH, iconpath);
}

bool PkgmgrSignal::sendSignal(const std::string& key,
                              const std::string& value) const
{
    if (!m_initialized) {
        _E("PkgmgrSingal not yet intialized");
        return false;
    }

    if (key.empty() || value.empty()) {
        _D("key or value is empty");
        return false;
    }

    if (m_handle == NULL || m_type.empty()) {
        _E("Some data of PkgmgrSignal is empty");
        return false;
    }

    // send pkgmgr signal
    if (pkgmgr_installer_send_signal(
            m_handle, m_type.c_str(), m_pkgname.c_str(),
            key.c_str(), value.c_str()))
    {
        _E("Fail to send pkgmgr signal");
        return false;
    }

    _D("Success to send pkgmgr signal: %s - %s", key.c_str(), value.c_str());
    return true;
}

std::string PkgmgrSignal::getPkgname() const
{
    if (!m_initialized) {
        _E("PkgmgrSingal not yet intialized");
    }

    return m_pkgname;
}

PkgmgrSignal::RequestType PkgmgrSignal::getRequestedType() const
{
    if (!m_initialized) {
        _E("PkgmgrSingal not yet intialized");
    }

    return m_reqType;
}

std::string PkgmgrSignal::getCallerId() const
{
    if (!m_initialized) {
        _E("PkgmgrSingal not yet intialized");
    }

    return m_callerId;
}
} // PackageManager
