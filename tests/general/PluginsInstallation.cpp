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
 * @file    PluginsInstallation.cpp
 * @author  Tomasz Iwanek (t.iwanek@samsung.com)
 * @version 1.0
 * @brief   PluginsInstallation tests implementation
 */

#include <string>
#include <dpl/test/test_runner.h>
#include <dpl/wrt-dao-ro/plugin_dao_read_only.h>
#include <dpl/static_block.h>
#include <installer_log.h>

////////////////////////////////////////////////////////////////////////////////

RUNNER_TEST_GROUP_INIT(PluginsInstallation)

STATIC_BLOCK
{
    (void)system("wrt_reset_all.sh");
    (void)system("wrt-installer -p");
}

#define MAKE_PLUGIN_CHECK_TESTCASE(TESTCASE, LIBNAME)                                                                                               \
    RUNNER_TEST(PluginsInstallation_##TESTCASE)                                                                                                     \
    {                                                                                                                                               \
        Try {                                                                                                                                       \
        WrtDB::PluginDAOReadOnly pdao(#LIBNAME);                                                                                                    \
        RUNNER_ASSERT_MSG(pdao.getInstallationStatus() == WrtDB::PluginDAOReadOnly::INSTALLATION_COMPLETED, "Plugin is not installed correctly");   \
        } Catch(DPL::Exception) {                                                                                                                   \
            _E("%s", _rethrown_exception.DumpToString().c_str());                                                                                   \
            RUNNER_ASSERT_MSG(false, "DPL::Exception");                                                                                             \
        }                                                                                                                                           \
    }                                                                                                                                               \

MAKE_PLUGIN_CHECK_TESTCASE(contact, libwrt-plugins-tizen-contact.so)
MAKE_PLUGIN_CHECK_TESTCASE(systemsetting, libwrt-plugins-tizen-systemsetting.so)
MAKE_PLUGIN_CHECK_TESTCASE(systeminfo, libwrt-plugins-tizen-systeminfo.so)
MAKE_PLUGIN_CHECK_TESTCASE(nfc, libwrt-plugins-tizen-nfc.so)
MAKE_PLUGIN_CHECK_TESTCASE(content, libwrt-plugins-tizen-content.so)
MAKE_PLUGIN_CHECK_TESTCASE(alarm, libwrt-plugins-tizen-alarm.so)
MAKE_PLUGIN_CHECK_TESTCASE(power, libwrt-plugins-tizen-power.so)
MAKE_PLUGIN_CHECK_TESTCASE(secureelement, libwrt-plugins-tizen-secureelement.so)
MAKE_PLUGIN_CHECK_TESTCASE(timeutil, libwrt-plugins-tizen-timeutil.so)
MAKE_PLUGIN_CHECK_TESTCASE(calendar, libwrt-plugins-tizen-calendar.so)
MAKE_PLUGIN_CHECK_TESTCASE(datacontrol, libwrt-plugins-tizen-datacontrol.so)
MAKE_PLUGIN_CHECK_TESTCASE(bookmark, libwrt-plugins-tizen-bookmark.so)
MAKE_PLUGIN_CHECK_TESTCASE(messaging, libwrt-plugins-tizen-messaging.so)
MAKE_PLUGIN_CHECK_TESTCASE(messageport, libwrt-plugins-tizen-messageport.so)
MAKE_PLUGIN_CHECK_TESTCASE(datasync, libwrt-plugins-tizen-datasync.so)
MAKE_PLUGIN_CHECK_TESTCASE(networkbearerselection, libwrt-plugins-tizen-networkbearerselection.so)
MAKE_PLUGIN_CHECK_TESTCASE(package, libwrt-plugins-tizen-package.so)
MAKE_PLUGIN_CHECK_TESTCASE(filesystem, libwrt-plugins-tizen-filesystem.so)
MAKE_PLUGIN_CHECK_TESTCASE(download, libwrt-plugins-tizen-download.so)
MAKE_PLUGIN_CHECK_TESTCASE(application, libwrt-plugins-tizen-application.so)
MAKE_PLUGIN_CHECK_TESTCASE(notification, libwrt-plugins-tizen-notification.so)
MAKE_PLUGIN_CHECK_TESTCASE(push, libwrt-plugins-tizen-push.so)
MAKE_PLUGIN_CHECK_TESTCASE(tizen, libwrt-plugins-tizen-tizen.so)
MAKE_PLUGIN_CHECK_TESTCASE(callhistory, libwrt-plugins-tizen-callhistory.so)
MAKE_PLUGIN_CHECK_TESTCASE(bluetooth, libwrt-plugins-tizen-bluetooth.so)
