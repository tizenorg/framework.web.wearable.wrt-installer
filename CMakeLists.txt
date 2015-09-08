# Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
# @file        CMakeLists.txt
# @author      Bartlomiej Grzelewski (b.grzelewski@samsung.com)
# @brief
#

############################# Check minimum CMake version #####################

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT("wrt-installer")

############################# cmake packages ##################################

INCLUDE(FindPkgConfig)

############################# build type ######################################

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)

############################# compilation defines #############################

OPTION(DPL_LOG "DPL logs status" ON)
OPTION(WITH_TESTS "Build tests" OFF)
OPTION(MULTIPROCESS_SERVICE_SUPPORT "Process per service" OFF)
OPTION(MULTIPROCESS_SERVICE_SUPPORT_INLINE "Process per service - inline mode support" OFF)
OPTION(SCHEMA_VALIDATION_SUPPORT "Support for XML schema validation" OFF)
OPTION(NFC_APP_CONTROL_EXCEPTION "Enable exception handling for NFC app-control" ON)
OPTION(CORE_HYBRID_PACKAGE "Support for Core Hybrid package installation" ON)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build")
INCLUDE(Options)

############################# compiler flags ##################################

SET(CMAKE_C_FLAGS_PROFILING    "-O2")
SET(CMAKE_CXX_FLAGS_PROFILING  "-O2 -std=c++0x")
SET(CMAKE_C_FLAGS_DEBUG        "-O0 -g")
SET(CMAKE_CXX_FLAGS_DEBUG      "-O0 -std=c++0x -g")
SET(CMAKE_C_FLAGS_RELEASE      "-O2 -g")
SET(CMAKE_CXX_FLAGS_RELEASE    "-O2 -std=c++0x -g -fvisibility-inlines-hidden")
SET(CMAKE_CXX_FLAGS_CCOV       "-O0 -std=c++0x -g --coverage")

IF(DPL_LOG AND NOT CMAKE_BUILD_TYPE MATCHES "profiling")
    MESSAGE(STATUS "Logging enabled for DPL")
    ADD_DEFINITIONS("-DDPL_LOGS_ENABLED")
ELSE(DPL_LOG AND NOT CMAKE_BUILD_TYPE MATCHES "profiling")
    MESSAGE(STATUS "Logging disabled for DPL")
ENDIF(DPL_LOG AND NOT CMAKE_BUILD_TYPE MATCHES "profiling")
MESSAGE(STATUS "WITH_TESTS: " ${WITH_TESTS})
IF(MULTIPROCESS_SERVICE_SUPPORT)
    ADD_DEFINITIONS("-DMULTIPROCESS_SERVICE_SUPPORT")
    IF (MULTIPROCESS_SERVICE_SUPPORT_INLINE)
        ADD_DEFINITIONS("-DMULTIPROCESS_SERVICE_SUPPORT_INLINE")
    ENDIF(MULTIPROCESS_SERVICE_SUPPORT_INLINE)
ENDIF(MULTIPROCESS_SERVICE_SUPPORT)
IF(SCHEMA_VALIDATION_SUPPORT)
    MESSAGE(STATUS "XML Schema validation of installed app enabled")
    ADD_DEFINITIONS("-DSCHEMA_VALIDATION_ENABLED")
ENDIF(SCHEMA_VALIDATION_SUPPORT)
IF(NFC_APP_CONTROL_EXCEPTION)
    MESSAGE(STATUS "Exception handling for NFC app-control is enabled")
    ADD_DEFINITIONS("-DNFC_EXCEPTION_HANDLING_FOR_TIZEN_2_2_ONLY")
ENDIF(NFC_APP_CONTROL_EXCEPTION)
IF(CORE_HYBRID_PACKAGE)
    MESSAGE(STATUS "Support for Core Hybrid package installation is enabled")
    ADD_DEFINITIONS("-DCORE_HYBRID")
ENDIF(CORE_HYBRID_PACKAGE)

IF(TIZEN_VERSION)
    ADD_DEFINITIONS("-DTIZEN_VERSION=\"${TIZEN_VERSION}\"")
ELSE(TIZEN_VERSION)
    message(FATAL_ERROR "TIZEN_VERSION MUST BE declared on build flag.")
ENDIF(TIZEN_VERSION)

# If supported for the target machine, emit position-independent code,suitable
# for dynamic linking and avoiding any limit on the size of the global offset
# table. This option makes a difference on the m68k, PowerPC and SPARC.
# (BJ: our ARM too?)
ADD_DEFINITIONS("-fPIC")

# Set the default ELF image symbol visibility to hidden - all symbols will be
# marked with this unless overridden within the code.
#ADD_DEFINITIONS("-fvisibility=hidden")

# Set compiler warning flags
#ADD_DEFINITIONS("-Werror")             # Make all warnings into errors.
ADD_DEFINITIONS("-Wall")                # Generate all warnings
ADD_DEFINITIONS("-Wextra")              # Generate even more extra warnings
ADD_DEFINITIONS("-Wno-variadic-macros") # Inhibit variadic macros warnings (needed for ORM)
ADD_DEFINITIONS("-Wno-deprecated")      # No warnings about deprecated features
ADD_DEFINITIONS("-std=c++0x")           # accept C++11x standard
ADD_DEFINITIONS("-DWRT_INSTALLER_LOG")  # enable installer log
ADD_DEFINITIONS("-DIME_ENABLED")        # enable Ime features
ADD_DEFINITIONS("-DSERVICE_ENABLED")    # enable Service features

############################# Targets names ###################################

SET(TARGET_INSTALLER_STATIC "wrt-installer_static")
SET(TARGET_INSTALLER "wrt-installer")
SET(TARGET_BACKEND_LIB "wgt")

############################# subdirectories ##################################
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(configuration)

IF(WITH_TESTS)
    ADD_SUBDIRECTORY(tests)
ENDIF(WITH_TESTS)

