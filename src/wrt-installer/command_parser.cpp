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
 * @file    command_parser.cpp
 * @author  Soyoung Kim (sy037.kim@samsung.com)
 * @brief   Implementation file for OptionParser.
 */

#include <dpl/string.h>
#include <dpl/errno_string.h>
#include <installer_log.h>
#include "command_parser.h"

namespace {
typedef std::pair<std::string, std::string> DataPair;

const char* const KEY_OP = "op";
const char* const KEY_PATH = "path";
const char* const KEY_REMOVABLE = "removable";
}

bool CommandParser::CscCommandParser(const std::string& arg, CscOption &option)
{
    using namespace Command;
    // path=/opt/system/csc/Ozq2iEG15R-2.0.0-arm.wgt:op=install:removable=true
    // parsing CSC configuration string
    _D("CscCommandParser");
    if (arg.empty()) {
        return false;
    }
    DataMap cmdMap = ArgumentsParser(arg);

    DataMap::iterator it;
    it = cmdMap.find(KEY_OP);
    if (it == cmdMap.end()) {
        return false;
    }

    if (it->second == VALUE_INSTALL) {
        _D("operation = %s", it->second.c_str());
        option.operation = VALUE_INSTALL;
        it = cmdMap.find(KEY_PATH);
        if (it == cmdMap.end()) {
            return false;
        }
        option.path = it->second;
        _D("path      = %s", option.path.c_str());

        it = cmdMap.find(KEY_REMOVABLE);
        if (it == cmdMap.end()) {
            return false;
        }

        option.removable = true;
        if (0 == it->second.compare(VALUE_FALSE)) {
            option.removable = false;
        }
    } else if (it->second == VALUE_UNINSTALL) {
        _D("operation = %s", it->second.c_str());
        option.operation = VALUE_UNINSTALL;
        // uninstall command isn't confirmed yet
        it = cmdMap.find(KEY_PATH);
        if (it == cmdMap.end()) {
            return false;
        }
        option.path = it->second;
        _D("operation = uninstall");
        _D("path      = %s", option.path.c_str());
    } else {
        _E("Unknown operation : %s", it->second.c_str());
        _D("operation = %s", it->second.c_str());
        return false;
    }

    return true;
}

bool CommandParser::FotaCommandParser(const std::string& arg, FotaOption
        &option)
{
    using namespace Command;
    // path=pkgid:op=install
    _D("FotaCommandParser");
    DataMap cmdMap = ArgumentsParser(arg);

    DataMap::iterator it;
    it = cmdMap.find(KEY_OP);
    if (it == cmdMap.end()) {
        return false;
    }
    option.operation = it->second;

    it = cmdMap.find(KEY_PATH);
    if (it == cmdMap.end()) {
        return false;
    }

    option.pkgId = it->second;
    _D("Fota : package_id [%s], operaion [%s]", option.pkgId.c_str(),
            option.operation.c_str());

    return true;
}

CommandParser::DataMap CommandParser::ArgumentsParser(const std::string& arg)
{
    DataMap result;

    if (arg.empty()) {
        _D("Input argument is empty");
        return result;
    }

    const char* ptr = strtok(const_cast<char*>(arg.c_str()),":");
    while (ptr != NULL) {
        std::string string = ptr;
        ptr = strtok (NULL, ":");
        size_t pos = string.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        result.insert(
                DataPair(string.substr(0, pos),
                    string.substr(pos+1)));
    }

    return result;
}
