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
 * @file    TaskConfigurationTests.cpp
 * @author  Dominik Duda (d.duda@samsung.com)
 * @version 1.0
 * @brief   Tests functions from
 * wrt-installer/src/jobs/widget_install/task_configuration.cpp
 */
#include <string>
#include <installer_log.h>
#include <InstallerWrapper.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/test/test_runner.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <widget_install/widget_install_errors.h>
#include <widget_install/widget_installer_struct.h>
#include <widget_install/task_configuration.h>
#include <widget_install/job_widget_install.h>
#include <installer_callbacks_translate.h>
#include <pkg-manager/pkgmgr_signal_dummy.h>
#include <memory>

using namespace Jobs;
using namespace WidgetInstall;

RUNNER_TEST_GROUP_INIT(TaskConfiguration)

namespace{
const std::string wgtTmpPath = "/tmp/J7ZwBudste";
const std::string wgtPath = InstallerWrapper::miscWidgetsStuff +
        "widgets/widgetUpdateVer100Signed.wgt";
const std::string tarCMD = "tar -xf " + InstallerWrapper::miscWidgetsStuff +
                "widgets/widgetInDir.tar -C /tmp";

std::string tizenId;

void staticWrtInitPreloadStatusCallback(std::string tizenId, WrtErrStatus status,
                                           void* userdata)
{
    return;
}
}

/*
Name: task_configuration_test_01
Description: Installs widget which is needed for further tests.
This installation will test standard use of the TaskConfiguration class.
Expected: The widget should be successfully installed.
*/
RUNNER_TEST(task_configuration_test_01)
{
    //Install the widget to get a tizenID
    InstallerWrapper::install(wgtPath, tizenId);

    //Uninstall the widget in order to be sure that a next installation
    //will be first.
    RUNNER_ASSERT_MSG(InstallerWrapper::uninstall(tizenId),
            "Failed to uninstall a widget");

    //That will be the first installation of the widget.
    RUNNER_ASSERT_MSG(
            InstallerWrapper::install(wgtPath, tizenId) == InstallerWrapper::Success,
            "Failed to install a widget");
}

/*
Name: task_configuration_test_02
Description: Tests recognizing an update installation.
Expected: All task configration steps should be completed without errors.
*/
RUNNER_TEST(task_configuration_test_02)
{
    const WidgetInstallationStruct installerStruct(
                    InstallerCallbacksTranslate::installFinishedCallback,
                    InstallerCallbacksTranslate::installProgressCallback,
                    new InstallerCallbacksTranslate::StatusCallbackStruct(
                            NULL, &staticWrtInitPreloadStatusCallback, NULL),
                    InstallMode(),
                    std::make_shared<PackageManager::PkgmgrSignalDummy>());

    InstallerContext i_context;

    i_context.mode = InstallMode();
    i_context.requestedPath = wgtPath;
    i_context.job = new Jobs::WidgetInstall::JobWidgetInstall(wgtPath, "",
                    installerStruct);

    TaskConfiguration taskConf(i_context);
    size_t stepsCnt = taskConf.GetStepCount();

    unsigned int i = 0;
    bool result = true;

    while(i < stepsCnt && result)
    {
        i++;
        result = taskConf.NextStep();
    }

    RUNNER_ASSERT(i == stepsCnt);
}

/*
Name: task_configuration_test_03
Description: Tests widget installation with incorrect config.xml file.
Expected: Task configuration process should throw an exception when parsing
configuration file.
*/
RUNNER_TEST(task_configuration_test_03)
{
    const WidgetInstallationStruct installerStruct(
                    InstallerCallbacksTranslate::installFinishedCallback,
                    InstallerCallbacksTranslate::installProgressCallback,
                    new InstallerCallbacksTranslate::StatusCallbackStruct(
                            NULL, &staticWrtInitPreloadStatusCallback, NULL),
                    InstallMode(),
                    std::make_shared<PackageManager::PkgmgrSignalDummy>());

    InstallerContext i_context;

    i_context.mode = InstallMode();
    i_context.requestedPath = InstallerWrapper::miscWidgetsStuff + "widgets/widgetFakeConfig.wgt";

    i_context.job =
            new Jobs::WidgetInstall::JobWidgetInstall(wgtPath, "",
                    installerStruct);

    TaskConfiguration taskConf(i_context);
    size_t stepsCnt = taskConf.GetStepCount();

    unsigned int i = 0;
    bool result = true;

    Try{
        while(i < stepsCnt && result)
        {
            i++;
            result = taskConf.NextStep();
        }
    }
    Catch(WidgetInstall::Exceptions::WidgetConfigFileInvalid){
        RUNNER_ASSERT(i == 4);
        return;
    }

    RUNNER_ASSERT_MSG(false,
            "An Exception should be thrown if config.xml file is incorrect.");
}

