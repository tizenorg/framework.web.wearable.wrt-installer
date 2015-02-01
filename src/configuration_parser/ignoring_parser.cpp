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
 * @file        ignoring_parser.cpp
 * @author      Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @version     0.1
 * @brief
 */
#include "ignoring_parser.h"

#include <functional>
#include <memory>

IgnoringParser::IgnoringParser() : ElementParser()
{}

ElementParserPtr IgnoringParser::Create()
{
    return ElementParserPtr(new IgnoringParser());
}

ElementParserPtr IgnoringParser::Reuse()
{
    return shared_from_this();
}

ElementParser::ActionFunc IgnoringParser::GetElementParser(const DPL::String& /*ns*/,
                                                           const DPL::String& /*name*/)
{
    return std::bind(&IgnoringParser::Reuse, this);
}

void IgnoringParser::Accept(const Element& /*element*/)
{}

void IgnoringParser::Accept(const Text& /*text*/)
{}

void IgnoringParser::Accept(const XmlAttribute& /*attribute*/)
{}

void IgnoringParser::Verify()
{}
