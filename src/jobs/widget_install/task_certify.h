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
#include <string>
#include <libxml/c14n.h>
#include <vcore/SignatureFinder.h>

//WRT INCLUDES
#include <dpl/task.h>

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

  private:
    //data
    InstallerContext& m_contextData;

    //steps
    void stepSignature();
    void stepVerifyUpdate();

    void StartStep();
    void EndStep();

    void processDistributorSignature(const ValidationCore::SignatureData &data);
    void processAuthorSignature(const ValidationCore::SignatureData &data);
    void getSignatureFiles(std::string path,
            ValidationCore::SignatureFileInfoSet& file);

    bool isTizenWebApp() const;
};
} //namespace WidgetInstall
} //namespace Jobs

#endif // INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_CERTIFY_H
