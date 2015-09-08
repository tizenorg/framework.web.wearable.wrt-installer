/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file       installer_log.h
 * @author     Sungsu Kim(sung-su.kim@samsung.com)
 * @version    0.1
 * @brief
 */

#ifndef INSTALLER_LOG_H
#define INSTALLER_LOG_H

#include <unistd.h>
#include <stdio.h>

#include <dpl/log/secure_log.h>

#ifdef WRT_INSTALLER_LOG

#undef COLOR_WARNING
#define COLOR_WARNING "\e[0m"
#undef COLOR_TAG
#define COLOR_TAG "\e[0m"

#endif

// For FOTA debuging
#if 0
#define PKGMGR_FOTA_PATH	 	"/opt/share/packages/.recovery/fota/"
#define FOTA_RESULT_FILE 		PKGMGR_FOTA_PATH"result.txt"

#define _FLOG(prio, fmt, arg...) do { \
        int __fd = 0;\
        FILE* __file = NULL;\
        __file = fopen(FOTA_RESULT_FILE, "a");\
        if (__file == NULL) break;\
        fprintf(__file, "[PKG_FOTA] [wrt-installer] [%s] [%s:%d] "fmt"\n", prio, __FUNCTION__, __LINE__, ##arg); \
        fflush(__file);\
        __fd = fileno(__file);\
        fsync(__fd);\
        fclose(__file);\
} while (0)

#undef _D
#define _D(fmt, arg ...) _FLOG("D", fmt, ##arg)

#undef _W
#define _W(fmt, arg ...) _FLOG("W", fmt, ##arg)

#undef _E
#define _E(fmt, arg ...) _FLOG("E", fmt, ##arg)
#endif


#endif // INSTALLER_LOG_H

