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
 * @file    directory_api.h
 * @author  Soyoung Kim(sy037.kim@samsung.com)
 * @version 1.0
 * @brief   directory api - header file
 */
#ifndef WRT_SRC_INSTALLER_CORE_DIRECTORY_API_H_
#define WRT_SRC_INSTALLER_CORE_DIRECTORY_API_H_

#include<string>

namespace DirectoryApi {
bool DirectoryCopy(std::string source, std::string dest);
}

#endif  /* WRT_SRC_INSTALLER_CORE_DIRECTORY_API_H_ */

