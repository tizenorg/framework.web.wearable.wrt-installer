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
 * @file    widget_update_info.cpp
 * @author  Chung Jihoon (jihoon.chung@samsung.com)
 * @version 1.0
 * @brief   Implementation file for WidgetUpdateInfo
 */

#include "widget_update_info.h"

WidgetUpdateInfo::WidgetUpdateInfo(
    const WrtDB::TizenAppId & appId,
    const OptionalWidgetVersion &existingversion,
    const OptionalWidgetVersion &incomingversion,
            WrtDB::AppType type) :
      tzAppId(appId),
      existingVersion(existingversion),
      incomingVersion(incomingversion),
      appType(type)
{
}

