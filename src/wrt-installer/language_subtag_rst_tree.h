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
 * @file    language_subtag_rst_tree.h
 * @author  Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @version 1.0
 */
#ifndef LANGUAGE_SUBTAG_RST_TREE_H
#define LANGUAGE_SUBTAG_RST_TREE_H

#include <dpl/singleton.h>
#include <dpl/noncopyable.h>
#include <string>
class LanguageSubtagRstTree : DPL::Noncopyable
{
  public:
    void Initialize();
    bool ValidateLanguageTag(const std::string &tag);
};

typedef DPL::Singleton<LanguageSubtagRstTree> LanguageSubtagRstTreeSingleton;

enum iana_record_types_e
{
    RECORD_TYPE_LANGUAGE,
    RECORD_TYPE_SCRIPT,
    RECORD_TYPE_REGION,
    RECORD_TYPE_VARIANT,
    RECORD_TYPE_GRANDFATHERED,
    RECORD_TYPE_REDUNDANT,
    RECORD_TYPE_EXTLANG
};

#endif  //LANGUAGE_SUBTAG_RST_TREE_H
