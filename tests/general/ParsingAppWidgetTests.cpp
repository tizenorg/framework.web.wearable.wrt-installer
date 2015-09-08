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
 * @file    ParsingAppWidgetTests.cpp
 * @author  Slawomir Pajak (s.pajak@partner.samsung.com)
 * @version 1.0
 * @brief   Parsing Tizen app-widget bodies
 */

#include <string>
#include <algorithm>
#include <dpl/test/test_runner.h>
#include <InstallerWrapper.h>
#include <ManifestFile.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/wrt-dao-ro/global_config.h>

#include <root_parser.h>
#include <widget_parser.h>
#include <parser_runner.h>

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

using namespace InstallerWrapper;

////////////////////////////////////////////////////////////////////////////////

RUNNER_TEST_GROUP_INIT(ParsingAppWidget)

/*
Name: InstallWidgetWithAppWidgetFull
Description: Tests if app-widget tag is correctly parsed when all children are included
Expected: widget should be installed. All information should be stored in manifest file
*/
RUNNER_TEST(InstallWidgetWithAppWidgetFull)
{
    std::string manifestPath = "/opt/share/packages/";

    std::string tizenId;
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/appWidgetFull.wgt", tizenId) == InstallerWrapper::Success);

    RUNNER_ASSERT(WrtUtilFileExists(manifestPath.append(tizenId.substr(0, 10)).append(".xml")));
    ManifestFile mf(manifestPath);
    WrtDB::WidgetDAOReadOnly dao(DPL::FromASCIIString(tizenId));

    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/@appid") == "jeyk39ehc8.appwidget.default");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/@primary") == "true");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/@period") == "1800.0");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:launch") == "true");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:label") == "My Widget");
    std::string iconPath = DPL::ToUTF8String(*dao.getWidgetInstalledPath()) + WrtDB::GlobalConfig::GetWidgetSharedPath() +
            WrtDB::GlobalConfig::GetWidgetDataPath() + "/" + tizenId + ".default.icon.png";
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:icon") == iconPath);
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:box/@mouse_event") == "false");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:box/@touch_effect") == "false");
    std::string boxSrcPath = DPL::ToUTF8String(*dao.getWidgetInstalledPath()) + WrtDB::GlobalConfig::GetWidgetSrcPath()
            + "/app-widget/index.html";
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:box/p:script/@src") == boxSrcPath);
    std::string boxPreviewPath = DPL::ToUTF8String(*dao.getWidgetInstalledPath()) + WrtDB::GlobalConfig::GetWidgetSharedPath() +
            WrtDB::GlobalConfig::GetWidgetDataPath() + "/" + tizenId + ".default.1x1.preview.png";
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:box/p:size/@preview") == boxPreviewPath);
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:box/p:size/@need_frame") == "false");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:box/p:size") == "1x1");
    std::string pdSrcPath = DPL::ToUTF8String(*dao.getWidgetInstalledPath()) + WrtDB::GlobalConfig::GetWidgetSrcPath()
            + "/pd/index.html";
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:pd/p:script/@src") == pdSrcPath);
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:pd/p:size") == "720x150");

    RUNNER_ASSERT(uninstall(tizenId));
}

/*
Name: InstallWidgetWithAppWidgetMinimal
Description: Tests if app-widget tag is correctly parsed when only mandatory children are included
Expected: widget should be installed. All information should be stored in manifest file
*/
RUNNER_TEST(InstallWidgetWithAppWidgetMinimal)
{
    std::string manifestPath = "/opt/share/packages/";

    std::string tizenId;
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/appWidgetMinimal.wgt", tizenId) == InstallerWrapper::Success);

    RUNNER_ASSERT(WrtUtilFileExists(manifestPath.append(tizenId.substr(0, 10)).append(".xml")));
    ManifestFile mf(manifestPath);
    WrtDB::WidgetDAOReadOnly dao(DPL::FromASCIIString(tizenId));

    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/@appid") == "djel94jdl9.appwidget.default");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/@primary") == "true");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:label") == "My Widget");
    std::string boxSrcPath = DPL::ToUTF8String(*dao.getWidgetInstalledPath()) + WrtDB::GlobalConfig::GetWidgetSrcPath()
            + "/app-widget/index.html";
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:box/p:script/@src") == boxSrcPath);
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:livebox/p:box/p:size") == "1x1");

    RUNNER_ASSERT(uninstall(tizenId));
}

