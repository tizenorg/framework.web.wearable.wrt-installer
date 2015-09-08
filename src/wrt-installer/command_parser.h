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
 * @file    command_parser.h
 * @author  Soyoung Kim (sy037.kim@samsung.com)
 * @brief   Header file for Command parser
 */

#ifndef WRT_INSTALLER_SRC_WRT_INSTALLER_COMMAND_PARSER_H_
#define WRT_INSTALLER_SRC_WRT_INSTALLER_COMMAND_PARSER_H_

#include <string>
#include <map>
#include <utility>

namespace Command {
const char* const VALUE_INSTALL = "install";
const char* const VALUE_UNINSTALL = "uninstall";
const char* const VALUE_UPGRADE = "upgrade";
const char* const VALUE_UPDATE = "update";
const char* const VALUE_TRUE = "true";
const char* const VALUE_FALSE = "false";
}

class CommandParser
{
  typedef std::map<std::string, std::string> DataMap;

  public:
    struct CscOption {
        std::string path;
        std::string operation;
        bool removable;
    };

    struct FotaOption {
        std::string pkgId;
        std::string operation;
    };

    static bool CscCommandParser(const std::string& arg, CscOption &option);
    static bool FotaCommandParser(const std::string& arg, FotaOption &option);

  private:
    static DataMap ArgumentsParser(const std::string& arg);
};

#endif
