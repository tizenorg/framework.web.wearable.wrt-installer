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
 * @brief       Interface for PkgmgrSignal.
 */

#ifndef WRT_PKGMGR_SIGNAL_INTERFACE_H_
#define WRT_PKGMGR_SIGNAL_INTERFACE_H_

#include <string>

#include <job_types.h>
#include <job_exception_error.h>

namespace PackageManager {
class IPkgmgrSignal
{
  public:
    virtual bool setPkgname(const std::string& name) = 0;
    virtual std::string getPkgname() const = 0;
    virtual std::string getCallerId() const = 0;

    virtual bool startJob(Jobs::InstallationType type) = 0;
    virtual bool endJob(Jobs::Exceptions::Type ecode) = 0;
    virtual bool sendProgress(int percent) = 0;
    virtual bool sendIconPath(const std::string & iconpath) = 0;
    virtual ~IPkgmgrSignal(){}
};
} // IPkgmgrSignal

#endif // WRT_PKGMGR_SIGNAL_INTERFACE_H_
