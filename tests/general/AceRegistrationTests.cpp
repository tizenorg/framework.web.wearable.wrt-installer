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
 * @file    AceRegistrationTests.cpp
 * @author  Dominik Duda (d.duda@samsung.com)
 * @version 1.0
 * @brief   Tests functions from wrt-installer/src/jobs/widget_install/ace_registration.cpp
 */

#include <string>
#include <dpl/test/test_runner.h>
#include <dpl/wrt-dao-ro/WrtDatabase.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <InstallerWrapper.h>
#include <ace_registration.h>
#include <ace_api_install.h>


using namespace AceApi;
using namespace WrtDB;
using namespace InstallerWrapper;

RUNNER_TEST_GROUP_INIT(AceRegistration)

namespace{
    const std::string wgtPath = miscWidgetsStuff + "widgets/widgetUpdateVer100Signed.wgt";

    std::string tizenId;
    DbWidgetHandle wgtHandle;
}

/*
Name: ace_registration_tests_00
Description: Install a widget which is needed for further tests.
Expected: The widget should be successfully installed.
*/
RUNNER_TEST(ace_registration_tests_00)
{
    RUNNER_ASSERT_MSG(install(wgtPath, tizenId) == InstallerWrapper::Success,
            "Failed to install widget");

    wgtHandle = WidgetDAOReadOnly::getHandle(DPL::FromUTF8String(tizenId));
}

/*
Name: ace_registration_tests_01
Description: Tests registration of the widget in ACE database.
Expected: The widget should be successfully registered.
*/
RUNNER_TEST(ace_registration_tests_01)
{
    bool test1;
    ace_return_t test2, test3, test4;
    WidgetRegisterInfo regInfo;
    WidgetCertificateDataList certificates;
    WidgetCertificateData cert1, cert2, cert3;

    regInfo.webAppType.appType = WrtDB::APP_TYPE_TIZENWEBAPP;
    regInfo.configInfo.widget_id = DPL::FromUTF8String(tizenId);
    regInfo.configInfo.version = DPL::FromUTF8String("1.0");
    regInfo.configInfo.authorName = DPL::FromUTF8String("Author");

    cert1.owner = WidgetCertificateData::Owner::AUTHOR;
    cert1.type = WidgetCertificateData::Type::ROOT;
    cert1.chainId = wgtHandle;
    cert1.strMD5Fingerprint = "";
    cert1.strSHA1Fingerprint = "";
    cert1.strCommonName = DPL::FromUTF8String("");

    cert2.owner = WidgetCertificateData::Owner::DISTRIBUTOR;
    cert2.type = WidgetCertificateData::Type::ENDENTITY;
    cert2.chainId = wgtHandle;
    cert2.strMD5Fingerprint = "";
    cert2.strSHA1Fingerprint = "";
    cert2.strCommonName = DPL::FromUTF8String("");

    cert3.owner = WidgetCertificateData::Owner::UNKNOWN;
    cert3.type = static_cast<WrtDB::WidgetCertificateData::Type>(2);
    cert3.chainId = wgtHandle;
    cert3.strMD5Fingerprint = "";
    cert3.strSHA1Fingerprint = "";
    cert3.strCommonName = DPL::FromUTF8String("");

    certificates.push_back(cert1);
    certificates.push_back(cert2);
    certificates.push_back(cert3);

    test2 = ace_install_initialize();
    wgtHandle = WidgetDAOReadOnly::getHandle(DPL::FromUTF8String(tizenId));

    //Unregister widget in ACE db in order to test registerAceWidget function
    test3 = ace_unregister_widget(
            static_cast<ace_widget_handle_t>(wgtHandle));
    test1 = registerAceWidget(wgtHandle, regInfo, certificates);
    test4 = ace_install_shutdown();

    RUNNER_ASSERT_MSG(test1, "Registering widget from the DB failed!");
    RUNNER_ASSERT_MSG(test2 == ACE_OK, "Cannot initialize ACE database!");
    RUNNER_ASSERT_MSG(test2 == ACE_OK, "Cannot unregister widget in ACE database!");
    RUNNER_ASSERT_MSG(test2 == ACE_OK, "Shutting down ACE database failed!");
}

/*
Name: ace_registration_tests_02
Description: Tests registration of the widget which data already are in
the database.
Expected: The widget should be successfully registered.
*/
RUNNER_TEST(ace_registration_tests_02)
{
    bool test1;
    ace_return_t test2, test3, test4;

    test2 = ace_install_initialize();

    //Unregister widget in ACE db in order to test registerAceWidgetFromDB
    //function
    test3 = ace_unregister_widget(
            static_cast<ace_widget_handle_t>(wgtHandle));
    test1 = registerAceWidgetFromDB(wgtHandle);
    test4 = ace_install_shutdown();

    RUNNER_ASSERT_MSG(test1, "Registering widget from the DB failed!");
    RUNNER_ASSERT_MSG(test2 == ACE_OK, "Cannot initialize ACE database!");
    RUNNER_ASSERT_MSG(test2 == ACE_OK, "Cannot unregister widget in ACE database!");
    RUNNER_ASSERT_MSG(test2 == ACE_OK, "Shutting down ACE database failed!");
}

/*
Name: ace_registration_tests_03
Description: Uninstalls the widget previously installed for tests.
Expected: The widget should be successfully uninstalled.
*/
RUNNER_TEST(ace_registration_tests_03)
{
    RUNNER_ASSERT_MSG(uninstall(tizenId), "Failed to uninstall widget!");
}
