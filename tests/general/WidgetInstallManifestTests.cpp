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
 * @file    WidgetInstallManifestTests.cpp
 * @author  Dominik Duda (d.duda@samsung.com)
 * @version 1.0
 * @brief   Tests functions from wrt-installer/src/jobs/widget_install/manifest.cpp
 */
#include <string>
#include <dpl/utils/wrt_utility.h>
#include <dpl/test/test_runner.h>
#include <installer_log.h>
#include <InstallerWrapper.h>
#include <ManifestFile.h>
#include <manifest.h>

#include <iostream>

using namespace Jobs;
using namespace WidgetInstall;

RUNNER_TEST_GROUP_INIT(WidgetInstallManifest)

namespace{
    const std::string manifestFilePath("/tmp/manifest.xml");
    Manifest manifest;
}

/*
Name: wgt_install_manifest_test_01
Description: Tests creation of an empty manifest file.
Expected: The file should be created.
*/
RUNNER_TEST(wgt_install_manifest_test_01)
{
    Manifest manifest;
    manifest.generate(DPL::FromASCIIString(manifestFilePath));

    RUNNER_ASSERT(WrtUtilFileExists(manifestFilePath));
}

/*
Name: wgt_install_manifest_test_02
Description: Tests creation of a manifest file with empty icon, label, author
and description nodes.
Expected: All nodes should be successfully created.
*/
RUNNER_TEST(wgt_install_manifest_test_02)
{
    Manifest manifest;

    manifest.addIcon(IconType());
    manifest.addLabel(LabelType());
    manifest.addAuthor(AuthorType());
    manifest.addDescription(DescriptionType());
    manifest.generate(DPL::FromASCIIString(manifestFilePath));

    ManifestFile mFile(manifestFilePath);

    RUNNER_ASSERT(mFile.getValueByXpath("/p:manifest/p:icon") == "");
    RUNNER_ASSERT(mFile.getValueByXpath("/p:manifest/p:label") == "");
    RUNNER_ASSERT(mFile.getValueByXpath("/p:manifest/p:author") == "");
    RUNNER_ASSERT(mFile.getValueByXpath("/p:manifest/p:description") == "");
}

/*
Name: wgt_install_manifest_test_03
Description: Tests creation of a manifest file with icon, label, author and
description nodes.
Expected: All nodes should be successfully created.
*/
RUNNER_TEST(wgt_install_manifest_test_03)
{
    Manifest manifest;

    manifest.addIcon(IconType(L"manifestIcon.png"));
    manifest.addLabel(LabelType(L"manifest label"));
    manifest.addDescription(DescriptionType(L"manifest description"));
    manifest.addDescription(DescriptionType(L"opis manifestu", L"pl-pl"));
    manifest.addAuthor(AuthorType(
            DPL::String(L"some@email.com"),
            DPL::String(L"emailto:some@email.com"),
            DPL::String(L"en"),
            DPL::String(L"Manifest email")));
    manifest.generate(DPL::FromASCIIString(manifestFilePath));

    ManifestFile mFile(manifestFilePath);

    RUNNER_ASSERT(mFile.getValueByXpath("/p:manifest/p:icon")
            == "manifestIcon.png");
    RUNNER_ASSERT(mFile.getValueByXpath("/p:manifest/p:label")
            == "manifest label");
    RUNNER_ASSERT(mFile.getValueByXpath("/p:manifest/p:author/@email")
            == "some@email.com");
    RUNNER_ASSERT(mFile.getValueByXpath("/p:manifest/p:author/@href")
            == "emailto:some@email.com");
    RUNNER_ASSERT(mFile.getValueByXpath("/p:manifest/p:author/@xml:lang")
            == "en");
    RUNNER_ASSERT(mFile.getValueByXpath("/p:manifest/p:author")
            == "Manifest email");
    RUNNER_ASSERT(mFile.getValueByXpath("/p:manifest/p:description[1]")
            == "manifest description");
    RUNNER_ASSERT(mFile.getValueByXpath("/p:manifest/p:description[2]")
            == "opis manifestu");
    RUNNER_ASSERT(mFile.getValueByXpath("/p:manifest/p:description[2]/@xml:lang")
            == "pl-pl");
}