/*
Name: InstallWidgetWithAppWidgetIncorrect
Description: Tests widget installation when app-widget Id and application Id are different
Expected: widget should not be installed.
*/
RUNNER_TEST(InstallWidgetWithAppWidgetIncorrect)
{
    std::string tizenId;
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/appWidgetIncorrect.wgt", tizenId) != InstallerWrapper::Success);
}


/*
Name: AppWidgetFull
Description: Tests parsing app-widget element with all children
Expected: Elements should be parsed correctly.
*/
RUNNER_TEST(AppWidgetFull)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/AppWidgetFull.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(L"tizenScmgz.Sample.default" == (**widgetConfig.m_livebox.begin()).m_liveboxId);
    RUNNER_ASSERT(L"true" == (**widgetConfig.m_livebox.begin()).m_primary);
    RUNNER_ASSERT(L"1800.0" == (**widgetConfig.m_livebox.begin()).m_updatePeriod);
    RUNNER_ASSERT(L"true" == (**widgetConfig.m_livebox.begin()).m_autoLaunch);
    RUNNER_ASSERT(L"My Widget" == (**widgetConfig.m_livebox.begin()).m_label.begin()->second);
    RUNNER_ASSERT(L"box-icon.png" == (**widgetConfig.m_livebox.begin()).m_icon);
    RUNNER_ASSERT(L"app-widget/index.html" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSrc);
    RUNNER_ASSERT(L"true" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxMouseEvent);
    RUNNER_ASSERT(L"false" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxTouchEffect);
    RUNNER_ASSERT(1 == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.size());
    RUNNER_ASSERT(L"app-widget/preview-lb1-11.png" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.begin()->m_preview);
    RUNNER_ASSERT(L"false" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.begin()->m_useDecoration);
    RUNNER_ASSERT(L"1x1" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.begin()->m_size);
    RUNNER_ASSERT(L"pd/index.html" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_pdSrc);
    RUNNER_ASSERT(L"720" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_pdWidth);
    RUNNER_ASSERT(L"150" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_pdHeight);
}


/*
Name: AppWidgetMinimal
Description: Tests parsing app-widget element with mandatory children
Expected: Elements should be parsed correctly.
*/
RUNNER_TEST(AppWidgetMinimal)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/AppWidgetMinimal.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(L"tizenScmgz.Sample.default" == (**widgetConfig.m_livebox.begin()).m_liveboxId);
    RUNNER_ASSERT(L"true" == (**widgetConfig.m_livebox.begin()).m_primary);
    RUNNER_ASSERT(L"My Widget" == (**widgetConfig.m_livebox.begin()).m_label.begin()->second);
    RUNNER_ASSERT(L"app-widget/index.html" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSrc);
    RUNNER_ASSERT(L"1x1" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.begin()->m_size);
}

