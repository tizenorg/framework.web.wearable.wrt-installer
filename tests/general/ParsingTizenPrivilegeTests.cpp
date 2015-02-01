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
 * @file    ParsingTizenPrivilegeTests.cpp
 * @author  Slawomir Pajak (s.pajak@partner.samsung.com)
 * @version 1.0
 * @brief   Parsing Tizen privilege bodies
 */

#include <string>
#include <algorithm>
#include <dpl/test/test_runner.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <InstallerWrapper.h>

#include <root_parser.h>
#include <widget_parser.h>
#include <parser_runner.h>

using namespace InstallerWrapper;

namespace {

class CompareFeatureByName {
public:
    CompareFeatureByName(DPL::String name) :
            m_name(name)
    {

    }
    bool operator()(const WrtDB::DbWidgetFeature& feature)
    {
        return feature.name == m_name;
    }
private:
    DPL::String m_name;
};

}
////////////////////////////////////////////////////////////////////////////////

RUNNER_TEST_GROUP_INIT(ParsingTizenPrivilege)

/*
Name: tizen_privilege
Description: Tests if widget privilege tag is correctly parsed
Expected: widget should be installed. Privileges and features registered in database
*/
RUNNER_TEST(InstallWidgetWithPrivilege)
{
    std::string tizenId;
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/privilege.wgt", tizenId) == InstallerWrapper::Success);

    WrtDB::WidgetDAOReadOnly dao(DPL::FromASCIIString(tizenId));
    WrtDB::PrivilegeList privilegeList = dao.getWidgetPrivilege();
    WrtDB::DbWidgetFeatureSet featureList = dao.getFeaturesList();
    uninstall(tizenId);

    RUNNER_ASSERT(privilegeList.size() == 5);
    RUNNER_ASSERT(std::count(privilegeList.begin(), privilegeList.end(), L"http://tizen.org/privilege/location") == 1);
    RUNNER_ASSERT(
            std::count(privilegeList.begin(), privilegeList.end(), L"http://tizen.org/privilege/notification") == 1);
    RUNNER_ASSERT(
            std::count(privilegeList.begin(), privilegeList.end(), L"http://tizen.org/privilege/mediacapture") == 1);
    RUNNER_ASSERT(
            std::count(privilegeList.begin(), privilegeList.end(), L"http://tizen.org/privilege/fullscreen") == 1);
    RUNNER_ASSERT(
            std::count(privilegeList.begin(), privilegeList.end(), L"http://tizen.org/privilege/unlimitedstorage") == 1);

    RUNNER_ASSERT(featureList.size() == 5);
    RUNNER_ASSERT(
            std::count_if(featureList.begin(), featureList.end(), CompareFeatureByName(L"http://tizen.org/privilege/location")) == 1);
    RUNNER_ASSERT(
            std::count_if(featureList.begin(), featureList.end(), CompareFeatureByName(L"http://tizen.org/privilege/notification")) == 1);
    RUNNER_ASSERT(
            std::count_if(featureList.begin(), featureList.end(), CompareFeatureByName(L"http://tizen.org/privilege/mediacapture")) == 1);
    RUNNER_ASSERT(
            std::count_if(featureList.begin(), featureList.end(), CompareFeatureByName(L"http://tizen.org/privilege/fullscreen")) == 1);
    RUNNER_ASSERT(
            std::count_if(featureList.begin(), featureList.end(), CompareFeatureByName(L"http://tizen.org/privilege/unlimitedstorage")) == 1);
}

