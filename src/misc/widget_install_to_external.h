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
 * @file        widget_install_to_external.h
 * @author      Soyoung Kim (sy037.kim@smasung.com)
 */
#ifndef WRT_INSTALLER_SRC_MISC_WIDGET_INSTALL_TO_EXTERNAL_H
#define WRT_INSTALLER_SRC_MISC_WIDGET_INSTALL_TO_EXTERNAL_H

#include <string>
#include <dpl/singleton.h>
#include <dpl/string.h>
#include <app2ext_interface.h>

class WidgetInstallToExt
{
  public:
    class Exception
    {
      public:
        DECLARE_EXCEPTION_TYPE(DPL::Exception, Base)
        DECLARE_EXCEPTION_TYPE(Base, ErrorInstallToExt)
    };

    void initialize(std::string appId);
    void deinitialize();
    void preInstallation(GList* dirList, int dSize);
    void postInstallation(bool status);
    void preUpgrade(GList* dirList, int dSize);
    void postUpgrade(bool status);
    void uninstallation();
    void disable();

  private:
    app2ext_handle *m_handle;
    std::string m_appId;

    WidgetInstallToExt();
    ~WidgetInstallToExt();

    friend class DPL::Singleton<WidgetInstallToExt>;
};

typedef DPL::Singleton<WidgetInstallToExt> WidgetInstallToExtSingleton;

#endif // WRT_INSTALLER_SRC_MISC_WIDGET_INSTALL_TO_EXTERNAL_H
