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
 * @file    widget_update_info.h
 * @author  Chung Jihoon (jihoon.chung@samsung.com)
 * @version 1.0
 * @brief   Header file for WidgetUpdateInfo
 */
#ifndef SRC_DOMAIN_WIDGET_UPDATE_INFO_H
#define SRC_DOMAIN_WIDGET_UPDATE_INFO_H

#include <wrt_common_types.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>

/**
 * WidgetUpdateInfo
 * A structure to hold widget's information needed to be registered.
 * @see WidgetConfigurationInfo
 */
struct WidgetUpdateInfo
{
    WrtDB::TizenAppId tzAppId;
    // Existing widget
    OptionalWidgetVersion existingVersion;
    // Incoming widget
    OptionalWidgetVersion incomingVersion;
    // widget type
    WrtDB::AppType appType;

    WidgetUpdateInfo() {};
    WidgetUpdateInfo(const WrtDB::TizenAppId & tzAppid,
                     const OptionalWidgetVersion &existringversion,
                     const OptionalWidgetVersion &incomingVersion,
                     const WrtDB::AppType type);
};

#endif // SRC_DOMAIN_WIDGET_UPDATE_INFO_H
