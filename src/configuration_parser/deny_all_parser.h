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
 * @file        deny_all_parser.h
 * @author      Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @version     0.1
 * @brief
 */
#ifndef DENY_ALL_PARSER_H
#define DENY_ALL_PARSER_H

#include "element_parser.h"

struct DenyAllParser : public ElementParser
{
    static ElementParserPtr Create();
    virtual void Accept(const Element& /*element*/);
    virtual void Accept(const XmlAttribute& /*attribute*/);
    virtual void Accept(const Text& /*text*/);
    virtual void Verify()
    {}
    virtual ActionFunc GetElementParser(const DPL::String& ns,
                                        const DPL::String& name);

    DenyAllParser();
};

#endif // DENY_ALL_PARSER_H
