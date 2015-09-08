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
 * @file    ParsingSplashImgTests.cpp
 * @author  Slawomir Pajak (s.pajak@partner.samsung.com)
 * @version 1.0
 * @brief   Splash element installation tests
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

RUNNER_TEST_GROUP_INIT(ParsingSplashImg)

/*
Name: InstallWidgetWithSplash
Description: Tests if widget with splash is installed correctly
Expected: widget should be installed correctly and splashImg registered in database
*/
RUNNER_TEST(InstallWidgetWithSplash)
{
    std::string tizenId;
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/splash.wgt", tizenId) == InstallerWrapper::Success);

    WrtDB::WidgetDAOReadOnly dao(DPL::FromASCIIString(tizenId));

    RUNNER_ASSERT(*dao.getWidgetInstalledPath() + L"/res/wgt/splash.html" == *dao.getSplashImgSrc());

    uninstall(tizenId);
}


/*
Name: SplashElementOk
Description: Tests parsing correct splash element
Expected: Element should be parsed correcty.
*/
RUNNER_TEST(SplashElementOk)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_splash1.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(DPL::OptionalString(L"splash.html") == widgetConfig.splashImgSrc);
}

/*
Name: SplashElementEmptySrc
Description: Tests parsing splash element with empty src attribute
Expected: No exception and splash should be null
*/
RUNNER_TEST(SplashElementEmptySrc)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_splash2.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                    DPL::
                    FromUTF32String(
                        L"widget"))));

    RUNNER_ASSERT(!widgetConfig.splashImgSrc);
}

/*
Name: SplashElementNoSrc
Description: Tests parsing splash element with no src attribute
Expected: No exception and splash should be null
*/
RUNNER_TEST(SplashElementNoSrc)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_splash3.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                    DPL::
                    FromUTF32String(
                        L"widget"))));

    RUNNER_ASSERT(!widgetConfig.splashImgSrc);

}

/*
Name: SplashElementNoNamespace
Description: Tests parsing splash element without tizen namespace
Expected: No exception and splash should be null
*/
RUNNER_TEST(SplashElementNoNamespace)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_splash4.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                    DPL::
                    FromUTF32String(
                        L"widget"))));

    RUNNER_ASSERT(!widgetConfig.splashImgSrc);

}
