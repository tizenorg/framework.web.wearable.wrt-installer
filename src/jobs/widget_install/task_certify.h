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
 * @file    task_certify.h
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @version
 * @brief
 */
#ifndef INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_CERTIFY_H
#define INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_CERTIFY_H

//SYSTEM INCLUDES
#include <map>
#include <string>
#include <libxml/c14n.h>

//WRT INCLUDES
#include <dpl/task.h>
#include <vcore/CertStoreType.h>
#include <vcore/SignatureFinder.h>

class InstallerContext;

namespace ValidationCore {
class SignatureData;
}

namespace Jobs {
namespace WidgetInstall {
class TaskCertify :
    public DPL::TaskDecl<TaskCertify>
{
  public:
    TaskCertify(InstallerContext &inCont);

    enum Level {
        UNKNOWN  = 0,
        PUBLIC   = 1,
        PARTNER  = 2,
        PLATFORM = 3
    };

  private:
    //data
    InstallerContext& m_contextData;

    typedef std::map<std::string, Level> secureSettingMap;
    typedef std::map<std::string, Level>::iterator secureSettingIter;

    //steps
    void stepSignature();
    void stepVerifyUpdate();
    void stepCertifyLevel();

    void StartStep();
    void EndStep();

    //util
    void processDistributorSignature(const ValidationCore::SignatureData &data);
    void processAuthorSignature(const ValidationCore::SignatureData &data);
    void getSignatureFiles(std::string path,
            ValidationCore::SignatureFileInfoSet& file);

    bool isTizenWebApp() const;

    Level getCertifyLevel();
    bool checkConfigurationLevel(Level level);
    bool checkSettingLevel(Level level);
    bool checkNPRuntime(Level level);
    bool checkServiceLevel(Level level);
    bool checkAppcontrolHasDisposition(Level level);
    std::string enumToString(Level level);
    Level certTypeToLevel(ValidationCore::CertStoreId::Type type);
};
} //namespace WidgetInstall
} //namespace Jobs

#endif // INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_CERTIFY_H