/*
Name: PrivilegeElementOk
Description: Tests parsing correct privilege element
Expected: Element should be parsed correcty.
*/
RUNNER_TEST(PrivilegeElementOk)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_privilege1.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(5 == widgetConfig.privilegeList.size());
    RUNNER_ASSERT(
            std::count(widgetConfig.privilegeList.begin(), widgetConfig.privilegeList.end(),
                    WrtDB::ConfigParserData::Privilege(L"http://tizen.org/privilege/location")) == 1);
    RUNNER_ASSERT(
            std::count(widgetConfig.privilegeList.begin(), widgetConfig.privilegeList.end(),
                    WrtDB::ConfigParserData::Privilege(L"http://tizen.org/privilege/notification")) == 1);
    RUNNER_ASSERT(
            std::count(widgetConfig.privilegeList.begin(), widgetConfig.privilegeList.end(),
                    WrtDB::ConfigParserData::Privilege(L"http://tizen.org/privilege/mediacapture")) == 1);
    RUNNER_ASSERT(
            std::count(widgetConfig.privilegeList.begin(), widgetConfig.privilegeList.end(),
                    WrtDB::ConfigParserData::Privilege(L"http://tizen.org/privilege/fullscreen")) == 1);
    RUNNER_ASSERT(
            std::count(widgetConfig.privilegeList.begin(), widgetConfig.privilegeList.end(),
                    WrtDB::ConfigParserData::Privilege(L"http://tizen.org/privilege/unlimitedstorage")) == 1);

    RUNNER_ASSERT(5 == widgetConfig.featuresList.size());
    RUNNER_ASSERT(
            std::count(widgetConfig.featuresList.begin(), widgetConfig.featuresList.end(),
                    WrtDB::ConfigParserData::Feature(L"http://tizen.org/privilege/location")) == 1);
    RUNNER_ASSERT(
            std::count(widgetConfig.featuresList.begin(), widgetConfig.featuresList.end(),
                    WrtDB::ConfigParserData::Feature(L"http://tizen.org/privilege/notification")) == 1);
    RUNNER_ASSERT(
            std::count(widgetConfig.featuresList.begin(), widgetConfig.featuresList.end(),
                    WrtDB::ConfigParserData::Feature(L"http://tizen.org/privilege/mediacapture")) == 1);
    RUNNER_ASSERT(
            std::count(widgetConfig.featuresList.begin(), widgetConfig.featuresList.end(),
                    WrtDB::ConfigParserData::Feature(L"http://tizen.org/privilege/fullscreen")) == 1);
    RUNNER_ASSERT(
            std::count(widgetConfig.featuresList.begin(), widgetConfig.featuresList.end(),
                    WrtDB::ConfigParserData::Feature(L"http://tizen.org/privilege/unlimitedstorage")) == 1);
}

/*
Name: CategoryElementEmptyName
Description: Tests parsing privilege element with empty name attribute
Expected: No exception. PrivilegeList and featuresList should be empty
*/
RUNNER_TEST(PrivilegeElementEmptyName)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_privilege2.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                    DPL::
                    FromUTF32String(
                        L"widget"))));

    RUNNER_ASSERT(widgetConfig.privilegeList.empty());
    RUNNER_ASSERT(widgetConfig.featuresList.empty());
}

/*
Name: PrivilegeElementNoName
Description: Tests parsing privilege element with no name attribute
Expected: No exception. PrivilegeList and featuresList should be empty
*/
RUNNER_TEST(PrivilegeElementNoName)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_privilege3.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                    DPL::
                    FromUTF32String(
                        L"widget"))));

    RUNNER_ASSERT(widgetConfig.categoryList.empty());
    RUNNER_ASSERT(widgetConfig.featuresList.empty());
}

/*
Name: PrivilegeElementNoNameSpace
Description: Tests parsing privilege element without proper namespace
Expected: No exception. PrivilegeList and featuresList should be empty
*/
RUNNER_TEST(PrivilegeElementNoNameSpace)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_privilege4.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                    DPL::
                    FromUTF32String(
                        L"widget"))));

    RUNNER_ASSERT(widgetConfig.categoryList.empty());
    RUNNER_ASSERT(widgetConfig.featuresList.empty());
}

/*
Name: PrivilegeElementDuplicated
Description: Tests parsing three privilege elements (two are identical)
Expected: No exception. PrivilegeList and featuresList should have two distinct elements
*/
RUNNER_TEST(PrivilegeElementDuplicated)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_privilege5.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                    DPL::
                    FromUTF32String(
                        L"widget"))));

    RUNNER_ASSERT(2 == widgetConfig.privilegeList.size());
    RUNNER_ASSERT(
            std::count(widgetConfig.privilegeList.begin(), widgetConfig.privilegeList.end(),
                    WrtDB::ConfigParserData::Privilege(L"http://tizen.org/privilege/location")) == 1);
    RUNNER_ASSERT(
            std::count(widgetConfig.privilegeList.begin(), widgetConfig.privilegeList.end(),
                    WrtDB::ConfigParserData::Privilege(L"http://tizen.org/privilege/notification")) == 1);

    RUNNER_ASSERT(2 == widgetConfig.featuresList.size());
    RUNNER_ASSERT(
            std::count(widgetConfig.featuresList.begin(), widgetConfig.featuresList.end(),
                    WrtDB::ConfigParserData::Feature(L"http://tizen.org/privilege/location")) == 1);
    RUNNER_ASSERT(
            std::count(widgetConfig.featuresList.begin(), widgetConfig.featuresList.end(),
                    WrtDB::ConfigParserData::Feature(L"http://tizen.org/privilege/notification")) == 1);

}

/*
Name: PrivilegeElementWrongFormat
Description: Tests parsing privilege elements with wrong format
Expected: No exception. PrivilegeList and featuresList should be empty
*/
RUNNER_TEST(PrivilegeElementWrongFormat)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/config_privilege6.xml",
            ElementParserPtr(
                new RootParser<WidgetParser>(widgetConfig,
                    DPL::
                    FromUTF32String(
                        L"widget"))));

    RUNNER_ASSERT(widgetConfig.privilegeList.empty());
    RUNNER_ASSERT(widgetConfig.featuresList.empty());
}
