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
 * @brief   Manifest installation test's bodies
 */

#include <string>
#include <dpl/utils/wrt_utility.h>
#include <dpl/test/test_runner.h>
#include <InstallerWrapper.h>
#include <ManifestFile.h>
#include <installer_log.h>
#include <dpl/wrt-dao-ro/global_config.h>

using namespace InstallerWrapper;

////////////////////////////////////////////////////////////////////////////////

RUNNER_TEST_GROUP_INIT(Manifest)

/*
Name: creatingManifestFile
Description: Creation of manifest file by wrt-installer test
Expected: file should be created and installed by wrt-installer. Content should
 match expected values
*/
RUNNER_TEST(creatingManifestFile)
{
    std::string manifestPath = "/opt/share/packages/";
    /* This widget removal should stay here in case previous test run failed
     * (so widget has not been uninstalled) */
    std::string tizenId;

    if(install(miscWidgetsStuff + "widgets/manifest.wgt", tizenId)
            != InstallerWrapper::Success)
    {
        uninstall(tizenId);
        RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/manifest.wgt", tizenId)
                == InstallerWrapper::Success);
    }

    RUNNER_ASSERT(WrtUtilFileExists(manifestPath.append(tizenId.substr(0,10)).append(".xml")));
    ManifestFile mf(manifestPath);

    Try
    {
        _D("Package %s", mf.getValueByXpath("/p:manifest/@package").c_str());
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/@package")
                      == tizenId.substr(0,10));
        _D("type %s", mf.getValueByXpath("/p:manifest/@type").c_str());
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/@type")
                      == "wgt");
        _D("version %s", mf.getValueByXpath("/p:manifest/@version").c_str());
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/@version")
                      == "1.0");
        _D("label %s", mf.getValueByXpath("/p:manifest/p:label").c_str());
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:label")
                      == "Manifest Example");

        _D("email %s", mf.getValueByXpath("/p:manifest/p:author/@email").c_str());
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:author/@email")
                      == "manifest@misc.test.create.desktop.com");
        _D("href %s", mf.getValueByXpath("/p:manifest/p:author/@href").c_str());
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:author/@href")
                      == "http://misc.test.create.desktop.com");
        _D("author %s", mf.getValueByXpath("/p:manifest/p:author").c_str());
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:author")
                      == "Manifest");

        _D("appid %s", mf.getValueByXpath("/p:manifest/p:ui-application/@appid").c_str());
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/@appid")
                      == tizenId);
        _D("type %s", mf.getValueByXpath("/p:manifest/p:ui-application/@type").c_str());
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/@type")
                      == "webapp");
        _D("extraid %s", mf.getValueByXpath("/p:manifest/p:ui-application/@extraid").c_str());
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/@extraid")
                      == "http://test.samsung.com/widget/manifestTest");
        _D("icon %s", mf.getValueByXpath("/p:manifest/p:ui-application/p:icon").c_str());
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/p:icon")
                      == (std::string(WrtDB::GlobalConfig::GetUserInstalledWidgetPath()) + "/" + tizenId.substr(0,10) + WrtDB::GlobalConfig::GetWidgetSharedPath() + WrtDB::GlobalConfig::GetWidgetResPath() + "/" + tizenId + ".png"));

        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/p:label[not(@xml:lang)]")
                      == "Manifest Example");
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/p:label[@xml:lang='de-de']")
                      == "Manifest Beispiel");
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/p:label[@xml:lang='en-us']")
                      == "Manifest Example");
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/p:label[@xml:lang='pl']")
                      == "Przykład Manifest");
        RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:ui-application/p:label[@xml:lang='pt-pt']")
                      == "Exemplo manifesto");
    }
    Catch(ManifestFile::ManifestParseError)
    {
        RUNNER_ASSERT_MSG(false,DPL::Exception::KnownExceptionToString(_rethrown_exception));
    }
    /* If test finished sucessfully than uninstall test widget */
    uninstall(tizenId);
}
