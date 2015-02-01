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
 *
 */
 /**
 * @file    WidgetLocationTests.cpp
 * @author  Maciej Piotrowski (m.piotrowski@samsung.com)
 * @version 1.0
 * @brief   WidgetLocation tests
 */

#include <dpl/test/test_runner.h>
#include <dpl/utils/wrt_utility.h>
#include <widget_location.h>

RUNNER_TEST_GROUP_INIT(WidgetLocation)


/*
Name: WidgetLocationCleanInit
Description: Tests WidgetLocation creation for WidgetLocation::WidgetLocation
*/
RUNNER_TEST(WidgetLocationCleanInit)
{
    WidgetLocation wl;

    RUNNER_ASSERT(wl.getInstallationDir() == "");
    RUNNER_ASSERT(wl.getPackageInstallationDir() == "/");
    RUNNER_ASSERT(wl.getSourceDir() == "//res/wgt");
    RUNNER_ASSERT(wl.getBinaryDir() == "//bin");
    RUNNER_ASSERT(wl.getUserBinaryDir() == "/opt/usr/apps///bin");
    RUNNER_ASSERT(wl.getExecFile() == "//bin/");
    RUNNER_ASSERT(wl.getBackupDir() == "/.backup");
    RUNNER_ASSERT(wl.getBackupSourceDir() == "/.backup/res/wgt");
    RUNNER_ASSERT(wl.getBackupBinaryDir() == "/.backup/bin");
    RUNNER_ASSERT(wl.getBackupExecFile() == "/.backup/bin/");
    RUNNER_ASSERT(wl.getBackupPrivateDir() == "/.backup/data");
    RUNNER_ASSERT(wl.getUserDataRootDir() == "/opt/usr/apps/");
    RUNNER_ASSERT(wl.getPrivateStorageDir() == "/opt/usr/apps//data");
    RUNNER_ASSERT(wl.getPrivateTempStorageDir() == "/opt/usr/apps//tmp");
    RUNNER_ASSERT(wl.getSharedRootDir() == "/opt/usr/apps//shared");
    RUNNER_ASSERT(wl.getSharedResourceDir() == "/opt/usr/apps//shared/res");
    RUNNER_ASSERT(wl.getSharedDataDir() == "/opt/usr/apps//shared/data");
    RUNNER_ASSERT(wl.getSharedTrustedDir() == "/opt/usr/apps//shared/trusted");
    RUNNER_ASSERT(wl.getBackupSharedDir() == "/.backup/shared");
    RUNNER_ASSERT(wl.getBackupSharedDataDir() == "/.backup/shared/data");
    RUNNER_ASSERT(wl.getBackupSharedTrustedDir() == "/.backup/shared/trusted");
    RUNNER_ASSERT(wl.getNPPluginsDir() == "//res/wgt/plugins");
    RUNNER_ASSERT(wl.getNPPluginsExecFile() == "//bin/.npruntime");
    RUNNER_ASSERT(WrtUtilDirExists(wl.getTemporaryPackageDir()));
    RUNNER_ASSERT(wl.getTemporaryRootDir() == "//res/wgt");
    RUNNER_ASSERT(wl.getInstalledIconPath() == "");
    RUNNER_ASSERT(wl.getWidgetSource() == "");
    RUNNER_ASSERT(wl.getPkgId() == DPL::String(L""));
}

