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
 * @author  Tomasz Iwanek (t.iwanek@samsung.com)
 * @version 1.0
 * @brief   Background page installation test's bodies
 */

#include <string>
#include <dpl/test/test_runner.h>
#include <InstallerWrapper.h>

using namespace InstallerWrapper;

////////////////////////////////////////////////////////////////////////////////

RUNNER_TEST_GROUP_INIT(BackgroundPage)

/*
Name: widgetWithBackgroundPage
Description: Tests if widget with background page is installed correctly
Expected: widget should be installed correctly
*/
RUNNER_TEST(widgetWithBackgroundPage)
{
    std::string tizenId;
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/bg-00-with_bg.wgt",
            tizenId) == InstallerWrapper::Success);
    uninstall(tizenId);
}

/*
Name: missingBackgroundFile
Description: Tests if widget with declared in conifg background page
 but missing background file will be installed correctly.
Expected: widget should NOT be installed
*/
RUNNER_TEST(missingBackgroundFile)
{
    std::string tizenId;
    if(install(miscWidgetsStuff + "widgets/bg-01-missing_file.wgt",
            tizenId) == InstallerWrapper::Success) {
        uninstall(tizenId);
        RUNNER_ASSERT_MSG(false, "Invalid widget package installed");
    }
}

/*
Name: widgetWithoutBackgroundPage
Description: Complementary test to check if normal widget\
 without background page is successfully installed
Expected: widget should be installed
*/
RUNNER_TEST(widgetWithoutBackgroundPage)
{
    std::string tizenId;
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/bg-02-without_bg.wgt",
            tizenId) == InstallerWrapper::Success);
    uninstall(tizenId);
}
