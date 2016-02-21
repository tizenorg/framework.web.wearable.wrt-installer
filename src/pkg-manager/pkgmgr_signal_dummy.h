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
 * @author      Jan Olszak (j.olszak@samsung.com)
 * @version     0.1
 * @brief       Dummy version of PkgmgrSignal.
 */

#ifndef WRT_PKGMGR_SIGNAL_DUMMY_H_
#define WRT_PKGMGR_SIGNAL_DUMMY_H_

#include <pkg-manager/pkgmgr_signal_interface.h>

#include <dpl/availability.h>

namespace PackageManager {
class PkgmgrSignalDummy : public IPkgmgrSignal
{
  public:
    PkgmgrSignalDummy()
    {}

    virtual ~PkgmgrSignalDummy()
    {}

    bool setPkgname(const std::string& /*name*/)
    {
        return false;
    }

    std::string getPkgname() const
    {
        return "";
    }

    std::string getCallerId() const
    {
        return "";
    }

    bool startJob(Jobs::InstallationType type DPL_UNUSED)
    {
        return false;
    }

    bool endJob(Jobs::Exceptions::Type ecode DPL_UNUSED, const char* message DPL_UNUSED)
    {
        return false;
    }

    bool sendProgress(int percent DPL_UNUSED)
    {
        return false;
    }

    bool sendIconPath(const std::string & iconpath DPL_UNUSED)
    {
        return false;
    }
};
} // PkgmgrSignalDummy

#endif // WRT_PKGMGR_SIGNAL_DUMMY_H_
