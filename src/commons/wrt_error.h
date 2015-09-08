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
 * This file contains the declaration of the error codes of Widget.
 *
 * @file    wrt_error.h
 * @author  MaQuan (jason.ma@samsung.com)
 * @version 0.7
 * @brief   This file contains the declaration of the error codes of Widget.
 */

#ifndef _WRT_ERROR_H_
#define _WRT_ERROR_H_

#ifndef WRT_ERROR_MASKL8
#define WRT_ERROR_MASKL8    0xFF
#endif

#ifndef WRT_SET_IDENT
#define WRT_SET_IDENT(X)    (X & WRT_ERROR_MASKL8)
#endif

#ifndef WRT_ERROR_SET
#define WRT_ERROR_SET(X)    ((X & WRT_ERROR_MASKL8) << 8)
#endif

#define WRT_MID_ERRCODE        0x10000 + WRT_SET_IDENT(5)

/*typedef */ enum
{
    WRT_GENERAL_ERRCODE = WRT_MID_ERRCODE + WRT_SET_IDENT(0),
    WRT_CONFIG_ERRCODE = WRT_MID_ERRCODE + WRT_SET_IDENT(1),
    WRT_DOMAIN_ERRCODE = WRT_MID_ERRCODE + WRT_SET_IDENT(2),
    WRT_JS_EXT_ERRCODE = WRT_MID_ERRCODE + WRT_SET_IDENT(3),
    WRT_WM_ERRCODE = WRT_MID_ERRCODE + WRT_SET_IDENT(4),
    WRT_PLUGIN_ERRCODE = WRT_MID_ERRCODE + WRT_SET_IDENT(5),
    //_ACE support
    WRT_SAI_ERRCODE = WRT_MID_ERRCODE + WRT_SET_IDENT(6)
};

/**
 * WRT error code description
 *
 * @ WRT_SUCCESS
 *    There is no error with WRT operations.
 *
 * @ WRT_ERR_UNKNOW
 *    An unknow error happened to WRT.
 *
 * @ WRT_ERR_INVALID_ARG
 *    Invalid arguments are passed into WRT functions.
 *
 * @ WRT_ERR_OUT_MEMORY
 *    No memory space available for WRT.
 *
 * @ WRT_ERR_NO_DISK_SPACE
 *    There is no disk space for widget applications.
 *
 *
 *
 *
 */
enum WrtError
{
    /* General errors */
    WRT_SUCCESS = WRT_GENERAL_ERRCODE + WRT_ERROR_SET(0x01),
    WRT_ERR_UNKNOWN = WRT_GENERAL_ERRCODE + WRT_ERROR_SET(0x02),
    WRT_ERR_INVALID_ARG = WRT_GENERAL_ERRCODE + WRT_ERROR_SET(0x03),
    WRT_ERR_OUT_OF_MEMORY = WRT_GENERAL_ERRCODE + WRT_ERROR_SET(0x04),
    WRT_ERR_NO_DISK_SPACE = WRT_GENERAL_ERRCODE + WRT_ERROR_SET(0x05),

    /* Configuration */
    WRT_CONF_ERR_GCONF_FAILURE = WRT_CONFIG_ERRCODE + WRT_ERROR_SET(0x01),
    WRT_CONF_ERR_OBJ_MISSING = WRT_CONFIG_ERRCODE + WRT_ERROR_SET(0x02),
    WRT_CONF_ERR_OBJ_EXIST = WRT_CONFIG_ERRCODE + WRT_ERROR_SET(0x03),
    WRT_CONF_ERR_START_FILE_MISSING = WRT_CONFIG_ERRCODE + WRT_ERROR_SET(0x04),
    WRT_CONF_ERR_EMDB_FAILURE = WRT_CONFIG_ERRCODE + WRT_ERROR_SET(0x05),
    WRT_CONF_ERR_EMDB_NO_RECORD = WRT_CONFIG_ERRCODE + WRT_ERROR_SET(0x06),

    /* Domain */
    WRT_DOMAIN_ERR_CREATE_JS_RT = WRT_DOMAIN_ERRCODE + WRT_ERROR_SET(0x01),
    WRT_DOMAIN_ERR_MSG_QUEUE = WRT_DOMAIN_ERRCODE + WRT_ERROR_SET(0x02),

    /* Widget manager*/
    WRT_WM_ERR_NOT_INSTALLED = WRT_WM_ERRCODE + WRT_ERROR_SET(0x01),
    WRT_WM_ERR_HIGH_VER_INSTALLED = WRT_WM_ERRCODE + WRT_ERROR_SET(0x02),
    WRT_WM_ERR_LOW_VER_INSTALLED = WRT_WM_ERRCODE + WRT_ERROR_SET(0x03),
    WRT_WM_ERR_INVALID_ARCHIVE = WRT_WM_ERRCODE + WRT_ERROR_SET(0x04),
    WRT_WM_ERR_INVALID_CERTIFICATION = WRT_WM_ERRCODE + WRT_ERROR_SET(0x05),
    WRT_WM_ERR_NULL_CERTIFICATION = WRT_WM_ERRCODE + WRT_ERROR_SET(0x06),
    WRT_WM_ERR_INSTALLATION_CANCEL = WRT_WM_ERRCODE + WRT_ERROR_SET(0x07),
    WRT_WM_ERR_ALREADY_INSTALLED = WRT_WM_ERRCODE + WRT_ERROR_SET(0x08),
    WRT_WM_ERR_INSTALL_FAILED = WRT_WM_ERRCODE + WRT_ERROR_SET(0x09),
    WRT_WM_ERR_DELETE_BY_SERVER = WRT_WM_ERRCODE + WRT_ERROR_SET(0x0a),
    WRT_WM_ERR_DEINSTALLATION_CANCEL = WRT_WM_ERRCODE + WRT_ERROR_SET(0x0b),
    WRT_WM_ERR_INCORRECT_UPDATE_INFO = WRT_WM_ERRCODE + WRT_ERROR_SET(0x0c),
    WRT_WM_ERR_UNREG_FAILED = WRT_WM_ERRCODE + WRT_ERROR_SET(0x0d),
    WRT_WM_ERR_REMOVE_FILES_FAILED = WRT_WM_ERRCODE + WRT_ERROR_SET(0x0e),
    WRT_WM_ERR_ALREADY_LATEST = WRT_WM_ERRCODE + WRT_ERROR_SET(0x0f),
    WRT_WM_ERR_UPDATE_CANCEL = WRT_WM_ERRCODE + WRT_ERROR_SET(0x10),
    WRT_WM_ERR_INVALID_APP_ID = WRT_WM_ERRCODE + WRT_ERROR_SET(0x11),

    /* Access Control Manager */
    WRT_SAI_ERR_INIT_ACE_FAILED = WRT_SAI_ERRCODE + WRT_ERROR_SET(0x01)
};

namespace CommonError {
enum Type
{
    WrtSuccess,                ///< Success

    HandleNotFound,         ///< Widget handle was not found
    AlreadyRunning,         ///< Widget is already running
    AlreadyStopped,         ///< Widget is already stopped
    InvalidLanguage,        ///< Widget is invalid in current locales
    StillAuthorizing,       ///< Widget is still autorizing and has not yet
                            // finished it
    EarlyKilled,            ///< Widget was early killed during launch
    AccessDenied,           ///< Access denied from ACE
    CertificateRevoked,     ///< Some certificate was revoked.
                            ///  Widget is not allowed to run.

    Unknown                 ///< Temporary error. Try to not use this.
};
}
#endif /* _WRT_ERROR_H_ */

