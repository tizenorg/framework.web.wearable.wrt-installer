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
#ifndef SRC_INSTALLER_CORE_JOBS_JOB_BASE_H
#define SRC_INSTALLER_CORE_JOBS_JOB_BASE_H

#include <string>

typedef std::string ProgressDescription;
typedef float ProgressPercent;

namespace Jobs {
template<typename T_InstallationStep,
         T_InstallationStep lastElement>
class JobProgressBase
{
  protected:
    bool m_progressFlag;
    ProgressDescription m_progresDescription;
    ProgressPercent m_progresPercent;

  public:
    JobProgressBase() : m_progressFlag(false),
        m_progresPercent(0.0)
    {}

    void SetProgressFlag(bool flag)
    {
        m_progressFlag = flag;
    }
    bool GetProgressFlag() const
    {
        return m_progressFlag;
    }

    ProgressDescription GetProgressDescription() const
    {
        return m_progresDescription;
    }

    ProgressPercent GetProgressPercent() const
    {
        return m_progresPercent;
    }

    void UpdateProgress(T_InstallationStep step,
                        ProgressDescription const &description)
    {
        m_progresPercent =
            ((static_cast<ProgressPercent>(step)) /
             static_cast<ProgressPercent>(lastElement)) * 100;
        m_progresDescription = description;
    }
};

template<class T_JobStruct>
class JobContextBase
{
  public:
    JobContextBase(const T_JobStruct& jobStruct) :
        m_jobStruct(jobStruct)
    {}

    T_JobStruct GetInstallerStruct() const
    {
        return m_jobStruct;
    }

  protected:
    T_JobStruct m_jobStruct;
};

template<typename T_finishedCb, typename T_progressCb>
struct JobCallbacksBase
{
    T_finishedCb finishedCallback;
    T_progressCb progressCallback;
    void *userParam;

    // It must be empty-constructible as a parameter of generic event
    JobCallbacksBase() :
        finishedCallback(0),
        progressCallback(0),
        userParam(0)
    {}

    JobCallbacksBase(T_finishedCb finished,
                     T_progressCb progress,
                     void *param) :
        finishedCallback(finished),
        progressCallback(progress),
        userParam(param)
    {}
};
} //namespace Jobs

#endif // SRC_INSTALLER_CORE_JOBS_JOB_BASE_H