/*
Name: WidgetLocationPkgIdInit
Description: Tests WidgetLocation creation for WidgetLocation::WidgetLocation wirh pkgid
*/
RUNNER_TEST(WidgetLocationPkgIdInit)
{
    WidgetLocation wl("1234567890");

    RUNNER_ASSERT(wl.getInstallationDir() == "");
    RUNNER_ASSERT(wl.getPackageInstallationDir() == "/1234567890");
    RUNNER_ASSERT(wl.getSourceDir() == "/1234567890/res/wgt");
    RUNNER_ASSERT(wl.getBinaryDir() == "/1234567890/bin");
    RUNNER_ASSERT(wl.getUserBinaryDir() == "/opt/usr/apps/1234567890//bin");
    RUNNER_ASSERT(wl.getExecFile() == "/1234567890/bin/");
    RUNNER_ASSERT(wl.getBackupDir() == "/1234567890.backup");
    RUNNER_ASSERT(wl.getBackupSourceDir() == "/1234567890.backup/res/wgt");
    RUNNER_ASSERT(wl.getBackupBinaryDir() == "/1234567890.backup/bin");
    RUNNER_ASSERT(wl.getBackupExecFile() == "/1234567890.backup/bin/");
    RUNNER_ASSERT(wl.getBackupPrivateDir() == "/1234567890.backup/data");
    RUNNER_ASSERT(wl.getUserDataRootDir() == "/opt/usr/apps/1234567890");
    RUNNER_ASSERT(wl.getPrivateStorageDir() == "/opt/usr/apps/1234567890/data");
    RUNNER_ASSERT(wl.getPrivateTempStorageDir() == "/opt/usr/apps/1234567890/tmp");
    RUNNER_ASSERT(wl.getSharedRootDir() == "/opt/usr/apps/1234567890/shared");
    RUNNER_ASSERT(wl.getSharedResourceDir() == "/opt/usr/apps/1234567890/shared/res");
    RUNNER_ASSERT(wl.getSharedDataDir() == "/opt/usr/apps/1234567890/shared/data");
    RUNNER_ASSERT(wl.getSharedTrustedDir() == "/opt/usr/apps/1234567890/shared/trusted");
    RUNNER_ASSERT(wl.getBackupSharedDir() == "/1234567890.backup/shared");
    RUNNER_ASSERT(wl.getBackupSharedDataDir() == "/1234567890.backup/shared/data");
    RUNNER_ASSERT(wl.getBackupSharedTrustedDir() == "/1234567890.backup/shared/trusted");
    RUNNER_ASSERT(wl.getNPPluginsDir() == "/1234567890/res/wgt/plugins");
    RUNNER_ASSERT(wl.getNPPluginsExecFile() == "/1234567890/bin/.npruntime");
    RUNNER_ASSERT(WrtUtilDirExists(wl.getTemporaryPackageDir()));
    RUNNER_ASSERT(wl.getTemporaryRootDir() == "/1234567890/res/wgt");
    RUNNER_ASSERT(wl.getInstalledIconPath() == "");
    RUNNER_ASSERT(wl.getWidgetSource() == "");
    RUNNER_ASSERT(wl.getPkgId() == DPL::String(L"1234567890"));
}