/*
Name: AppWidgetIdTooShort
Description: Tests parsing app-widget element with too short id
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetIdTooShort)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
            parser.Parse(miscWidgetsStuff + "configs/AppWidgetIdTooShort.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
            );
}

/*
Name: AppWidgetIdTooLong
Description: Tests parsing app-widget element with too long id
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetIdTooLong)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
            parser.Parse(miscWidgetsStuff + "configs/AppWidgetIdTooLong.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
            );
}

/*
Name: AppWidgetIdWrongChar
Description: Tests parsing app-widget element with ill-formed id
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetIdWrongChar)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
            parser.Parse(miscWidgetsStuff + "configs/AppWidgetIdWrongChar.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
            );
}

/*
Name: AppWidgetIdEmpty
Description: Tests parsing app-widget element with empty id
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetIdEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
            parser.Parse(miscWidgetsStuff + "configs/AppWidgetIdEmpty.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
            );
}

/*
Name: AppWidgetNoId
Description: Tests parsing app-widget element without id
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetNoId)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
            parser.Parse(miscWidgetsStuff + "configs/AppWidgetNoId.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
            );
}

/*
Name: AppWidgetPrimaryWrongValue
Description: Tests parsing app-widget element with wrong primary argument
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetPrimaryWrongValue)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
            parser.Parse(miscWidgetsStuff + "configs/AppWidgetPrimaryWrongValue.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
            );
}

/*
Name: AppWidgetPrimaryEmpty
Description: Tests parsing app-widget element with empty primary argument
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetPrimaryEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
            parser.Parse(miscWidgetsStuff + "configs/AppWidgetPrimaryEmpty.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
            );
}

/*
Name: AppWidgetNoPrimary
Description: Tests parsing app-widget element without primary argument
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetNoPrimary)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
            parser.Parse(miscWidgetsStuff + "configs/AppWidgetNoPrimary.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
            );
}

/*
Name: AppWidgetMultiplePrimary
Description: Tests parsing configuration with multiple app-widget element.
Expected: Parsing should be successful.
*/
RUNNER_TEST(AppWidgetMultiplePrimary)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/AppWidgetMultiplePrimary.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(3 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(2 == std::count_if(widgetConfig.m_livebox.begin(), widgetConfig.m_livebox.end(),
            [](const WrtDB::ConfigParserData::OptionalLiveboxInfo& liveBox){
                return liveBox->m_primary == L"true";
            })
    );
    RUNNER_ASSERT(1 == std::count_if(widgetConfig.m_livebox.begin(), widgetConfig.m_livebox.end(),
            [](const WrtDB::ConfigParserData::OptionalLiveboxInfo& liveBox){
                return liveBox->m_primary == L"false";
            })
    );
}

/*
Name: AppWidgetUpdatePeriodLow
Description: Tests parsing app-widget element with update-period argument too low
Expected: Parsing should be successful. updatePeriod should have low boundary value.
*/
RUNNER_TEST(AppWidgetUpdatePeriodLow)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/AppWidgetUpdatePeriodLow.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(L"1800.0" == (**widgetConfig.m_livebox.begin()).m_updatePeriod);
}

/*
Name: AppWidgetUpdatePeriodWrongFormat
Description: Tests parsing app-widget element with wrong update-period argument.
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetUpdatePeriodWrongFormat)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
            parser.Parse(miscWidgetsStuff + "configs/AppWidgetUpdatePeriodWrongFormat.xml",
                        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
             );
}

/*
Name: AppWidgetUpdatePeriodEmpty
Description: Tests parsing app-widget element with empty update-period argument.
Expected: Parsing should be successful. updatePeriod should have empty value.
*/
RUNNER_TEST(AppWidgetUpdatePeriodEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/AppWidgetUpdatePeriodEmpty.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT((**widgetConfig.m_livebox.begin()).m_updatePeriod.empty());
}

/*
Name: RUNNER_TEST(AppWidgetAutoLaunchWrongValue)
Description: Tests parsing app-widget element with wrong auto-launch argument.
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetAutoLaunchWrongValue)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
            parser.Parse(miscWidgetsStuff + "configs/AppWidgetAutoLaunchWrongValue.xml",
            ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
            );
}

/*
Name: AppWidgetAutoLaunchEmpty
Description: Tests parsing app-widget element with empty auto-launch argument.
Expected: Parsing should be successful. auto-launch should have empty value.
*/
RUNNER_TEST(AppWidgetAutoLaunchEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/AppWidgetAutoLaunchEmpty.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(L"false" == (**widgetConfig.m_livebox.begin()).m_autoLaunch);
}