/*
Name: task_configuration_test_04
Description: Tests task configuration for widget installation directly from
a directory.
Expected: Widget should be successfully installed.
*/
RUNNER_TEST(task_configuration_test_04)
{
    int ret = system(tarCMD.c_str());

    RUNNER_ASSERT_MSG(!WIFEXITED(ret) || !WIFSIGNALED(ret),
            "Cannot untar a widget to check direct installation from the directory!");

    const WidgetInstallationStruct installerStruct(
                    InstallerCallbacksTranslate::installFinishedCallback,
                    InstallerCallbacksTranslate::installProgressCallback,
                    new InstallerCallbacksTranslate::StatusCallbackStruct(
                            NULL, &staticWrtInitPreloadStatusCallback, NULL),
                    InstallMode(),
                    std::make_shared<PackageManager::PkgmgrSignalDummy>());

    InstallerContext i_context;

    i_context.mode = InstallMode();
    i_context.mode.extension = InstallMode::ExtensionType::DIR;
    i_context.requestedPath = wgtTmpPath;

    i_context.job =
            new Jobs::WidgetInstall::JobWidgetInstall(wgtPath, "",
                    installerStruct);

    TaskConfiguration taskConf(i_context);
    size_t stepsCnt = taskConf.GetStepCount();

    unsigned int i = 0;
    bool result = true;

    while(i < stepsCnt && result)
    {
        i++;
        result = taskConf.NextStep();
    }

    RUNNER_ASSERT(i == stepsCnt);
}

/*
Name: task_configuration_test_05
Description: Tests if an exception will ocure when there is no config.xml file
in the widget directory.
Expected: An exception should be thrown.
*/
RUNNER_TEST(task_configuration_test_05)
{
    int ret = system(tarCMD.c_str());

    RUNNER_ASSERT_MSG(!WIFEXITED(ret) || !WIFSIGNALED(ret),
            "Cannot untar widget to check direct installation from directory!");

    const WidgetInstallationStruct installerStruct(
                    InstallerCallbacksTranslate::installFinishedCallback,
                    InstallerCallbacksTranslate::installProgressCallback,
                    new InstallerCallbacksTranslate::StatusCallbackStruct(
                            NULL, &staticWrtInitPreloadStatusCallback, NULL),
                    InstallMode(),
                    std::make_shared<PackageManager::PkgmgrSignalDummy>());

    InstallerContext i_context;

    i_context.mode = InstallMode();
    i_context.mode.extension = InstallMode::ExtensionType::DIR;
    i_context.requestedPath = wgtTmpPath + "/TestWgt.wgt";

    i_context.job =
            new Jobs::WidgetInstall::JobWidgetInstall(wgtPath, "",
                    installerStruct);

    TaskConfiguration taskConf(i_context);
    size_t stepsCnt = taskConf.GetStepCount();

    unsigned int i = 0;
    bool result = true;

    Try{
        while(i < stepsCnt && result)
        {
            i++;

            if (i == 3)
            {
                //Remove config file
                RUNNER_ASSERT(WrtUtilRemove(wgtTmpPath + "/config.xml"));
            }

            result = taskConf.NextStep();
        }
    }
    Catch(WrtDB::WidgetDAOReadOnly::Exception::WidgetNotExist){
        RUNNER_ASSERT(i == 3);
        return;
    }

    RUNNER_ASSERT_MSG(false,
            "An Exception should be thrown after deletion of config.xml file.");
}