/*
Name: WidgetLocationPkgIdInitAppId
Description: Tests WidgetLocation creation for WidgetLocation::WidgetLocation
             with pkgid and appid
*/
RUNNER_TEST(WidgetLocationPkgIdInitAppId)
{
    WidgetLocation wl("1234567890");
    wl.registerAppid("id123456");

    RUNNER_ASSERT(wl.getInstallationDir() == "");
    RUNNER_ASSERT(wl.getPackageInstallationDir() == "/1234567890");
    RUNNER_ASSERT(wl.getSourceDir() == "/1234567890/res/wgt");
    RUNNER_ASSERT(wl.getBinaryDir() == "/1234567890/bin");
    RUNNER_ASSERT(wl.getUserBinaryDir() == "/opt/usr/apps/1234567890//bin");
    RUNNER_ASSERT(wl.getExecFile() == "/1234567890/bin/id123456");
    RUNNER_ASSERT(wl.getBackupDir() == "/1234567890.backup");
    RUNNER_ASSERT(wl.getBackupSourceDir() == "/1234567890.backup/res/wgt");
    RUNNER_ASSERT(wl.getBackupBinaryDir() == "/1234567890.backup/bin");
    RUNNER_ASSERT(wl.getBackupExecFile() == "/1234567890.backup/bin/id123456");
    RUNNER_ASSERT(wl.getBackupPrivateDir() == "/1234567890.backup/data");
    RUNNER_ASSERT(wl.getUserDataRootDir() == "/opt/usr/apps/1234567890");
    RUNNER_ASSERT(wl.getPrivateStorageDir() == "/opt/usr/apps/1234567890/data");
    RUNNER_ASSERT(wl.getPrivateTempStorageDir() == "/opt/usr/apps/1234567890/tmp");
    RUNNER_ASSERT(wl.getSharedRootDir() == "/opt/usr/apps/1234567890/shared");
    RUNNER_ASSERT(wl.getSharedResourceDir() == "/opt/usr/apps/1234567890/shared/res");
    RUNNER_ASSERT(wl.getSharedDataDir() == "/opt/usr/apps/1234567890/shared/data");
    RUNNER_ASSERT(wl.getSharedTrustedDir() == "/opt/usr/apps/1234567890/shared/trusted");
    RUNNER_ASSERT(wl.getBackupSharedDir() == "/1234567890.backup/shared");
    RUNNER_ASSERT(wl.getBackupSharedDataDir() == "/1234567890.backup/shared/data");
    RUNNER_ASSERT(wl.getBackupSharedTrustedDir() == "/1234567890.backup/shared/trusted");
    RUNNER_ASSERT(wl.getNPPluginsDir() == "/1234567890/res/wgt/plugins");
    RUNNER_ASSERT(wl.getNPPluginsExecFile() == "/1234567890/bin/id123456.npruntime");
    RUNNER_ASSERT(WrtUtilDirExists(wl.getTemporaryPackageDir()));
    RUNNER_ASSERT(wl.getTemporaryRootDir() == "/1234567890/res/wgt");
    RUNNER_ASSERT(wl.getWidgetSource() == "");
    RUNNER_ASSERT(wl.getPkgId() == DPL::String(L"1234567890"));
}

/*
Name: WidgetLocationExternalLocations
Description: Tests WidgetLocation::listExternalLocations() and WidgetLocation::registerExternalLocation()
*/
RUNNER_TEST(WidgetLocationExternalLocations)
{
    WidgetLocation wl;
    RUNNER_ASSERT(wl.listExternalLocations().size() == 0);
    wl.registerExternalLocation("filepath1");
    wl.registerExternalLocation("filepath2");
    wl.registerExternalLocation("filepath3");
    RUNNER_ASSERT(wl.listExternalLocations().size() == 3);
    wl.registerExternalLocation("filepath1");
    RUNNER_ASSERT(wl.listExternalLocations().size() == 4);
}

/*
Name: WidgetLocationAdvancedInit1
Description: Tests WidgetLocation::WidgetLocation()
*/
RUNNER_TEST(WidgetLocationAdvancedInit1)
{
    WidgetLocation wl("9876543210", "/opt/usr/apps/9876543210", WrtDB::PKG_TYPE_NOMAL_WEB_APP, false, InstallMode::ExtensionType::DIR);
    RUNNER_ASSERT(WrtUtilDirExists(wl.getTemporaryPackageDir()));
    RUNNER_ASSERT(wl.getTemporaryRootDir() == "/opt/usr/apps/9876543210/res/wgt");
}


/*
Name: WidgetLocationAdvancedInit2
Description: Tests WidgetLocation::WidgetLocation()
*/
RUNNER_TEST(WidgetLocationAdvancedInit2)
{
    WidgetLocation wl("1234567890", "/opt/usr/apps/1234567890/", "/tmp/tempdir/", WrtDB::PKG_TYPE_NOMAL_WEB_APP, false, InstallMode::ExtensionType::WGT);
    RUNNER_ASSERT(wl.getTemporaryPackageDir() == "/tmp/tempdir/");
    RUNNER_ASSERT(wl.getTemporaryRootDir() == "/opt/usr/apps/1234567890/res/wgt");
    //fails because there is no use of Jobs::WidgetInstall::createTempPath like it is in constructor
    //from WidgetLocationAdvancedInit1 case
    //RUNNER_ASSERT(WrtUtilDirExists(wl.getTemporaryPackageDir()));
}
