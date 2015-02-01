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
 * This file  have been implemented in compliance with  W3C WARP SPEC.
 * but there are some patent issue between  W3C WARP SPEC and APPLE.
 * so if you want to use this file, refer to the README file in root directory
 */
/**
 * @file        widget_parser.h
 * @author      Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @version     0.1
 * @brief
 */
#ifndef WIDGET_PARSER_H_
#define WIDGET_PARSER_H_

#include "element_parser.h"
#include <list>
#include <map>
#include <dpl/foreach.h>
#include <dpl/optional_typedefs.h>
#include <dpl/wrt-dao-ro/config_parser_data.h>

namespace ConfigurationNamespace {
static const DPL::String W3CWidgetNamespaceName =
    L"http://www.w3.org/ns/widgets";
static const DPL::String TizenWebAppNamespaceName =
    L"http://tizen.org/ns/widgets";
}

namespace PluginsPrefix {
const char * const W3CPluginsPrefix = "http://www.w3.org/";
const char * const TIZENPluginsPrefix = "http://tizen.org/api/";
}

namespace Unicode {
enum Direction
{
    LRE,
    RLE,
    LRO,
    RLO,
    EMPTY
};
}

class WidgetParser : public ElementParser
{
  public:
    ElementParserPtr OnNameElement();
    ElementParserPtr OnDescriptionElement();
    ElementParserPtr OnAuthorElement();
    ElementParserPtr OnLicenseElement();
    ElementParserPtr OnIconElement();
    ElementParserPtr OnSmallIconElement();
    ElementParserPtr OnContentElement();
    ElementParserPtr OnPreferenceElement();
    ElementParserPtr OnAccessElement();
    ElementParserPtr OnSettingElement();
    ElementParserPtr OnApplicationElement();
    ElementParserPtr OnSplashElement();
    ElementParserPtr OnBackgroundElement();
    ElementParserPtr OnPrivilegeElement();
    ElementParserPtr OnAppControlElement();
    ElementParserPtr OnCategoryElement();
    ElementParserPtr OnAppWidgetElement();
    ElementParserPtr OnCspElement();
    ElementParserPtr OnCspReportOnlyElement();
    ElementParserPtr OnAllowNavigationElement();
    ElementParserPtr OnAccountElement();
    ElementParserPtr OnMetadataElement();

#ifdef IME_ENABLED
    ElementParserPtr OnImeElement();
#endif
#ifdef SERVICE_ENABLED
    ElementParserPtr OnServiceAppElement();
#endif

    virtual ActionFunc GetElementParser(const DPL::String& ns,
                                        const DPL::String& name);

    virtual void Accept(const Element&);
    virtual void Accept(const Text&);
    virtual void Accept(const XmlAttribute&);
    virtual void Verify();

    //Typedef used by RootParser
    typedef WrtDB::ConfigParserData& Data;

    WidgetParser(Data&);

  private:
    Data& m_data;
    Unicode::Direction m_textDirection;
    FuncMap m_map;
    DPL::OptionalString m_widgetId;
    DPL::OptionalString m_version;
    DPL::OptionalString m_minVersion;
    std::list<DPL::String> m_windowModes;
    DPL::OptionalString m_defaultlocale;
    std::map<DPL::String, DPL::String> m_nameSpaces;
};

struct IconParser;
struct ContentParser;

#endif // WIDGET_PARSER_H_
