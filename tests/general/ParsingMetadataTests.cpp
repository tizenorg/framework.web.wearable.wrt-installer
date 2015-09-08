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
 * @file    ParsingMetadataTests.cpp
 * @author  Slawomir Pajak (s.pajak@partner.samsung.com)
 * @version 1.0
 * @brief   metadata element installation tests
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

RUNNER_TEST_GROUP_INIT(ParsingMetadata)

/*
Name: InstallWidgetWithMetadata
Description: Tests if widget with metadata element is installed correctly
Expected: widget should be installed correctly and metadata info should be stored in manifest file
*/
RUNNER_TEST(InstallWidgetWithMetadata)
{
    std::string tizenId;
    std::string manifestPath = "/opt/share/packages/";
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/metadata.wgt", tizenId) == InstallerWrapper::Success);

    RUNNER_ASSERT(WrtUtilFileExists(manifestPath.append(tizenId.substr(0, 10)).append(".xml")));
    ManifestFile mf(manifestPath);

    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/p:metadata[1]/@key") == "key1");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/p:metadata[2]/@key") == "key2");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/p:metadata[2]/@value") == "value2");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/p:metadata[3]/@key") == "key3");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/p:metadata[3]/@value") == "value3");
    uninstall(tizenId);
}


/*
Name: NoMetadata
Description: Tests parsing configuration file without metadata element
Expected: Element should be parsed correctly.
*/
RUNNER_TEST(NoMetadata)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/NoMetadata.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(widgetConfig.metadataList.empty());
}

/*
Name: MetadataEmpty
Description: Tests parsing configuration file with empty metadata element
Expected: Exception should be thrown
*/
//TODO: Fix in parser needed
//RUNNER_TEST(MetadataEmpty)
//{
//    ParserRunner parser;
//    WrtDB::ConfigParserData widgetConfig;
//
//    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
//        parser.Parse(miscWidgetsStuff + "configs/MetadataEmpty.xml",
//                ElementParserPtr( new RootParser<WidgetParser>(widgetConfig, L"widget")));
//    );
//}

/*
Name: MultipleMetadata
Description: Tests parsing configuration file with multiple metadata element
Expected: Element should be parsed correctly. All values should be stored
*/
RUNNER_TEST(MultipleMetadata)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/MultipleMetadata.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                        L"widget")));

    RUNNER_ASSERT(3 == widgetConfig.metadataList.size());

    RUNNER_ASSERT(1 == std::count(widgetConfig.metadataList.begin(), widgetConfig.metadataList.end(),
            WrtDB::ConfigParserData::Metadata(DPL::OptionalString(L"key1"), DPL::OptionalString())));
    RUNNER_ASSERT(1 == std::count(widgetConfig.metadataList.begin(), widgetConfig.metadataList.end(),
            WrtDB::ConfigParserData::Metadata(DPL::OptionalString(L"key2"), DPL::OptionalString(L"value2"))));
    RUNNER_ASSERT(1 == std::count(widgetConfig.metadataList.begin(), widgetConfig.metadataList.end(),
            WrtDB::ConfigParserData::Metadata(DPL::OptionalString(L"key3"), DPL::OptionalString(L"value3"))));

}

/*
Name: MetadataDuplicatedKey
Description: Tests parsing configuration file with duplicated key attribute value in metadata element
Expected: Exception should be thrown
*/
//TODO: Fix in parser needed
//RUNNER_TEST(MetadataDuplicatedKey)
//{
//    ParserRunner parser;
//    WrtDB::ConfigParserData widgetConfig;
//
//    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
//        parser.Parse(miscWidgetsStuff + "configs/MetadataDuplicatedKey.xml",
//                ElementParserPtr( new RootParser<WidgetParser>(widgetConfig, L"widget")));
//    );
//}