/*
Name: task_configuration_test_06
Description: Tests if missing config file will be detected during parsing step.
Expected: An exception should be thrown if there is no config file.
*/
RUNNER_TEST(task_configuration_test_06)
{
    int ret = system(tarCMD.c_str());

    RUNNER_ASSERT_MSG(!WIFEXITED(ret) || !WIFSIGNALED(ret),
            "Cannot untar widget to check direct installation from directory!");

    const WidgetInstallationStruct installerStruct(
                    InstallerCallbacksTranslate::installFinishedCallback,
                    InstallerCallbacksTranslate::installProgressCallback,
                    new InstallerCallbacksTranslate::StatusCallbackStruct(
                            NULL, &staticWrtInitPreloadStatusCallback, NULL),
                    InstallMode(),
                    std::make_shared<PackageManager::PkgmgrSignalDummy>());

    InstallerContext i_context;

    i_context.mode = InstallMode();
    i_context.mode.extension = InstallMode::ExtensionType::DIR;
    i_context.requestedPath = wgtTmpPath;

    i_context.job =
            new Jobs::WidgetInstall::JobWidgetInstall(wgtPath, "",
                    installerStruct);

    TaskConfiguration taskConf(i_context);
    size_t stepsCnt = taskConf.GetStepCount();

    unsigned int i = 0;
    bool result = true;

    Try{
        while(i < stepsCnt && result)
        {
            i++;

            if (i == 4)
            {
                //Remove config file
                RUNNER_ASSERT(WrtUtilRemove(wgtTmpPath + "/config.xml"));
            }

            result = taskConf.NextStep();
        }
    }
    Catch(WidgetInstall::Exceptions::MissingConfig){
        RUNNER_ASSERT(i == 4);
        return;
    }

    RUNNER_ASSERT_MSG(false,
            "An Exception should be thrown in parsing step if there is no "
            "config.xml file in the directory.");
}

/*
Name: task_configuration_test_07
Description: Tests reinstallation of a widget from the directory.
Expected: A widget should be successfully installed.
*/
RUNNER_TEST(task_configuration_test_07)
{
    int ret = system(tarCMD.c_str());

    RUNNER_ASSERT_MSG(!WIFEXITED(ret) || !WIFSIGNALED(ret),
            "Cannot untar widget to check direct installation from directory!");

    //This widget is needed to be installed to find the tizen PkgId in the database
    //during reinstallation step.
    RUNNER_ASSERT_MSG(
            InstallerWrapper::install(wgtTmpPath + "/TestWgt.wgt", tizenId) == InstallerWrapper::Success,
            "Failed to install a widget");

    const WidgetInstallationStruct installerStruct(
                    InstallerCallbacksTranslate::installFinishedCallback,
                    InstallerCallbacksTranslate::installProgressCallback,
                    new InstallerCallbacksTranslate::StatusCallbackStruct(
                            NULL, &staticWrtInitPreloadStatusCallback, NULL),
                    InstallMode(),
                    std::make_shared<PackageManager::PkgmgrSignalDummy>());

    InstallerContext i_context;

    i_context.mode = InstallMode();
    i_context.mode.command = InstallMode::Command::REINSTALL;
    i_context.mode.extension = InstallMode::ExtensionType::DIR;
    i_context.requestedPath = wgtTmpPath;

    i_context.job =
            new Jobs::WidgetInstall::JobWidgetInstall(wgtPath, "",
                    installerStruct);

    TaskConfiguration taskConf(i_context);
    size_t stepsCnt = taskConf.GetStepCount();

    unsigned int i = 0;
    bool result = true;

    while(i < stepsCnt && result)
    {
        i++;
        result = taskConf.NextStep();
    }

    RUNNER_ASSERT(i == stepsCnt);
}

