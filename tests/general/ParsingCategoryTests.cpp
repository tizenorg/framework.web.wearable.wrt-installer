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
 * @file    ParsingCategoryTests.cpp
 * @author  Slawomir Pajak (s.pajak@partner.samsung.com)
 * @version 1.0
 * @brief   Category element installation tests
 */

#include <string>
#include <dpl/test/test_runner.h>
#include <InstallerWrapper.h>
#include <ManifestFile.h>
#include <dpl/utils/wrt_utility.h>

#include <root_parser.h>
#include <widget_parser.h>
#include <parser_runner.h>

using namespace InstallerWrapper;

////////////////////////////////////////////////////////////////////////////////

RUNNER_TEST_GROUP_INIT(ParsingCategory)

/*
Name: InstallWidgetWithCategory
Description: Tests if widget with category is installed correctly
Expected: widget should be installed correctly and category should be present in manifest file
*/
RUNNER_TEST(InstallWidgetWithCategory)
{
    std::string tizenId;
    std::string manifestPath = "/opt/share/packages/";
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/category.wgt", tizenId) == InstallerWrapper::Success);

    RUNNER_ASSERT(WrtUtilFileExists(manifestPath.append(tizenId.substr(0, 10)).append(".xml")));
    ManifestFile mf(manifestPath);

    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/p:category[1]/@name") == "testCategory");
    uninstall(tizenId);
}


/*
Name: CategoryElementOk
Description: Tests parsing correct category element
Expected: Element should be parsed correcty.
*/
RUNNER_TEST(CategoryElementOk)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_category1.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(2 == widgetConfig.categoryList.size());
    RUNNER_ASSERT(std::count(widgetConfig.categoryList.begin(), widgetConfig.categoryList.end(), L"testCategory1") == 1);
    RUNNER_ASSERT(std::count(widgetConfig.categoryList.begin(), widgetConfig.categoryList.end(), L"testCategory2") == 1);
}

/*
Name: CategoryElementEmptyName
Description: Tests parsing splash element with empty name attribute
Expected: No exception and categoryList should be empty
*/
RUNNER_TEST(CategoryElementEmptyName)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_category2.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                    DPL::
                    FromUTF32String(
                        L"widget"))));

    RUNNER_ASSERT(widgetConfig.categoryList.empty());
}

/*
Name: CategoryElementNoName
Description: Tests parsing category element with no name attribute
Expected: No exception and categoryList should be empty
*/
RUNNER_TEST(CategoryElementNoName)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_category3.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                    DPL::
                    FromUTF32String(
                        L"widget"))));

    RUNNER_ASSERT(widgetConfig.categoryList.empty());
}

/*
Name: CategoryElementDuplicated
Description: Tests parsing three category elements (two are identical)
Expected: No exception and categoryList should have two distinct elements
*/
RUNNER_TEST(CategoryElementDuplicated)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_category4.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                    DPL::
                    FromUTF32String(
                        L"widget"))));

    RUNNER_ASSERT(2 == widgetConfig.categoryList.size());
    RUNNER_ASSERT(std::count(widgetConfig.categoryList.begin(), widgetConfig.categoryList.end(), L"testCategory1") == 1);
    RUNNER_ASSERT(std::count(widgetConfig.categoryList.begin(), widgetConfig.categoryList.end(), L"testCategory2") == 1);

}
