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
#ifndef INSTALLER_MODEL_H
#define INSTALLER_MODEL_H

#include <dpl/mutable_task_list.h>

#include <job_types.h>

namespace Jobs {
class JobExceptionBase;

typedef int JobHandle;

class Job :
    public DPL::MutableTaskList
{
  public:
    Job(InstallationType installType);

    InstallationType GetInstallationType() const;

    // Undo
    void SetAbortStarted(bool flag);
    bool GetAbortStarted() const;

    // Pause/resume support
    bool IsPaused() const;
    void SetPaused(bool paused);
    void Pause();
    void Resume();
    void SetJobHandle(JobHandle handle);
    JobHandle GetJobHandle() const;
    virtual void SendProgress();
    virtual void SendFinishedSuccess();
    virtual void SendFinishedFailure();
    virtual void SendProgressIconPath(const std::string &path);

    virtual void SaveExceptionData(const Jobs::JobExceptionBase&);

  private:
    JobHandle m_handle;
    InstallationType m_installationType;
    bool m_abortStarted;
    bool m_paused;
};
} //namespace Jobs

#endif // INSTALLER_MODEL_H