/*
Name: task_configuration_test_08
Description: Tests recovery installation of the widget from the directory.
Expected: A widget should be successfully installed.
*/
RUNNER_TEST(task_configuration_test_08)
{
    int ret = system(tarCMD.c_str());

    RUNNER_ASSERT_MSG(!WIFEXITED(ret) || !WIFSIGNALED(ret),
            "Cannot untar widget to check direct installation from directory!");

    //This widget is needed to be installed to find the tizen PkgId in the database
    //during reinstallation step.
    RUNNER_ASSERT_MSG(
            InstallerWrapper::install(wgtTmpPath + "/TestWgt.wgt", tizenId) == InstallerWrapper::Success,
            "Failed to install a widget");

    const WidgetInstallationStruct installerStruct(
                    InstallerCallbacksTranslate::installFinishedCallback,
                    InstallerCallbacksTranslate::installProgressCallback,
                    new InstallerCallbacksTranslate::StatusCallbackStruct(
                            NULL, &staticWrtInitPreloadStatusCallback, NULL),
                    InstallMode(),
                    std::make_shared<PackageManager::PkgmgrSignalDummy>());

    InstallerContext i_context;

    i_context.mode = InstallMode();
    i_context.mode.command = InstallMode::Command::RECOVERY;
    i_context.mode.extension = InstallMode::ExtensionType::DIR;
    i_context.requestedPath = wgtTmpPath;

    i_context.job =
            new Jobs::WidgetInstall::JobWidgetInstall(wgtPath, "",
                    installerStruct);

    TaskConfiguration taskConf(i_context);
    size_t stepsCnt = taskConf.GetStepCount();

    unsigned int i = 0;
    bool result = true;

    while(i < stepsCnt && result)
    {
        i++;
        result = taskConf.NextStep();
    }

    RUNNER_ASSERT(i == stepsCnt);
}

/*
Name: task_configuration_test_09
Description: Tests if a tizenAppID and tizenPkgID will be generated if were not
set earlier.
Expected: IDs should be properly generated.
*/
RUNNER_TEST(task_configuration_test_09)
{
    const WidgetInstallationStruct installerStruct(
                    InstallerCallbacksTranslate::installFinishedCallback,
                    InstallerCallbacksTranslate::installProgressCallback,
                    new InstallerCallbacksTranslate::StatusCallbackStruct(
                            NULL, &staticWrtInitPreloadStatusCallback, NULL),
                    InstallMode(),
                    std::make_shared<PackageManager::PkgmgrSignalDummy>());

    InstallerContext i_context;

    i_context.mode = InstallMode();
    i_context.requestedPath = wgtPath;

    i_context.job =
            new Jobs::WidgetInstall::JobWidgetInstall(wgtPath, "",
                    installerStruct);

    TaskConfiguration taskConf(i_context);
    size_t stepsCnt = taskConf.GetStepCount();

    unsigned int i = 0;
    bool result = true;

    while(i < stepsCnt && result)
    {
        i++;

        if (i == 5){
            i_context.widgetConfig.tzAppid = L"";
            i_context.widgetConfig.tzPkgid = L"";
            i_context.widgetConfig.configInfo.tizenAppId = boost::none;
            i_context.widgetConfig.configInfo.tizenPkgId = boost::none;
        }

        result = taskConf.NextStep();
    }

    RUNNER_ASSERT(i == stepsCnt &&
            i_context.widgetConfig.tzAppid != L"" &&
            i_context.widgetConfig.tzPkgid != L"");
}

/*
Name: task_configuration_test_10
Description: Tests if a tizenPkgID will be generated if was not set earlier.
Expected: ID should be properly generated.
*/
RUNNER_TEST(task_configuration_test_10)
{
    const WidgetInstallationStruct installerStruct(
                    InstallerCallbacksTranslate::installFinishedCallback,
                    InstallerCallbacksTranslate::installProgressCallback,
                    new InstallerCallbacksTranslate::StatusCallbackStruct(
                            NULL, &staticWrtInitPreloadStatusCallback, NULL),
                    InstallMode(),
                    std::make_shared<PackageManager::PkgmgrSignalDummy>());

    InstallerContext i_context;

    i_context.mode = InstallMode();
    i_context.requestedPath = wgtPath;

    i_context.job =
            new Jobs::WidgetInstall::JobWidgetInstall(wgtPath, "",
                    installerStruct);

    TaskConfiguration taskConf(i_context);
    size_t stepsCnt = taskConf.GetStepCount();

    unsigned int i = 0;
    bool result = true;

    while(i < stepsCnt && result)
    {
        i++;

        if (i == 5){
            i_context.widgetConfig.tzPkgid = L"";
            i_context.widgetConfig.configInfo.tizenPkgId = boost::none;
        }

        result = taskConf.NextStep();
    }

    RUNNER_ASSERT(i == stepsCnt &&
            i_context.widgetConfig.tzPkgid != L"");
}