/*
Name: wgt_install_manifest_test_04
Description: Tests creation of an account node with a capability node as a child
node of the account-provider node.
Expected: The node should be successfully created with all attributes and child
nodes.
*/
RUNNER_TEST(wgt_install_manifest_test_04)
{
    Manifest manifest;
    Account acc;
    AccountProviderType accProv;
    std::pair<DPL::String, DPL::String> icon;

    accProv.appid = L"id.manifest.xml";
    accProv.multiAccount = L"true";

    accProv.name.push_back(LabelType());
    accProv.name.push_back(LabelType(L"only name"));
    accProv.name.push_back(LabelType(L"name with lang", L"en-gb"));
    accProv.name.push_back(LabelType(L"nazwa z lang", L"pl-pl"));

    accProv.capability.push_back(L"Capability_01");
    accProv.capability.push_back(L"");

    icon.first = L"account";
    icon.second = L"/tmp/icon.png";
    accProv.icon.push_back(icon);

    acc.addAccountProvider(accProv);

    manifest.addAccount(acc);
    manifest.generate(DPL::FromASCIIString(manifestFilePath));

    ManifestFile manReader(manifestFilePath);

    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:account/"
            "p:account-provider/@appid") == "id.manifest.xml");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:account/"
            "p:account-provider/@multiple-accounts-support") == "true");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:account/"
            "p:account-provider/p:icon/@section") == "account");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:account/"
            "p:account-provider/p:icon") == "/tmp/icon.png");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:account/"
            "p:account-provider/p:label[1]") == "");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:account/"
            "p:account-provider/p:label[2]") == "only name");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:account/"
            "p:account-provider/p:label[3]") == "name with lang");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:account/"
            "p:account-provider/p:label[4]") == "name with lang");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:account/"
            "p:account-provider/p:label[4]/@xml:lang") == "en-gb");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:account/"
            "p:account-provider/p:label[5]") == "nazwa z lang");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:account/"
            "p:account-provider/p:label[5]/@xml:lang") == "pl-pl");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:account/"
            "p:account-provider/p:capability[1]") == "Capability_01");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:account/"
            "p:account-provider/p:capability[2]") == "");
}

/*
Name: wgt_install_manifest_test_05
Description: Tests creation of a service-application node.
Expected: The node should be successfully created with all attributes and child
nodes.
*/
RUNNER_TEST(wgt_install_manifest_test_05)
{
    Manifest manifest;
    AppControlType appType;
    ServiceApplicationType servApp;

    appType.addMime(L"text/plain");
    appType.addUri(L"http://someurl.org");
    appType.addOperation(L"simple operation");

    servApp.setAppid(L"manifest.id");
    servApp.setAutoRestart(false);
    servApp.setExec(L"exec");
    servApp.setOnBoot(false);
    servApp.setType(L"someType");
    servApp.addLabel(LabelType(L"simpleLabel"));
    servApp.addIcon(IconType(L"simpleIcon.png"));
    servApp.addAppControl(appType);

    manifest.addServiceApplication(servApp);
    manifest.generate(DPL::FromASCIIString(manifestFilePath));

    ManifestFile manReader(manifestFilePath);

    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:service-application/@appid") == "manifest.id");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:service-application/@auto-restart") == "false");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:service-application/@exec") == "exec");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:service-application/@on-boot") == "false");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:service-application/@type") == "someType");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:service-application/p:label") == "simpleLabel");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:service-application/p:icon") == "simpleIcon.png");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:service-application/p:app-control/p:operation/@name") == "simple operation");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:service-application/p:app-control/p:uri/@name") == "http://someurl.org");
    RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:service-application/p:app-control/p:mime/@name") == "text/plain");
}