/*
Name: BoxLabelEmpty
Description: Tests parsing empty box-label element.
Expected: Parsing should be successful. label should have empty value.
*/
RUNNER_TEST(BoxLabelEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/BoxLabelEmpty.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(1 == (**widgetConfig.m_livebox.begin()).m_label.size());
    RUNNER_ASSERT((**widgetConfig.m_livebox.begin()).m_label.begin()->first.empty());
    RUNNER_ASSERT((**widgetConfig.m_livebox.begin()).m_label.begin()->second.empty());
}

/*
Name: AppWidgetNoBoxLabel
Description: Tests parsing app-widget element without box-label element.
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetNoBoxLabel)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/AppWidgetNoBoxLabel.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: AppWidgetMultipleBoxLabel
Description: Tests parsing app-widget element with multiple box-label element.
Expected: Parsing should be successful and elements stored correctly.
*/
RUNNER_TEST(AppWidgetMultipleBoxLabel)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/AppWidgetMultipleBoxLabel.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(3 == (**widgetConfig.m_livebox.begin()).m_label.size());

    RUNNER_ASSERT(
       1 == std::count((**widgetConfig.m_livebox.begin()).m_label.begin(), (**widgetConfig.m_livebox.begin()).m_label.end(),
            std::pair<DPL::String, DPL::String>(L"en",L"test_en") ));
    RUNNER_ASSERT(
        1 == std::count((**widgetConfig.m_livebox.begin()).m_label.begin(), (**widgetConfig.m_livebox.begin()).m_label.end(),
                std::pair<DPL::String, DPL::String>(L"pl",L"test_pl") ));
    RUNNER_ASSERT(
        1 == std::count((**widgetConfig.m_livebox.begin()).m_label.begin(), (**widgetConfig.m_livebox.begin()).m_label.end(),
                std::pair<DPL::String, DPL::String>(L"",L"test") ));

}

/*
Name: BoxIconEmpty
Description: Tests parsing empty box-icon element.
Expected: Exception should be thrown.
*/
RUNNER_TEST(BoxIconEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/BoxIconEmpty.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: BoxIconSrcEmpty
Description: Tests parsing box-icon element with empty src attribute.
Expected: Exception should be thrown.
*/
RUNNER_TEST(BoxIconSrcEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/BoxIconSrcEmpty.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: AppWidgetMultipleBoxIcon
Description: Tests parsing app-widget with multiple box-icon elements.
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetMultipleBoxIcon)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/AppWidgetMultipleBoxIcon.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: AppWidgetNoBoxContent
Description: Tests parsing app-widget without box-content element.
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetNoBoxContent)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/AppWidgetNoBoxContent.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}


/*
Name: AppWidgetMultipleBoxContent
Description: Tests parsing app-widget with multiple box-content element.
Expected: Exception should be thrown.
*/
RUNNER_TEST(AppWidgetMultipleBoxContent)
{

    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/AppWidgetMultipleBoxContent.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}


