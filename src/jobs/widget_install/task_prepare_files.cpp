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
 * @file       task_generate_config.cpp
 * @author     Krzysztof Jackiewicz (k.jackiewicz@samsung.com)
 * @version    1.0
 */

#include "task_prepare_files.h"
#include <memory>
#include <string>
#include <iostream>
#include <dpl/file_output.h>
#include <dpl/file_input.h>
#include <dpl/copy.h>
#include <dpl/wrt-dao-ro/feature_dao_read_only.h>
#include <dpl/foreach.h>
#include <widget_install/widget_install_context.h>
#include <widget_install_errors.h>
#include <task_commons.h>
#include <installer_log.h>

namespace Jobs {
namespace WidgetInstall {
TaskPrepareFiles::TaskPrepareFiles(InstallerContext &installerContext) :
    DPL::TaskDecl<TaskPrepareFiles>(this),
    m_installerContext(installerContext)
{
    AddStep(&TaskPrepareFiles::StartStep);
    AddStep(&TaskPrepareFiles::StepCopyFiles);
    AddStep(&TaskPrepareFiles::EndStep);
}

void TaskPrepareFiles::CopyFile(const std::string& source)
{
    if (source.empty()) {
        _W("No source file specified");
        return;
    }

    std::string filename = source;
    size_t last = source.find_last_of("\\/");
    if (last != std::string::npos) {
        filename = source.substr(last + 1);
    }
    std::string target =
        m_installerContext.locations->getSourceDir() + '/' +
        filename;
    _D("source %s", source.c_str());
    _D("target %s", target.c_str());

    Try
    {
        DPL::FileInput input(source);
        DPL::FileOutput output(target);
        DPL::Copy(&input, &output);
    }
    Catch(DPL::FileInput::Exception::Base)
    {
        _E("File input error");
        // Error while opening or closing source file
        ReThrowMsg(Exceptions::CopyIconFailed, source);
    }
    Catch(DPL::FileOutput::Exception::Base)
    {
        _E("File output error");
        // Error while opening or closing target file
        ReThrowMsg(Exceptions::CopyIconFailed, target);
    }
    Catch(DPL::CopyFailed)
    {
        _E("File copy error");
        // Error while copying
        ReThrowMsg(Exceptions::CopyIconFailed, target);
    }
}

void TaskPrepareFiles::StepCopyFiles()
{
    CopyFile(m_installerContext.locations->getWidgetSource());

    size_t last = m_installerContext.locations->getWidgetSource().find_last_of(
            "\\/");
    std::string sourceDir = "";
    if (last != std::string::npos) {
        sourceDir = m_installerContext.locations->getWidgetSource().substr(
                0,
                last
                + 1);
    }

    _D("Icons copy...");
    FOREACH(it, m_installerContext.widgetConfig.configInfo.iconsList) {
        std::ostringstream os;
        _D("Coping: %s%ls", sourceDir.c_str(), (it->src).c_str());
        os << sourceDir << DPL::ToUTF8String(it->src);
        CopyFile(os.str());
    }
}

void TaskPrepareFiles::StartStep()
{
    LOGI("--------- <TaskPrepareFiles> : START ----------");
}

void TaskPrepareFiles::EndStep()
{
    LOGI("--------- <TaskPrepareFiles> : END ----------");
}
} // namespace WidgetInstall
} // namespace Jobs
