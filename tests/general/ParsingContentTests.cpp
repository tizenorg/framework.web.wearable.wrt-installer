/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file    ParsingContentTests.cpp
 * @author  Slawomir Pajak (s.pajak@partner.samsung.com)
 * @version 1.0
 * @brief   content element installation tests
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

template<typename Exception, typename Function>
bool checkException(Function fun)
{
    Try
    {
        fun();
    }
    Catch(Exception){
        return true;
    }
    return false;
}

} // namespace

#define RUNNER_ASSERT_EXCEPTION(exceptionType, function)             \
    {                                                                \
        RUNNER_ASSERT(checkException<exceptionType>([&](){function})); \
    }



////////////////////////////////////////////////////////////////////////////////

RUNNER_TEST_GROUP_INIT(ParsingContent)

/*
Name: InstallWidgetWithContentCorrect
Description: Tests if widget with correct content element is installed correctly
Expected: widget should be installed correctly and startFile info should be stored in database
*/
RUNNER_TEST(InstallWidgetWithContentCorrect)
{
    std::string tizenId;
    std::string manifestPath = "/opt/share/packages/";
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/contentCorrect.wgt", tizenId) == InstallerWrapper::Success);

    WrtDB::WidgetDAOReadOnly dao(DPL::FromASCIIString(tizenId));
    WrtDB::WidgetDAOReadOnly::WidgetStartFileList startFileList = dao.getStartFileList();

    RUNNER_ASSERT(6 == startFileList.size());
    RUNNER_ASSERT(
       1 == std::count_if(startFileList.begin(), startFileList.end(),
               [](const WrtDB::WidgetDAOReadOnly::WidgetStartFileRow& startFileRow){
                   return L"http://test.org" == startFileRow.src;
               })
       );


    uninstall(tizenId);
}

/*
Name: InstallWidgetWithContentIncorrect
Description: Tests if widget with incorrect content element is not installed correctly
Expected: widget should not be installed
*/
RUNNER_TEST(InstallWidgetWithContentIncorrect)
{
    std::string tizenId;
    std::string manifestPath = "/opt/share/packages/";
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/contentIncorrect.wgt", tizenId) == InstallerWrapper::OtherError);
}

/*
Name: NoContent
Description: Tests parsing configuration file without content element
Expected: Element should be parsed correctly.
*/
RUNNER_TEST(NoContent)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/NoContent.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(widgetConfig.metadataList.empty());
}

/*
Name: ContentEmpty
Description: Tests parsing configuration file with empty content element
Expected: Exception should be thrown
*/
RUNNER_TEST(ContentEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/ContentEmpty.xml",
                ElementParserPtr( new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: ContentSrcEmpty
Description: Tests parsing configuration file with empty src attribute in content element
Expected: Exception should be thrown
*/
RUNNER_TEST(ContentSrcEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/ContentSrcEmpty.xml",
                ElementParserPtr( new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: ContentSrcCorrect
Description: Tests parsing configuration file with correct content element
Expected: Element should be parsed correctly.
*/
RUNNER_TEST(ContentSrcCorrect)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/ContentSrcCorrect.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(DPL::OptionalString(L"http://test.org") == widgetConfig.startFile);
}

/*
Name: MultipleContentCorrect
Description: Tests parsing configuration file with multiple content element
Expected: Element should be parsed correctly.
*/
RUNNER_TEST(MultipleContentCorrect)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/MultipleContentCorrect.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(DPL::OptionalString(L"http://test.org") == widgetConfig.startFile);
}

/*
Name: MultipleContentCorrect
Description: Tests parsing configuration file with multiple content element.
            First occurrence is incorrect
Expected: Exception should be thrown
*/
RUNNER_TEST(MultipleContentIncorrect)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/MultipleContentIncorrect.xml",
                ElementParserPtr( new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}
