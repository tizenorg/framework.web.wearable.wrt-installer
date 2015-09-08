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
 * plugin_common_types.h
 *
 *      Author: Soyoung Kim(sy037.kim@samsung.com)
 */

#ifndef PLUGIN_COMMON_TYPES_H
#define PLUGIN_COMMON_TYPES_H

#include <boost/optional.hpp>
#include <dpl/utils/widget_version.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>

/**
 * Widget version is optional
 */
typedef boost::optional<WidgetVersion> OptionalWidgetVersion;

/* Define db type */
typedef WrtDB::DbWidgetFeature WidgetFeature;
typedef WrtDB::DbWidgetFeatureSet WidgetFeatureSet;

typedef WrtDB::DbWidgetSize WidgetSize;

typedef WrtDB::DbPluginHandle PluginHandle;

enum InstallLocationType
{
    INSTALL_LOCATION_TYPE_UNKNOWN = 0,
    INSTALL_LOCATION_TYPE_INTERNAL_ONLY,
    INSTALL_LOCATION_TYPE_AUTO,
    INSTALL_LOCATION_TYPE_PREFER_EXTERNAL,
};

#endif /* PLUGIN_COMMON_TYPES_H */