/*
Name: wgt_install_manifest_test_06
Description: Creats manifest file with an empty ui-application node.
Expected: The empty ui-application node should be created. All node's parameters
should be empty too.
*/
RUNNER_TEST(wgt_install_manifest_test_06)
{
    Manifest manifest;
    UiApplicationType uiApp;

    manifest.addUiApplication(uiApp);
    manifest.generate(DPL::FromASCIIString(manifestFilePath));

    ManifestFile manReader(manifestFilePath);

    Try
    {
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application") == "");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/@appid") == "");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/@exec") == "");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/@type") == "");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/@extraid") == "");
    }
    Catch(ManifestFile::ManifestParseError)
    {
        RUNNER_ASSERT_MSG(false,DPL::Exception::KnownExceptionToString(_rethrown_exception));
    }
}

/*
Name: wgt_install_manifest_test_07
Description: Tests creation of an ui-application node.
Expected: The node should be successfully created with all attributes and child
nodes.
*/
RUNNER_TEST(wgt_install_manifest_test_07)
{
    Manifest manifest;
    UiApplicationType uiApp;

    uiApp.setAppid(L"manifest.AppID");
    uiApp.setCategories(L"categories");
    uiApp.setExtraid(L"extraID");
    uiApp.setExec(L"exec");
    uiApp.setMultiple(false);
    uiApp.setNodisplay(false);
    uiApp.setTaskmanage(true);
    uiApp.setType(L"uiType");

    uiApp.addLabel(LabelType(L"uiLabel"));
    uiApp.addIcon(IconType(L"icon.png"));
    uiApp.addAppCategory(L"appCategory");
    uiApp.addMetadata(MetadataType(DPL::OptionalString(L"key"), DPL::OptionalString(L"value")));

    AppControlType appCtrl;
    appCtrl.addMime(L"text/plain");
    appCtrl.addOperation(L"appOperation");
    appCtrl.addUri(L"some.uri.com");

    uiApp.addAppControl(appCtrl);

    manifest.addUiApplication(uiApp);
    manifest.generate(DPL::FromASCIIString(manifestFilePath));

    ManifestFile manReader(manifestFilePath);

    Try
    {
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/@appid") == "manifest.AppID");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/@exec") == "exec");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/@type") == "uiType");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/@extraid") == "extraID");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/@multiple") == "false");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/@nodisplay") == "false");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/@taskmanage") == "true");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/@categories") == "categories");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/p:label") == "uiLabel");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/p:icon") == "icon.png");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/p:category/@name") == "appCategory");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/p:metadata/@key") == "key");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/p:metadata/@value") == "value");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/p:app-control/p:operation/@name") == "appOperation");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/p:app-control/p:uri/@name") == "some.uri.com");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ui-application/p:app-control/p:mime/@name") == "text/plain");
    }
    Catch(ManifestFile::ManifestParseError)
    {
        RUNNER_ASSERT_MSG(false,DPL::Exception::KnownExceptionToString(_rethrown_exception));
    }
}

/*
Name: wgt_install_manifest_test_08
Description: Tests creation of an ime-application node.
Expected: The node should be successfully created with all attributes and child
nodes.
*/
RUNNER_TEST(wgt_install_manifest_test_08)
{
    Manifest manifest;
    ImeApplicationType ime;

    ime.setAppid(L"appID");
    ime.setExec(L"exec");
    ime.setMultiple(true);
    ime.setNodisplay(true);
    ime.setType(L"type");
    ime.addIcon(IconType(L"imeicon.png"));
    ime.addLabel(LabelType(L"imeLabel"));

    manifest.addImeApplication(ime);
    manifest.generate(DPL::FromASCIIString(manifestFilePath));

    ManifestFile manReader(manifestFilePath);

    Try
    {
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ime-application/@appid") == "appID");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ime-application/@exec") == "exec");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ime-application/@multiple") == "true");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ime-application/@nodisplay") == "true");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ime-application/@type") == "type");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ime-application/p:label") == "imeLabel");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:ime-application/p:icon") == "imeicon.png");
    }
    Catch(ManifestFile::ManifestParseError)
    {
        RUNNER_ASSERT_MSG(false,DPL::Exception::KnownExceptionToString(_rethrown_exception));
    }
}

