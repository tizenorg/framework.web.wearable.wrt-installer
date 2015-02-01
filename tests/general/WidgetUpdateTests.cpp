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
 * @file    WidgetUpdateTests.cpp
 * @author  Grzegorz Rynkowski (g.rynkowski@samsung.com)
 * @version 1.0
 * @brief   Test a process of updating web applications.
 */

#include <string>
#include <dpl/test/test_runner.h>
#include <InstallerWrapper.h>
#include <fstream>

using namespace InstallerWrapper;

#define RUNNER_ASSERT_MSG_SAFE(test, message)                                  \
    {                                                                          \
        DPL::Test::TestRunnerSingleton::Instance().MarkAssertion();            \
                                                                               \
        if (!(test))                                                           \
        {                                                                      \
            uninstall(tizenId);                                                \
            std::ostringstream assertMsg;                                      \
            assertMsg << message;                                              \
            throw DPL::Test::TestRunner::TestFailed(#test,                     \
                                                    __FILE__,                  \
                                                    __LINE__,                  \
                                                    assertMsg.str());          \
        }                                                                      \
    }

RUNNER_TEST_GROUP_INIT(WidgetUpdate)

/*
Name: validUpdateOfSigned
Description: Tests update the web app where origin and a new one are signed.
Expected: Widget should be successfully installed.
*/
RUNNER_TEST(validUpdateOfSigned)
{
    std::string tizenId;
    std::string wgtPath;

    wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer100Signed.wgt";
    RUNNER_ASSERT_MSG(install(wgtPath, tizenId) == InstallerWrapper::Success,
            "Failed to install widget");

    wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer220Signed.wgt";
    RUNNER_ASSERT_MSG_SAFE(install(wgtPath, tizenId) == InstallerWrapper::Success,
            "Failed update in case both programs are signed");
    uninstall(tizenId);
}

/*
Name: validUpdateOfUnsigned
Description: Tests update the web app where origin and a new one are unsigned.
Expected: Widget should be successfully updated.
*/
RUNNER_TEST(validUpdateOfUnsigned)
{
    std::string tizenId;
    std::string wgtPath;

    wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer100Unsigned.wgt";
    RUNNER_ASSERT_MSG(InstallerWrapper::Success == install(wgtPath, tizenId),
            "Failed to install widget");

    wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer220Unsigned.wgt";
    RUNNER_ASSERT_MSG_SAFE(InstallerWrapper::Success == install(wgtPath, tizenId),
                "Failed update in case both programs are signed");
    uninstall(tizenId);
}

/*
 * Information:
 * These tests are incompatible to the specification 2.1
 *      (but compatible to the specification 3.0).
 */
///*
//Name: unupdateOfSigned
//Description: Tests update that signed web app could be downgraded.
//Expected: Widget should not be updated.
//*/
//RUNNER_TEST(unupdateOfSigned)
//{
//    std::string tizenId;
//    std::string wgtPath;
//
//    wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer220Signed.wgt";
//    RUNNER_ASSERT_MSG(InstallerWrapper::Success == install(wgtPath, tizenId),
//            "Failed to install widget");
//
//    wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer100Signed.wgt";
//    RUNNER_ASSERT_MSG_SAFE(InstallerWrapper::Success != install(wgtPath, tizenId),
//            "Unupdate should be prohibited.");
//    uninstall(tizenId);
//}
//
///*
//Name: unupdateOfSigned
//Description: Tests update that unsigned web app could be downgraded.
//Expected: Widget should not be updated.
//*/
//RUNNER_TEST(unupdateOfUnsigned)
//{
//    std::string tizenId;
//    std::string wgtPath;
//
//    wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer220Unsigned.wgt";
//    RUNNER_ASSERT_MSG(InstallerWrapper::Success == install(wgtPath, tizenId),
//            "Failed to install widget");
//
//    wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer100Unsigned.wgt";
//    RUNNER_ASSERT_MSG_SAFE(InstallerWrapper::Success != install(wgtPath, tizenId),
//            "Unupdate should be prohibited.");
//    uninstall(tizenId);
//}

/*
Name: validUpdateOfCrossSigned
Description: Tests update the web app where one of widgets are signed and second not.
Expected: Widget should not be updated.
*/
RUNNER_TEST(updateOfCrossSignedWidgets)
{
    std::string tizenId;
    std::string wgtPath;

    {
        wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer100Unsigned.wgt";
        RUNNER_ASSERT_MSG(InstallerWrapper::Success == install(wgtPath, tizenId),
                "Failed to install widget");

        wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer220Signed.wgt";
        RUNNER_ASSERT_MSG_SAFE(InstallerWrapper::Success != install(wgtPath, tizenId),
                "The update unsigned app by the signed app should not be possible");
        uninstall(tizenId);
    }
    {
        wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer100Signed.wgt";
        RUNNER_ASSERT_MSG(InstallerWrapper::Success == install(wgtPath, tizenId),
                "Failed to install widget");

        wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer220Unsigned.wgt";
        RUNNER_ASSERT_MSG_SAFE(InstallerWrapper::Success != install(wgtPath, tizenId),
                "The update signed app by the unsigned app should not be possible");
        uninstall(tizenId);
    }
}


/*
Name: updateAnotherAuthor
Description: Tests update the web app by the widget signed by another author.
Expected: Widget should not be updated.
*/
RUNNER_TEST(updateAnotherAuthor)
{
    std::string tizenId;
    std::string wgtPath;

    wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer100Signed.wgt";
    RUNNER_ASSERT_MSG(InstallerWrapper::Success == install(wgtPath, tizenId),
            "Failed to install widget");

    wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer220SignedAnotherAuthor.wgt";
    RUNNER_ASSERT_MSG_SAFE(InstallerWrapper::Success != install(wgtPath, tizenId),
            "The update by another author should not be possible");
    uninstall(tizenId);
}


/*
Name: updateWidgetDataRemember
Description: Tests of keeping app data during widget updating.
Expected: App data should be kept.
*/
RUNNER_TEST(updateWidgetDataRemember)
{
    std::string tizenId;
    std::string wgtPath;

    // Installation of the widget
    wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer100Signed.wgt";
    RUNNER_ASSERT_MSG(install(wgtPath, tizenId) == InstallerWrapper::Success,
            "Failed to install widget");

    // Creating a file
    std::string filePath = "/opt/usr/apps/HAdisUJ4Kn/data/test";
    std::string text = "slonce swieci dzisiaj wyjatkowo wysoko";
    std::string command = "echo " + text + " > " + filePath;
    system(command.c_str());

    // Second installation of the widget
    wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer220Signed.wgt";
    RUNNER_ASSERT_MSG_SAFE(InstallerWrapper::Success == install(wgtPath, tizenId),
            "Failed update in case both programs are signed");


    // Checking of the file created before
    std::stringstream ss;
    std::ifstream file(filePath);
    RUNNER_ASSERT_MSG_SAFE(file.good(), "File is gone");

    for( std::string line; getline( file, line ); ss << line);
    file.close();
    RUNNER_ASSERT_MSG_SAFE(text == ss.str(), "Content of file is not the same");
    uninstall(tizenId);
}
