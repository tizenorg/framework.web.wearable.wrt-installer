/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
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
/*
 * @file        widget_install_to_external.cpp
 * @author      Soyoung Kim (sy037.kim@smasung.com)
 */
#include "widget_install_to_external.h"

#include <dpl/singleton_safe_impl.h>
#include <dpl/assert.h>
#include <installer_log.h>

IMPLEMENT_SAFE_SINGLETON(WidgetInstallToExt)

WidgetInstallToExt::WidgetInstallToExt() :
    m_handle(NULL),
    m_appId("")
{}

WidgetInstallToExt::~WidgetInstallToExt()
{}

void WidgetInstallToExt::initialize(std::string appId)
{
    _D("WidgetInstallToExt::initialize()");
    m_appId = appId;

    if (NULL == m_handle) {
        m_handle = app2ext_init(APP2EXT_SD_CARD);
        if (NULL == m_handle) {
            ThrowMsg(Exception::ErrorInstallToExt, "initialize failed");
        }
    }
}

void WidgetInstallToExt::deinitialize()
{
    _D("WidgetInstallToExt::deinitialize()");
    if (NULL != m_handle) {
        if (0 < app2ext_deinit(m_handle)) {
            ThrowMsg(Exception::ErrorInstallToExt,
                     "app2ext deinitialize \
                    failed");
        }
    }
}

void WidgetInstallToExt::preInstallation(GList *dirList, int dSize)
{
    _D("WidgetInstallToExt::preInstallation()");
    Assert(m_handle);

    int ret = m_handle->interface.pre_install(m_appId.c_str(), dirList, dSize);

    if (APP2EXT_SUCCESS == ret) {
        _D("App2Ext pre install success");
    } else {
        postInstallation(false);
        ThrowMsg(Exception::ErrorInstallToExt, "pre-install failed");
    }
}

void WidgetInstallToExt::postInstallation(bool status)
{
    _D("WidgetInstallToExt::postInstallation()");

    if (NULL != m_handle) {
        if (status) {
            m_handle->interface.post_install(m_appId.c_str(),
                                             APP2EXT_STATUS_SUCCESS);
        } else {
            m_handle->interface.post_install(m_appId.c_str(),
                                             APP2EXT_STATUS_FAILED);
        }
    }
}

void WidgetInstallToExt::preUpgrade(GList *dirList, int dSize)
{
    _D("WidgetInstallToExt::preUpgrade()");
    Assert(m_handle);

    int ret = m_handle->interface.pre_upgrade(m_appId.c_str(), dirList, dSize);
    if (APP2EXT_SUCCESS == ret) {
        _D("App2Ext pre-upgrade success");
    } else {
        postUpgrade(false);
        ThrowMsg(Exception::ErrorInstallToExt, "pre-upgrade failed");
    }
}

void WidgetInstallToExt::postUpgrade(bool status)
{
    _D("WidgetInstallToExt::postUpgrade()");
    if (NULL != m_handle) {
        if (status) {
            m_handle->interface.post_upgrade(m_appId.c_str(),
                                             APP2EXT_STATUS_SUCCESS);
        } else {
            m_handle->interface.post_upgrade(m_appId.c_str(),
                                             APP2EXT_STATUS_FAILED);
        }
    }
}

void WidgetInstallToExt::uninstallation()
{
    _D("WidgetInstallToExt::uninstallation()");

    Assert(m_handle);

    int ret = m_handle->interface.pre_uninstall(m_appId.c_str());
    if (APP2EXT_SUCCESS == ret) {
        if (APP2EXT_SUCCESS ==
            m_handle->interface.post_uninstall(m_appId.c_str()))
        {
            _D("App2Ext pre-uninstall success");
        } else {
            ThrowMsg(Exception::ErrorInstallToExt, "post-uninstall failed");
        }
    } else {
        ThrowMsg(Exception::ErrorInstallToExt, "pre-uninstall failed");
    }
}

void WidgetInstallToExt::disable()
{
    _D("WidgetInstallToExt::disable()");
    if (NULL != m_handle) {
        int ret = m_handle->interface.disable(m_appId.c_str());
        if (APP2EXT_SUCCESS != ret && APP2EXT_ERROR_UNMOUNT != ret) {
            ThrowMsg(Exception::ErrorInstallToExt, "disable failed");
        }
    }
}
