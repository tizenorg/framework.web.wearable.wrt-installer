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
 * @file    ParsingAccountTests.cpp
 * @author  Slawomir Pajak (s.pajak@partner.samsung.com)
 * @version 1.0
 * @brief   Account element installation tests
 */

#include <string>
#include <dpl/test/test_runner.h>
#include <InstallerWrapper.h>
#include <ManifestFile.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/wrt-dao-ro/global_config.h>

#include <root_parser.h>
#include <widget_parser.h>
#include <parser_runner.h>

using namespace InstallerWrapper;

////////////////////////////////////////////////////////////////////////////////

RUNNER_TEST_GROUP_INIT(ParsingAccount)

/*
Name: InstallWidgetWithAccount
Description: Tests if widget with account is installed correctly
Expected: widget should be installed correctly and account information should be present in manifest file
*/
RUNNER_TEST(InstallWidgetWithAccount)
{
    std::string tizenId;
    std::string manifestPath = "/opt/share/packages/";
    RUNNER_ASSERT(install(miscWidgetsStuff + "widgets/account.wgt", tizenId) == InstallerWrapper::Success);

    RUNNER_ASSERT(WrtUtilFileExists(manifestPath.append(tizenId.substr(0, 10)).append(".xml")));
    ManifestFile mf(manifestPath);
    WrtDB::WidgetDAOReadOnly dao(DPL::FromASCIIString(tizenId));

    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:account/p:account-provider/@multiple-accounts-support") == "true");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:account/p:account-provider/p:icon[1]/@section") == "account");
    std::string iconPath = DPL::ToUTF8String(*dao.getWidgetInstalledPath()) + WrtDB::GlobalConfig::GetWidgetSharedPath() +
                WrtDB::GlobalConfig::GetWidgetResPath() + "/";
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:account/p:account-provider/p:icon[1]") == iconPath + "icon1.png");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:account/p:account-provider/p:icon[2]/@section") == "account-small");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:account/p:account-provider/p:icon[2]") == iconPath + "icon2.png");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:account/p:account-provider/p:label[1]") == "Name1");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:account/p:account-provider/p:label[2]/@xml:lang") == "en-US");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:account/p:account-provider/p:label[2]") == "Name2");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:account/p:account-provider/p:capability[1]") == "http://tizen.org/account/capability/contact");
    RUNNER_ASSERT(mf.getValueByXpath("/p:manifest/p:account/p:account-provider/p:capability[2]") == "http://tizen.org/account/capability/calendar");
    uninstall(tizenId);
}
