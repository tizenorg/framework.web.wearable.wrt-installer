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
 * @file
 * @author      Bartlomiej Grzelewski (b.grzelewski@samsung.com)
 * @version     1.0
 * @brief
 */
#ifndef WRT_ENGINE_SRC_INSTALLER_CORE_MISC_WAC_WIDGET_ID_H
#define WRT_ENGINE_SRC_INSTALLER_CORE_MISC_WAC_WIDGET_ID_H

#include <dpl/string.h>
#include <dpl/optional_typedefs.h>

class WacWidgetId
{
  public:
    explicit WacWidgetId(const DPL::OptionalString &widgetId);
    bool matchHost(const DPL::String &second) const;

  private:
    void parse(const char *url);

    bool m_schemaMatch;
    std::string m_host;
};

#endif // WRT_ENGINE_SRC_INSTALLER_CORE_MISC_WAC_WIDGET_ID_H