/*
Name: BoxContentEmpty
Description: Tests parsing empty box-content element.
Expected: Exception should be thrown.
*/
RUNNER_TEST(BoxContentEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/BoxContentEmpty.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: BoxContentNoSrc
Description: Tests parsing box-content element without src attribute.
Expected: Exception should be thrown.
*/
RUNNER_TEST(BoxContentNoSrc)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/BoxContentNoSrc.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: BoxContentSrcEmpty
Description: Tests parsing box-content element with empty src attribute.
Expected: Exception should be thrown.
*/
RUNNER_TEST(BoxContentSrcEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/BoxContentSrcEmpty.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: BoxContentNoMouseEvent
Description: Tests parsing box-content element without mouse-event attribute.
Expected: Parsing should be successful. boxMouseEvent should have default value.
*/
RUNNER_TEST(BoxContentNoMouseEvent)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/BoxContentNoMouseEvent.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(L"false" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxMouseEvent);
}

/*
Name: BoxContentMouseEventEmpty
Description: Tests parsing box-content element with empty mouse-event attribute.
Expected: Parsing should be successful. boxMouseEvent should have default value.
*/
RUNNER_TEST(BoxContentMouseEventEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/BoxContentMouseEventEmpty.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(L"false" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxMouseEvent);
}

/*
Name: BoxContentMouseEventWrongValue
Description: Tests parsing box-content element with wrong mouse-event attribute.
Expected: Exception should be thrown.
*/
RUNNER_TEST(BoxContentMouseEventWrongValue)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/BoxContentMouseEventWrongValue.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: BoxContentNoTouchEfect
Description: Tests parsing box-content element without touch-effect attribute.
Expected: Parsing should be successful. boxTouchEffect should have default value.
*/
RUNNER_TEST(BoxContentNoTouchEfect)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/BoxContentNoTouchEfect.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(L"true" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxTouchEffect);
}

/*
Name: BoxContentTouchEfectEmpty
Description: Tests parsing box-content element with empty touch-effect attribute.
Expected: Parsing should be successful. boxTouchEffect should have default value.
*/
RUNNER_TEST(BoxContentTouchEfectEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/BoxContentTouchEfectEmpty.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(L"true" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxTouchEffect);
}

