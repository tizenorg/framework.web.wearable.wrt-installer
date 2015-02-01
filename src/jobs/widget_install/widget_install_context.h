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
 * @file    installer_structs.h
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @version
 * @brief   Definition file of installer tasks data structures
 */
#ifndef INSTALLER_CONTEXT_H
#define INSTALLER_CONTEXT_H

#include <map>
#include <string>
#include <boost/optional.hpp>
#include <dpl/string.h>
#include <dpl/wrt-dao-rw/widget_dao.h>
#include <widget_install/widget_security.h>
#include <feature_logic.h>
#include <widget_install/widget_update_info.h>
#include <widget_location.h>
#include <wrt_install_mode.h>

namespace Jobs {
namespace WidgetInstall {
class JobWidgetInstall;
} //namespace Jobs
} //namespace WidgetInstall

class WidgetModel;

typedef std::map<DPL::String, bool> RequestedDevCapsMap;

struct InstallerContext
{
    typedef enum InstallStepEnum
    {
        INSTALL_START = 0,
        INSTALL_PARSE_CONFIG,
        INSTALL_CHECK_FILE,

        INSTALL_RDS_DELTA_CHECK,
        INSTALL_RDS_PREPARE,

        INSTALL_CREATE_BACKUP_DIR,                     /* For Update */
        INSTALL_DIR_CREATE,
        INSTALL_UNZIP_WGT,
        INSTALL_WIDGET_CONFIG1,
        INSTALL_WIDGET_CONFIG2,
        INSTALL_DIGSIG_CHECK,
        INSTALL_CERT_CHECK,
        INSTALL_CERTIFY_LEVEL_CHECK,
        INSTALL_CREATE_PRIVATE_STORAGE,
        INSTALL_BACKUP_ICONFILE,                         /* For Update */
        INSTALL_COPY_ICONFILE,
        INSTALL_COPY_LIVEBOX_FILES,
        INSTALL_CREATE_EXECFILE,
        INSTALL_CREATE_MANIFEST,
        INSTALL_ECRYPTION_FILES,
        INSTALL_INSTALL_OSPSVC,
        INSTALL_NEW_DB_INSERT,
        INSTALL_ACE_PREPARE,
        INSTALL_ACE_CHECK,
        INSTALL_SMACK_ENABLE,
        INSTALL_PKGINFO_UPDATE,
        INSTALL_SET_CERTINFO,

        INSTALL_END
    } InstallStep;

    // Installation state variables
    WrtDB::WidgetRegisterInfo widgetConfig;      ///< WidgetConfigInfo
    boost::optional<WidgetLocation> locations;
    Jobs::WidgetInstall::WidgetSecurity widgetSecurity; ///< Widget Domain
                                                  // information.
    InstallStep installStep;              ///< current step of installation
    Jobs::WidgetInstall::JobWidgetInstall *job;
     ///< Whether this is an update or normal installation
    Jobs::WidgetInstall::FeatureLogicPtr featureLogic;
    /** List of dev-caps that are requested in widget config file.
     * Additional flag tells whether dev cap gets "static" permission
     * (will always have PERMIT from ACE Policy). They will therefore receive
     * static SMACK permission. (They may be forbidden because
     * of ACE User Settings, but for now we do not protect this
     * case with SMACK). */
    RequestedDevCapsMap staticPermittedDevCaps;
    std::string installInfo;            ///<For recovery>
    InstallLocationType locationType;
    bool isUpdateMode;
    InstallMode mode;
    DPL::String callerPkgId;

    std::string requestedPath; ///input path of widget
    bool needEncryption;  ///for configuring right task if encryption needed
    int certLevel;
};

#endif // INSTALLER_CONTEXT_H
