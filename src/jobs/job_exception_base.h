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
/**
 * @file    job_exception_base.h
 * @author  Pawel Sikorski (p.sikorski@samgsung.com)
 * @version
 * @brief
 */

#include <dpl/exception.h>

#ifndef SRC_INSTALLER_CORE_JOBS_JOB_EXCEPTION_BASE_H_
#define SRC_INSTALLER_CORE_JOBS_JOB_EXCEPTION_BASE_H_

#define DECLARE_JOB_EXCEPTION_BASE(Base, Class, Param)                       \
    class Class :                                                                    \
        public Base {                                                       \
      public:                                                                  \
        Class(const char *path,                                              \
              const char *function,                                          \
              int line,                                                      \
              const std::string & message = std::string()) :                                                                  \
            Base(path, function, line, message)                              \
        {                                                                    \
            m_className = #Class;                                            \
            m_param = Param;                                                   \
        }                                                                    \
                                                                             \
        Class(const char *path,                                              \
              const char *function,                                          \
              int line,                                                      \
              const Exception &reason,                                       \
              const std::string & message = std::string()) :                                                                  \
            Base(path, function, line, reason, message)                      \
        {                                                                    \
            m_className = #Class;                                            \
            m_param = Param;                                                   \
        }                                                                    \
                                                                             \
        virtual int getParam() const                                         \
        {                                                                    \
            return m_param;                                                  \
        }                                                                    \
      protected:                                                               \
        int m_param;                                                         \
    };

#define DECLARE_JOB_EXCEPTION(Base, Class, Param)                            \
    class Class :                                                                    \
        public Base {                                                       \
      public:                                                                  \
        Class(const char *path,                                              \
              const char *function,                                          \
              int line,                                                      \
              const std::string & message = std::string()) :                                                                  \
            Base(path, function, line, message)                              \
        {                                                                    \
            m_className = #Class;                                            \
            m_param = Param;                                                   \
        }                                                                    \
                                                                             \
        Class(const char *path,                                              \
              const char *function,                                          \
              int line,                                                      \
              const Exception &reason,                                       \
              const std::string & message = std::string()) :                                                                  \
            Base(path, function, line, reason, message)                      \
        {                                                                    \
            m_className = #Class;                                            \
            m_param = Param;                                                   \
        }                                                                    \
                                                                             \
        virtual int getParam() const                                         \
        {                                                                    \
            return m_param;                                                  \
        }                                                                    \
    };

namespace Jobs {
DECLARE_JOB_EXCEPTION_BASE(DPL::Exception, JobExceptionBase, 0)
}

#endif /* SRC_INSTALLER_CORE_JOBS_JOB_EXCEPTION_BASE_H_ */
