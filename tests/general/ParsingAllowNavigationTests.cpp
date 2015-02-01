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
 * @file    ParsingAllowNavigationTests.cpp
 * @author  Slawomir Pajak (s.pajak@partner.samsung.com)
 * @version 1.0
 * @brief   allow-navigation element installation tests
 */

#include <string>
#include <dpl/test/test_runner.h>
#include <InstallerWrapper.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>

#include <root_parser.h>
#include <widget_parser.h>
#include <parser_runner.h>

using namespace InstallerWrapper;

namespace{

struct AllowNavigationComparator {
    AllowNavigationComparator(const DPL::String & scheme, const DPL::String& host) :
            m_scheme(scheme), m_host(host){}
    const DPL::String m_scheme;
    const DPL::String m_host;
    bool operator()(const WrtDB::ConfigParserData::AllowNavigationInfo& navs) const
    {
        return navs.m_host == m_host && navs.m_scheme == m_scheme;
    }
    bool operator()(const WrtDB::WidgetAllowNavigationInfo& navs) const
    {
        return navs.host == m_host && navs.scheme == m_scheme;
    }
};

}



////////////////////////////////////////////////////////////////////////////////

RUNNER_TEST_GROUP_INIT(ParsingAllowNavigation)

/*
Name: InstallWidgetWithAllowNavigation
Description: Tests if widget with allow-navigation is installed correctly
Expected: widget should be installed correctly and allowNavigation info should be stored in database
*/
RUNNER_TEST(InstallWidgetWithAllowNavigation)
{
    std::string tizenId;
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/allowNavigation.wgt", tizenId) == InstallerWrapper::Success);

    WrtDB::WidgetDAOReadOnly dao(DPL::FromASCIIString(tizenId));
    WrtDB::WidgetAllowNavigationInfoList allowNavigationList;
    dao.getWidgetAllowNavigationInfo(allowNavigationList);

    uninstall(tizenId);

    RUNNER_ASSERT(allowNavigationList.size() == 4);
    RUNNER_ASSERT(1 == std::count_if(allowNavigationList.begin(), allowNavigationList.end(),
            AllowNavigationComparator(L"http",L"test2.org")));
    RUNNER_ASSERT(1 == std::count_if(allowNavigationList.begin(), allowNavigationList.end(),
            AllowNavigationComparator(L"*",L"test3.org")));
    RUNNER_ASSERT(1 == std::count_if(allowNavigationList.begin(), allowNavigationList.end(),
            AllowNavigationComparator(L"*",L"*.test4.org")));
    RUNNER_ASSERT(1 == std::count_if(allowNavigationList.begin(), allowNavigationList.end(),
            AllowNavigationComparator(L"*",L"*")));
}


/*
Name: NoAllowNavigation
Description: Tests parsing configuration file without allow-navigation element
Expected: Element should be parsed correctly.
*/
RUNNER_TEST(NoAllowNavigation)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/NoAllowNavigation.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(0 == widgetConfig.allowNavigationInfoList.size());
}

/*
Name: AllowNavigationEmpty
Description: Tests parsing configuration file with empty allow-navigation element
Expected: Element should be parsed correctly.
*/
RUNNER_TEST(AllowNavigationEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/AllowNavigationEmpty.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(0 == widgetConfig.allowNavigationInfoList.size());
}

/*
Name: MultipleAllowNavigation
Description: Tests parsing configuration file with multiple allow-navigation element
Expected: Element should be parsed correctly. Only values from first element should be stored
*/
RUNNER_TEST(MultipleAllowNavigation)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/MultipleAllowNavigation.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.allowNavigationInfoList.size());
    RUNNER_ASSERT(L"*" == widgetConfig.allowNavigationInfoList.begin()->m_scheme);
    RUNNER_ASSERT(L"test1.org" == widgetConfig.allowNavigationInfoList.begin()->m_host);
}

/*
Name: AllowNavigationMultipleHosts
Description: Tests parsing configuration file with multiple values in allow-navigation element
Expected: Element should be parsed correctly.
*/
RUNNER_TEST(AllowNavigationMultipleHosts)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/AllowNavigationMultipleHosts.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(4 == widgetConfig.allowNavigationInfoList.size());
    RUNNER_ASSERT(1 == std::count_if(widgetConfig.allowNavigationInfoList.begin(), widgetConfig.allowNavigationInfoList.end(),
            AllowNavigationComparator(L"http",L"test2.org")));
    RUNNER_ASSERT(1 == std::count_if(widgetConfig.allowNavigationInfoList.begin(), widgetConfig.allowNavigationInfoList.end(),
            AllowNavigationComparator(L"*",L"test3.org")));
    RUNNER_ASSERT(1 == std::count_if(widgetConfig.allowNavigationInfoList.begin(), widgetConfig.allowNavigationInfoList.end(),
            AllowNavigationComparator(L"*",L"*.test4.org")));
    RUNNER_ASSERT(1 == std::count_if(widgetConfig.allowNavigationInfoList.begin(), widgetConfig.allowNavigationInfoList.end(),
            AllowNavigationComparator(L"*",L"*")));
}

/*
Name: AllowNavigationMultipleHosts
Description: Tests parsing configuration file with multiple values in allow-navigation element
             with special characters like \n and \t
Expected: Element should be parsed correctly.
*/
RUNNER_TEST(AllowNavigationMultipleHostsMultiline)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/AllowNavigationMultipleHostsMultiline.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(4 == widgetConfig.allowNavigationInfoList.size());
    RUNNER_ASSERT(1 == std::count_if(widgetConfig.allowNavigationInfoList.begin(), widgetConfig.allowNavigationInfoList.end(),
            AllowNavigationComparator(L"http",L"test2.org")));
    RUNNER_ASSERT(1 == std::count_if(widgetConfig.allowNavigationInfoList.begin(), widgetConfig.allowNavigationInfoList.end(),
            AllowNavigationComparator(L"*",L"test3.org")));
    RUNNER_ASSERT(1 == std::count_if(widgetConfig.allowNavigationInfoList.begin(), widgetConfig.allowNavigationInfoList.end(),
            AllowNavigationComparator(L"*",L"*.test4.org")));
    RUNNER_ASSERT(1 == std::count_if(widgetConfig.allowNavigationInfoList.begin(), widgetConfig.allowNavigationInfoList.end(),
            AllowNavigationComparator(L"*",L"*")));
}