/*
Name: wgt_install_manifest_test_09
Description: Tests creation of a livebox node.
Expected: The node should be successfully created with all child nodes.
*/
RUNNER_TEST(wgt_install_manifest_test_09)
{
    Manifest manifest;
    LiveBoxInfo lbox;
    BoxInfoType binfo;
    BoxSizeType bst;
    BoxLabelType blt;
    WrtDB::ConfigParserData::LiveboxInfo::BoxSizeInfo bsize;

    lbox.setLiveboxId(L"lboxID");
    lbox.setIcon(L"lboxicon.png");
    lbox.setPrimary(L"lboxprim");


    blt.push_back(std::pair<DPL::String,DPL::String>(L"pl-pl", L"lbl"));
    lbox.setLabel(blt);

    bsize.m_preview = L"true";
    bsize.m_size = L"20;20";
    bsize.m_useDecoration = L"false";
    bst.push_back(bsize);

    binfo.boxMouseEvent = L"onclick";
    binfo.boxSize = bst;
    binfo.boxSrc = L"boxSrc";
    binfo.boxTouchEffect = L"false";
    binfo.pdHeight = L"100";
    binfo.pdSrc = L"pdSrc";
    binfo.pdWidth = L"100";
    lbox.setBox(binfo);

    manifest.addLivebox(lbox);
    manifest.generate(DPL::FromASCIIString(manifestFilePath));

    ManifestFile manReader(manifestFilePath);

    Try
    {
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/@appid") == "lboxID");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/@primary") == "lboxprim");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/@abi") == "html");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/@network") == "true");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/@nodisplay") == "false");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:label[1]") == "lbl");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:label[1]/@xml:lang") == "pl-pl");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:label[2]") == "NO NAME");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:icon") == "lboxicon.png");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:box/@type") == "buffer");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:box/@mouse_event") == "onclick");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:box/@touch_effect") == "false");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:box/p:size") == "20;20");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:box/p:size/@preview") == "true");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:box/p:size/@need_frame") == "false");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:box/p:script/@src") == "boxSrc");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:pd/@type") == "buffer");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:pd/p:size") == "100x100");
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:livebox/p:pd/p:script/@src") == "pdSrc");
    }
    Catch(ManifestFile::ManifestParseError)
    {
        RUNNER_ASSERT_MSG(false,DPL::Exception::KnownExceptionToString(_rethrown_exception));
    }
}

/*
Name: wgt_install_manifest_test_10
Description: Tests creation of a privilege node.
Expected: The node should be successfully created.
*/
RUNNER_TEST(wgt_install_manifest_test_10)
{
    PrivilegeType prv1;

    prv1.addPrivilegeName(L"name_1");

    manifest.addPrivileges(prv1);
    manifest.generate(DPL::FromASCIIString(manifestFilePath));

    ManifestFile manReader(manifestFilePath);

    Try
    {
        RUNNER_ASSERT(manReader.getValueByXpath("/p:manifest/p:privileges/p:privilege") == "name_1");
    }
    Catch(ManifestFile::ManifestParseError)
    {
        RUNNER_ASSERT_MSG(false,DPL::Exception::KnownExceptionToString(_rethrown_exception));
    }
}

/*
Name: wgt_install_manifest_test_11
Description: Deletes the XML file used for tests.
Expected: The file should be successfully deleted.
*/
RUNNER_TEST(wgt_install_manifest_test_11)
{
    RUNNER_ASSERT(WrtUtilRemove(manifestFilePath));
}
