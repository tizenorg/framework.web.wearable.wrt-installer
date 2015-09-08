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
 * @file    ParsingCspTests.cpp
 * @author  Slawomir Pajak (s.pajak@partner.samsung.com)
 * @version 1.0
 * @brief   content-security-policy and content-security-policy-report-only element installation
 *          tests
 */

#include <string>
#include <dpl/test/test_runner.h>
#include <InstallerWrapper.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>

#include <root_parser.h>
#include <widget_parser.h>
#include <parser_runner.h>

using namespace InstallerWrapper;

////////////////////////////////////////////////////////////////////////////////

RUNNER_TEST_GROUP_INIT(ParsingCsp)

/*
Name: InstallWidgetWithCsp
Description: Tests if widget with content-security-policy and content-security-policy-report-only is installed correctly
Expected: widget should be installed correctly and CSP values should be correctly stored in database
*/
RUNNER_TEST(InstallWidgetWithCsp)
{
    std::string tizenId;
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/csp.wgt", tizenId) == InstallerWrapper::Success);

    WrtDB::WidgetDAOReadOnly dao(DPL::FromASCIIString(tizenId));

    RUNNER_ASSERT(L"testCSP" == *dao.getCspPolicy());
    RUNNER_ASSERT(L"testCSPro" == *dao.getCspPolicyReportOnly());

    uninstall(tizenId);
}


/*
Name: NoCsp
Description: Tests parsing configuration file without content-security-policy and content-security-policy-report-only element
Expected: Element should be parsed correctly.
*/
RUNNER_TEST(NoCsp)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/NoCsp.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(!widgetConfig.cspPolicy);
    RUNNER_ASSERT(!widgetConfig.cspPolicyReportOnly);
}

/*
Name: CspEmpty
Description: Tests parsing configuration file with empty content-security-policy element
Expected: Element should be parsed correctly.
*/
RUNNER_TEST(CspEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/CspEmpty.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(!widgetConfig.cspPolicy);
    RUNNER_ASSERT(!widgetConfig.cspPolicyReportOnly);
}

/*
Name: CspReportOnlyEmpty
Description: Tests parsing configuration file with empty content-security-policy-report-only element
Expected: Element should be parsed correctly.
*/
RUNNER_TEST(CspReportOnlyEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/CspReportOnlyEmpty.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(!widgetConfig.cspPolicy);
    RUNNER_ASSERT(!widgetConfig.cspPolicyReportOnly);
}

/*
Name: MultipleCsp
Description: Tests parsing configuration file with multiple content-security-policy  elements
Expected: Element should be parsed correctly. Only values from first element should be stored
*/
RUNNER_TEST(MultipleCsp)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/MultipleCsp.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(L"testCSP" == *widgetConfig.cspPolicy);
}

/*
Name: MultipleCsp
Description: Tests parsing configuration file with multiple content-security-policy-report-only  elements
Expected: Element should be parsed correctly. Only values from first element should be stored
*/
RUNNER_TEST(MultipleCspReportOnly)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/MultipleCspReportOnly.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(L"testCSP" == *widgetConfig.cspPolicyReportOnly);
}
