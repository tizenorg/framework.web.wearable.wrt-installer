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
/**
 * @file    task_certify_level.h
 * @author  Jihoon Chung (jihoon.chung@samgsung.com)
 * @version
 * @brief
 */
#ifndef INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_CERTIFY_LEVEL_H
#define INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_CERTIFY_LEVEL_H

//SYSTEM INCLUDES
#include <string>
#include <cstdint>
#include <map>

//WRT INCLUDES
#include <vcore/CertStoreType.h>
#include <vcore/SignatureFinder.h>
#include <dpl/task.h>

class InstallerContext;

namespace Jobs {
namespace WidgetInstall {
class TaskCertifyLevel :
    public DPL::TaskDecl<TaskCertifyLevel>
{
  public:
    TaskCertifyLevel(InstallerContext &inCont);

  private:
    //data
    InstallerContext& m_contextData;

    enum Level {
        UNKNOWN  = 0,
        PUBLIC   = 1,
        PARTNER  = 2,
        PLATFORM = 3
    };
    typedef std::map<std::string, Level> secureSettingMap;
    typedef std::map<std::string, Level>::iterator secureSettingIter;

    //steps
    void stepCertifyLevel();
    void StartStep();
    void EndStep();

    //util
    void getSignatureFiles(const std::string& path,
                           ValidationCore::SignatureFileInfoSet& file);
    Level getCertifyLevel();
    bool checkConfigurationLevel(Level level);
    bool checkSettingLevel(Level level);
    bool checkAppcontrolHasDisposition(Level level);
    bool checkNPRuntime(Level level);
    bool checkServiceLevel(Level level);
    std::string enumToString(Level level);
    Level certTypeToLevel(ValidationCore::CertStoreId::Type type);

};
} //namespace WidgetInstall
} //namespace Jobs

#endif // INSTALLER_CORE_JOS_WIDGET_INSTALL_TASK_CERTIFY_LEVEL_H
