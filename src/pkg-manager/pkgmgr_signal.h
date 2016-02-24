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
 * @author      Jan Olszak (j.olszak@samsung.com)
 * @version     0.2
 * @brief
 */

#ifndef WRT_PKGMGR_SIGNAL_H_
#define WRT_PKGMGR_SIGNAL_H_

#include <pkg-manager/pkgmgr_signal_interface.h>

struct pkgmgr_installer;

namespace PackageManager {

class PkgmgrSignal : public IPkgmgrSignal
{
public:
    enum class RequestType
    {
        UNSUPPORTED,
        INSTALL,
        UNINSTALL,
        REINSTALL
    };

    bool initialize(int argc, char* argv[]);
    bool deinitialize();
    bool setPkgname(const std::string& name);
    std::string getPkgname() const;
    RequestType getRequestedType() const;
    std::string getCallerId() const;

    bool startJob(Jobs::InstallationType type);
    bool endJob(Jobs::Exceptions::Type ecode);
    bool sendProgress(int percent);
    bool sendIconPath(const std::string & iconpath);
    void setRecoveryFile();

    PkgmgrSignal();
    virtual ~PkgmgrSignal();

protected:
    bool sendSignal(const std::string& key, const std::string& value) const;

private:
    bool m_initialized;
    pkgmgr_installer* m_handle;
    std::string m_type;
    std::string m_pkgname;
    RequestType m_reqType;
    std::string m_callerId;
    int m_percent;
    std::string m_recoveryFile;
};
} // PackageManager

#endif // WRT_PKGMGR_SIGNAL_H_

