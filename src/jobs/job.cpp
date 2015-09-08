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
#include <job.h>
#include <installer_controller.h>

namespace Jobs {
Job::Job(InstallationType installType) :
    m_handle(0),
    m_installationType(installType),
    m_abortStarted(false),
    m_paused(false)
{}

InstallationType Job::GetInstallationType() const
{
    return m_installationType;
}

bool Job::GetAbortStarted() const
{
    return m_abortStarted;
}

void Job::SetAbortStarted(bool flag)
{
    m_abortStarted = flag;
}

bool Job::IsPaused() const
{
    return m_paused;
}

void Job::SetPaused(bool paused)
{
    if (paused) {
        Pause();
    } else {
        Resume();
    }
}

void Job::Pause()
{
    if (m_paused) {
        return;
    }

    // Pause
    m_paused = true;
}

void Job::Resume()
{
    if (!m_paused) {
        return;
    }

    // Continue
    m_paused = false;

    // Trigger next steps
    CONTROLLER_POST_EVENT(Logic::InstallerController,
                          InstallerControllerEvents::NextStepEvent(this));
}

void Job::SetJobHandle(JobHandle handle)
{
    m_handle = handle;
}

JobHandle Job::GetJobHandle() const
{
    return m_handle;
}

void Job::SendProgress()
{}

void Job::SendFinishedSuccess()
{}

void Job::SendFinishedFailure()
{}

void Job::SendProgressIconPath(const std::string &/*path*/)
{}

void Job::SaveExceptionData(const Jobs::JobExceptionBase&)
{}
} //namespace Jobs
