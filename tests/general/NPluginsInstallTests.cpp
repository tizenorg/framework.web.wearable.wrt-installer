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
 * @brief   NPlugins installation test's bodies
 */

#include <string>
#include <dpl/test/test_runner.h>
#include <InstallerWrapper.h>

using namespace InstallerWrapper;

////////////////////////////////////////////////////////////////////////////////

RUNNER_TEST_GROUP_INIT(NPluginsInstall)

/*
Name: pluginFilesAdded
Description: Tests installation of plugins attached to widget
Expected: widget should be succesfully installed
*/
RUNNER_TEST(pluginFilesAdded)
{
    std::string tizenId;
    RUNNER_ASSERT(install(miscWidgetsStuff
            + "widgets/inst_nplug_1.wgt", tizenId) == InstallerWrapper::Success);
    uninstall(tizenId);
}

/*
Name: pluginFileAndOtherFile
Description: Tests installation with plugins directory and data files
Expected: widget should be installed
*/
RUNNER_TEST(pluginFileAndOtherFile)
{
    std::string tizenId;
    RUNNER_ASSERT(install(miscWidgetsStuff
            + "widgets/inst_nplug_3.wgt", tizenId) == InstallerWrapper::Success);
    uninstall(tizenId);
}
