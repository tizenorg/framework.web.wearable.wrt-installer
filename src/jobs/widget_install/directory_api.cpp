/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file    directory_api.cpp
 * @author  Soyoung Kim(sy037.kim@samsung.com)
 * @version 1.0
 * @brief   directory api - implementation file
 */

#include <cerrno>
#include <directory_api.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fstream>
#include <dpl/foreach.h>
#include <dpl/utils/wrt_utility.h>
#include <dpl/errno_string.h>
#include <installer_log.h>

namespace DirectoryApi {
bool DirectoryCopy(std::string source, std::string dest)
{
    DIR* dir = opendir(source.c_str());
    if (NULL == dir) {
        return false;
    }

    struct dirent dEntry;
    struct dirent *dEntryResult;
    int return_code;

    do {
        struct stat statInfo;
        return_code = readdir_r(dir, &dEntry, &dEntryResult);
        if (dEntryResult != NULL && return_code == 0) {
            std::string fileName = dEntry.d_name;
            std::string fullName = source + "/" + fileName;

            if (stat(fullName.c_str(), &statInfo) != 0) {
                closedir(dir);
                return false;
            }

            if (S_ISDIR(statInfo.st_mode)) {
                if (("." == fileName) || (".." == fileName)) {
                    continue;
                }
                std::string destFolder = dest + "/" + fileName;
                WrtUtilMakeDir(destFolder);

                if (!DirectoryCopy(fullName, destFolder)) {
                    closedir(dir);
                    return false;
                }
            }

            std::string destFile = dest + "/" + fileName;
            std::ifstream infile(fullName);
            std::ofstream outfile(destFile);
            outfile << infile.rdbuf();
            outfile.close();
            infile.close();

            errno = 0;
            if (-1 == chown(destFile.c_str(),
                            statInfo.st_uid,
                            statInfo.st_gid)) {
                int error = errno;
                _D("Failed to change owner [%s]", DPL::GetErrnoString(error).c_str());
            }
        }
    } while (dEntryResult != NULL && return_code == 0);
    closedir(dir);
    return true;
}
}
