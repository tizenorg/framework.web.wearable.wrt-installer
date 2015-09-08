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
 * @file    language_subtag_rst_tree.cpp
 * @author  Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @version 1.0
 */
#include <language_subtag_rst_tree.h>
#include <dpl/db/orm.h>
#include <dpl/string.h>
#include <dpl/scope_guard.h>
#include <wrt-commons/i18n-dao-ro/i18n_dao_read_only.h>
#include <wrt-commons/i18n-dao-ro/i18n_database.h>
#include <iterator>
#include <vector>
#include <ctype.h>
#include <dpl/singleton_impl.h>
#include <installer_log.h>

IMPLEMENT_SINGLETON(LanguageSubtagRstTree)

namespace I18nDAOReadOnly = I18n::DB::I18nDAOReadOnly;

bool LanguageSubtagRstTree::ValidateLanguageTag(const std::string &tag_input)
{
    std::string tag = tag_input;
    std::transform(tag.begin(), tag.end(), tag.begin(), &tolower);

    std::vector<DPL::String> parts;
    DPL::Tokenize(DPL::FromUTF8String(tag),
                  '-',
                  std::back_inserter(parts),
                  false);
    std::vector<DPL::String>::iterator token = parts.begin();
    if (token == parts.end())
    {
        return false;
    }

    I18n::DB::Interface::attachDatabaseRO();
    DPL_SCOPE_EXIT()
    {
        I18n::DB::Interface::detachDatabase();
    };

    if (I18nDAOReadOnly::IsValidSubTag(*token, RECORD_TYPE_LANGUAGE))
    {
        ++token;
    }
    else
    {
        return false;
    }

    if (token == parts.end())
    {
        return true;
    }

    if (I18nDAOReadOnly::IsValidSubTag(*token, RECORD_TYPE_EXTLANG))
    {
        ++token;
    }

    if (token == parts.end())
    {
        return true;
    }

    if (I18nDAOReadOnly::IsValidSubTag(*token, RECORD_TYPE_SCRIPT))
    {
        ++token;
    }

    if (token == parts.end())
    {
        return true;
    }

    if (I18nDAOReadOnly::IsValidSubTag(*token, RECORD_TYPE_REGION))
    {
        ++token;
    }

    if (token == parts.end())
    {
        return true;
    }

    while (token != parts.end())
    {
        if (I18nDAOReadOnly::IsValidSubTag(*token, RECORD_TYPE_VARIANT))
        {
            ++token;
        }
        else
        {
            break;
        }
    }

    //'u' - unicode extension - only one BCP47 extension is registered.
    //TODO: unicode extension should be also validated (l.wrzosek)
    if (token == parts.end())
    {
        return true;
    }

    if (*token == L"u")
    {
        ++token;
        bool one_or_more = false;
        while (token != parts.end() &&
               token->size() > 1 &&
               token->size() <= 8)
        {
            one_or_more = true;
            ++token;
        }
        if (!one_or_more)
        {
            return false;
        }
    }

    //'x' - privateuse
    if (token == parts.end())
    {
        return true;
    }

    if (*token == L"x")
    {
        ++token;
        bool one_or_more = false;
        while (token != parts.end() &&
               !token->empty() &&
               token->size() <= 8)
        {
            one_or_more = true;
            ++token;
        }
        if (!one_or_more)
        {
            return false;
        }
    }

    if (token == parts.end())
    {
        return true;
    }

    //Try private use now:
    token = parts.begin();
    if (*token == L"x")
    {
        ++token;
        bool one_or_more = false;
        while (token != parts.end() &&
               !token->empty() &&
               token->size() <= 8)
        {
            one_or_more = true;
            ++token;
        }
        return one_or_more;
    }

    //grandfathered is always rejected
    return false;
}

#define TEST_LANG(str, cond) \
    if (LanguageSubtagRstTreeSingleton::Instance(). \
            ValidateLanguageTag(str) == cond) { \
        _D("Good validate status for lang: %s", str); \
    } else { \
        _E("Wrong validate status for lang: %s, should be %d", str, cond); \
    }

void LanguageSubtagRstTree::Initialize()
{
    /* Temporarily added unit test. Commented out due to performance drop.
     * TEST_LANG("zh", true);
     * TEST_LANG("esx-al", true);
     * TEST_LANG("zh-Hant", true);
     * TEST_LANG("zh-Hant-CN", true);
     * TEST_LANG("zh-Hant-CN-x-private1-private2", true);
     * TEST_LANG("plxxx", false);
     * TEST_LANG("pl-x-private111", false);
     * TEST_LANG("x-private1", false); //do not support pure private ones
     * TEST_LANG("x-private22", false);
     * TEST_LANG("i-private22", false); //do not support i-*
     */
}

#undef TEST_LANG
