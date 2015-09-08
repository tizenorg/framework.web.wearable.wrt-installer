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
 * @file       installer_main_thread.cpp
 * @author     Krzysztof Jackiewicz (k.jackiewicz@samsung.com)
 * @version    1.0
 */

#include "installer_main_thread.h"
#include <dpl/assert.h>
#include <dpl/wrt-dao-ro/WrtDatabase.h>
#include <vcore/VCore.h>
#include <dpl/singleton_impl.h>
#include <dpl/assert.h>
#include <installer_controller.h>
#include <ace_api_install.h>

IMPLEMENT_SINGLETON(InstallerMainThread)

using namespace WrtDB;

InstallerMainThread::InstallerMainThread() : m_attached(false) {}

InstallerMainThread::~InstallerMainThread()
{
    Assert(!m_attached);
}

void InstallerMainThread::AttachDatabases()
{
    Assert(!m_attached);
    // Attach databases
    ValidationCore::AttachToThreadRW();
    ace_return_t ret = ace_install_initialize();
    Assert(ACE_OK == ret); // to be changed to exception in the future
    WrtDB::WrtDatabase::attachToThreadRW();
    m_attached = true;
}

void InstallerMainThread::DetachDatabases()
{
    Assert(m_attached);
    m_attached = false;
    // Detach databases
    ValidationCore::DetachFromThread();
    ace_return_t ret = ace_install_shutdown();
    Assert(ACE_OK == ret); // to be changed to exception in the future
    WrtDB::WrtDatabase::detachFromThread();
}

void InstallerMainThread::TouchArchitecture()
{
    // Touch controller
    Logic::InstallerControllerSingleton::Instance().Touch();
}

void InstallerMainThread::TouchArchitectureOnlyInstaller()
{
    // Touch controller
    Logic::InstallerControllerSingleton::Instance().Touch();
}
