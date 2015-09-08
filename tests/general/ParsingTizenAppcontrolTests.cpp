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
 * @file    TestCases.cpp
 * @author  Karol Pawlowski (k.pawlowski@samsung.com)
 * @author  Andrzej Surdej (a.surdej@samsung.com)
 * @version 1.0
 * @brief   Parsing Tizen app-control test's bodies
 */

#include <string>
#include <algorithm>
#include <dpl/test/test_runner.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <InstallerWrapper.h>
#include <installer_log.h>

using namespace InstallerWrapper;

////////////////////////////////////////////////////////////////////////////////

RUNNER_TEST_GROUP_INIT(ParsingTizenAppcontrol)

/*
Name: tizen_app-contro
Description: Tests if widget app-control tag is correctly parsed
Expected: widget should be installed
*/
RUNNER_TEST(tizen_app_control)
{
    std::string tizenId;
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/app-control.wgt",
            tizenId) == InstallerWrapper::Success);

    WrtDB::WidgetDAOReadOnly dao(DPL::FromASCIIString(tizenId));
    WrtDB::WidgetAppControlList appcontrolList;
    dao.getAppControlList(appcontrolList);
    uninstall(tizenId);

    _D("Actual size %d", appcontrolList.size());
    RUNNER_ASSERT_MSG(appcontrolList.size() == 4, "Incorrect list size");
    WrtDB::WidgetAppControl s;
    s.src = DPL::FromUTF8String("edit1.html");
    s.operation = DPL::FromUTF8String("http://tizen.org/appcontrol/operation/edit");
    s.mime = DPL::FromUTF8String("image/jpg");      /* mime type */
    s.disposition = WrtDB::WidgetAppControl::Disposition::WINDOW;

    RUNNER_ASSERT_MSG(
        std::find(appcontrolList.begin(), appcontrolList.end(), s) != appcontrolList.end(),
        "Unable to find service #");

    s.src = DPL::FromUTF8String("edit2.html");
    s.operation = DPL::FromUTF8String("http://tizen.org/appcontrol/operation/view");
    s.mime = DPL::FromUTF8String("audio/ogg");      /* mime type */
    s.disposition = WrtDB::WidgetAppControl::Disposition::WINDOW;

    RUNNER_ASSERT_MSG(
        std::find(appcontrolList.begin(), appcontrolList.end(), s) != appcontrolList.end(),
        "Unable to find service ##");

    s.src = DPL::FromUTF8String("edit3.html");
    s.operation = DPL::FromUTF8String("http://tizen.org/appcontrol/operation/call");
    s.mime = DPL::FromUTF8String("image/png");      /* mime type */
    s.disposition = WrtDB::WidgetAppControl::Disposition::WINDOW;

    RUNNER_ASSERT_MSG(
        std::find(appcontrolList.begin(), appcontrolList.end(), s) != appcontrolList.end(),
        "Unable to find service ###");

    s.src = DPL::FromUTF8String("edit4.html");
    s.operation = DPL::FromUTF8String("http://tizen.org/appcontrol/operation/send");
    s.mime = DPL::FromUTF8String("text/css");      /* mime type */
    s.disposition = WrtDB::WidgetAppControl::Disposition::WINDOW;

    RUNNER_ASSERT_MSG(
        std::find(appcontrolList.begin(), appcontrolList.end(), s) != appcontrolList.end(),
        "Unable to find service ####");
}
