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
 * @file    uninstaller_context.h
 * @author  Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
 * @version
 * @brief   Definition file of installer tasks data structures
 */

#ifndef WRT_SRC_INSTALLER_CORE_UNINSTALLER_TASKS_UNINSTALLER_CONTEXT_H_
#define WRT_SRC_INSTALLER_CORE_UNINSTALLER_TASKS_UNINSTALLER_CONTEXT_H_

#include <string>
#include <boost/optional.hpp>
#include <widget_uninstall/widget_uninstaller_struct.h>
#include <widget_location.h>
#include <dpl/utils/path.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>

namespace Jobs {
namespace WidgetUninstall {
class JobWidgetUninstall;
} //namespace WidgetUninstall
} //namespace Jobs

struct UninstallerContext
{
    enum UninstallStep
    {
        UNINSTALL_START,
        UNINSTALL_PRECHECK,
        UNINSTALL_REMOVE_WIDGETDIR,
        UNINSTALL_REMOVE_DESKTOP,
        UNINSTALL_REMOVE_FINISHED,
        UNINSTALL_DB_UPDATE,
        UNINSTALL_REMOVE_OSPSVC,
        UNINSTALL_SMACK_DISABLE,
        UNINSTALL_END
    };

    ///< flag that indicates whether installer starts
    //to remove files.rStruct;
    bool removeStarted;
    ///< flag that indicates whether installer finishes
    //to remove files completely.
    bool removeFinished;

    boost::optional<WidgetLocation> locations;

    UninstallStep uninstallStep;       ///< current step of installation
    Jobs::WidgetUninstall::JobWidgetUninstall *job;
    std::list<DPL::String> tzAppIdList;
    std::string tzAppid;
    std::string tzPkgid;
    std::string tzServiceid;
    bool removeAbnormal;
    DPL::Utils::Path installedPath;
    DPL::Utils::Path manifestFile;
    WrtDB::AppType appType;
};

#endif // WRT_SRC_INSTALLER_CORE_UNINSTALLER_TASKS_UNINSTALLER_CONTEXT_H_