/*
Name: BoxContentTouchEfectWrongValue
Description: Tests parsing box-content element with wrong touch-effect attribute.
Expected: Exception should be thrown.
*/
RUNNER_TEST(BoxContentTouchEfectWrongValue)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/BoxContentTouchEfectWrongValue.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: BoxContentMultipleBoxSize
Description: Tests parsing box-content element with multiple box-size elements.
Expected: Parsing should be successful.
*/
RUNNER_TEST(BoxContentMultipleBoxSize)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/BoxContentMultipleBoxSize.xml",
           ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(3 == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.size());

    RUNNER_ASSERT(1 == std::count_if((**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.begin(),
            (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.end(),
            [](const WrtDB::ConfigParserData::LiveboxInfo::BoxSizeInfo& boxSize){
                return boxSize.m_size == L"1x1";
            })
    );
    RUNNER_ASSERT(1 == std::count_if((**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.begin(),
            (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.end(),
            [](const WrtDB::ConfigParserData::LiveboxInfo::BoxSizeInfo& boxSize){
                return boxSize.m_size == L"2x1";
            })
    );
    RUNNER_ASSERT(1 == std::count_if((**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.begin(),
            (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.end(),
            [](const WrtDB::ConfigParserData::LiveboxInfo::BoxSizeInfo& boxSize){
                return boxSize.m_size == L"2x2";
            })
    );
}

/*
Name: BoxSizeEmpty
Description: Tests parsing empty box-size element.
Expected: Exception should be thrown.
*/
RUNNER_TEST(BoxSizeEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/BoxSizeEmpty.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: BoxSizePreviewEmpty
Description: Tests parsing box-size element with empty preview attribute.
Expected: Parsing should be successful. Preview value should be empty
*/
RUNNER_TEST(BoxSizePreviewEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/BoxSizePreviewEmpty.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(1 == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.size());
    RUNNER_ASSERT((**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.begin()->m_preview.empty());
}

/*
Name: BoxSizeNoUserDecoration
Description: Tests parsing box-size element without use-decoration attribute.
Expected: Parsing should be successful. useDecoration should be set to default value
*/
RUNNER_TEST(BoxSizeNoUserDecoration)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/BoxSizeNoUserDecoration.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(1 == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.size());
    RUNNER_ASSERT(L"true" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.begin()->m_useDecoration);
}

/*
Name: BoxSizeUserDecorationEmpty
Description: Tests parsing box-size element with empty use-decoration attribute.
Expected: Parsing should be successful. useDecoration should be set to default value
*/
RUNNER_TEST(BoxSizeUserDecorationEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/BoxSizeUserDecorationEmpty.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(1 == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.size());
    RUNNER_ASSERT(L"true" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_boxSize.begin()->m_useDecoration);
}

/*
Name: BoxContentMultiplePd
Description: Tests parsing box-content element with multiple pd element.
Expected: Exception should be thrown
*/
RUNNER_TEST(BoxContentMultiplePd)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/BoxContentMultiplePd.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: PdNoSrc
Description: Tests parsing pd element without src attribute.
Expected: Exception should be thrown
*/
RUNNER_TEST(PdNoSrc)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/PdNoSrc.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: PdSrcEmpty
Description: Tests parsing pd element with empty src attribute.
Expected: Exception should be thrown
*/
RUNNER_TEST(PdSrcEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/PdSrcEmpty.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: PdNoWidth
Description: Tests parsing pd element without width attribute.
Expected: Exception should be thrown
*/
RUNNER_TEST(PdNoWidth)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/PdNoWidth.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: PdWidthEmpty
Description: Tests parsing pd element with empty width attribute.
Expected: Exception should be thrown
*/
RUNNER_TEST(PdWidthEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/PdWidthEmpty.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: PdWidthZero
Description: Tests parsing pd element with width zero value.
Expected: Parsing should be successful.
*/
RUNNER_TEST(PdWidthZero)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/PdWidthZero.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(L"0" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_pdWidth);
}

/*
Name: PdWidthNegative
Description: Tests parsing pd element with width negative value.
Expected: Parsing should be successful.
*/
RUNNER_TEST(PdWidthNegative)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/PdWidthNegative.xml",
        ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(L"-1" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_pdWidth);
}

/*
Name: PdWidthWrongValue
Description: Tests parsing pd element with width wrong value.
Expected: Exception should be thrown
*/
RUNNER_TEST(PdWidthWrongValue)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/PdWidthWrongValue.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: PdNoHeight
Description: Tests parsing pd element without height attribute.
Expected: Exception should be thrown
*/
RUNNER_TEST(PdNoHeight)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/PdNoHeight.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: PdHeightEmpty
Description: Tests parsing pd element with empty height attribute.
Expected: Exception should be thrown
*/
RUNNER_TEST(PdHeightEmpty)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/PdHeightEmpty.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}

/*
Name: PdHeightTooLow
Description: Tests parsing pd element with height attribute below range.
Expected: Parsing should be successful. Height should have low boundary value.
*/
RUNNER_TEST(PdHeightTooLow)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/PdHeightTooLow.xml",
       ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(L"1" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_pdHeight);
}

/*
Name: PdHeightExcessive
Description: Tests parsing pd element with height attribute with value above range.
Expected: Parsing should be successful. Height should have high boundary value.
*/
RUNNER_TEST(PdHeightExcessive)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    parser.Parse(miscWidgetsStuff + "configs/PdHeightExcessive.xml",
       ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));

    RUNNER_ASSERT(1 == widgetConfig.m_livebox.size());
    RUNNER_ASSERT(L"380" == (**widgetConfig.m_livebox.begin()).m_boxInfo.m_pdHeight);
}

/*
Name: PdHeightWrongValue
Description: Tests parsing pd element with wrong height attribute.
Expected: Exception should be thrown
*/
RUNNER_TEST(PdHeightWrongValue)
{
    ParserRunner parser;
    WrtDB::ConfigParserData widgetConfig;

    RUNNER_ASSERT_EXCEPTION(ElementParser::Exception::ParseError,
        parser.Parse(miscWidgetsStuff + "configs/PdHeightWrongValue.xml",
                ElementParserPtr(new RootParser<WidgetParser>(widgetConfig, L"widget")));
    );
}
