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
 * @file       installer_main_thread.h
 * @author     Krzysztof Jackiewicz (k.jackiewicz@samsung.com)
 * @version    1.0
 */

#ifndef INSTALLER_MAINTHREAD_H_
#define INSTALLER_MAINTHREAD_H_

#include <dpl/singleton.h>

class InstallerMainThread
{
  public:
    void AttachDatabases();
    void DetachDatabases();
    void TouchArchitecture();
    void TouchArchitectureOnlyInstaller();

  private:
    friend class DPL::Singleton<InstallerMainThread>;

    InstallerMainThread();
    virtual ~InstallerMainThread();

    bool m_attached;
};

typedef DPL::Singleton<InstallerMainThread> InstallerMainThreadSingleton;

#endif /* INSTALLER_MAINTHREAD_H_ */