/*
Name: task_configuration_test_11
Description: Tests if a tizenAppId and tizenPkgID will be generated if
tizenApp ID is too short and tizenPkgID is not set.
Expected: IDs should be properly generated. An exception should be thrown
in step 9 beacuse this widget is already installed.
*/
RUNNER_TEST(task_configuration_test_11)
{
    const WidgetInstallationStruct installerStruct(
                    InstallerCallbacksTranslate::installFinishedCallback,
                    InstallerCallbacksTranslate::installProgressCallback,
                    new InstallerCallbacksTranslate::StatusCallbackStruct(
                            NULL, &staticWrtInitPreloadStatusCallback, NULL),
                    InstallMode(),
                    std::make_shared<PackageManager::PkgmgrSignalDummy>());

    InstallerContext i_context;

    i_context.mode = InstallMode();
    i_context.requestedPath = wgtPath;

    i_context.job =
            new Jobs::WidgetInstall::JobWidgetInstall(wgtPath, "",
                    installerStruct);

    TaskConfiguration taskConf(i_context);
    size_t stepsCnt = taskConf.GetStepCount();

    unsigned int i = 0;
    bool result = true;

    Try{
        while(i < stepsCnt && result)
        {
            i++;

            if (i == 5){
                i_context.widgetConfig.tzPkgid = L"";
                i_context.widgetConfig.configInfo.tizenAppId = L"abcd";
                i_context.widgetConfig.configInfo.tizenPkgId = boost::none;
            }

            result = taskConf.NextStep();
        }
    }
    Catch(Jobs::WidgetInstall::Exceptions::WidgetConfigFileInvalid){
        RUNNER_ASSERT(i == 9 &&
                i_context.widgetConfig.tzPkgid != L"");
        return;
    }

    RUNNER_ASSERT_MSG(false, "An exception should be thrown because this widget"
            " is already installed!");
}

/*
Name: task_configuration_test_12
Description: Tests if a tizenAppId and tizenPkgID will be generated if
tizenApp ID has incorrect characters and tizenPkgID is not set.
Expected: IDs should be properly generated. An exception should be thrown
in step 9 beacuse this widget is already installed.
*/
RUNNER_TEST(task_configuration_test_12)
{
    const WidgetInstallationStruct installerStruct(
                    InstallerCallbacksTranslate::installFinishedCallback,
                    InstallerCallbacksTranslate::installProgressCallback,
                    new InstallerCallbacksTranslate::StatusCallbackStruct(
                            NULL, &staticWrtInitPreloadStatusCallback, NULL),
                    InstallMode(),
                    std::make_shared<PackageManager::PkgmgrSignalDummy>());

    InstallerContext i_context;

    i_context.mode = InstallMode();
    i_context.requestedPath = wgtPath;

    i_context.job =
            new Jobs::WidgetInstall::JobWidgetInstall(wgtPath, "",
                    installerStruct);

    TaskConfiguration taskConf(i_context);
    size_t stepsCnt = taskConf.GetStepCount();

    unsigned int i = 0;
    bool result = true;

    Try{
        while(i < stepsCnt && result)
        {
            i++;

            if (i == 5){
                i_context.widgetConfig.tzPkgid = L"";
                i_context.widgetConfig.configInfo.tizenAppId = L"1234!@#$qw";
                i_context.widgetConfig.configInfo.tizenPkgId = boost::none;

                FOREACH(localizedData, i_context.widgetConfig.configInfo.localizedDataSet)
                {
                    localizedData->second.name = L"1234!@#$qw";
                }
            }

            result = taskConf.NextStep();
        }
    }
    Catch(Jobs::WidgetInstall::Exceptions::WidgetConfigFileInvalid){
        RUNNER_ASSERT(i == 9 &&
                i_context.widgetConfig.tzPkgid != L"");
        return;
    }

    RUNNER_ASSERT_MSG(false, "An exception should be thrown because this widget"
            " is already installed!");
}
