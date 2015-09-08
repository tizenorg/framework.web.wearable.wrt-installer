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
#include "deny_all_parser.h"
#include <dpl/assert.h>

DenyAllParser::DenyAllParser() : ElementParser()
{}

ElementParserPtr DenyAllParser::Create()
{
    ThrowMsg(Exception::ParseError, "There must not be any subelement");
}

ElementParser::ActionFunc DenyAllParser::GetElementParser(const DPL::String& /*ns*/,
                                                          const DPL::String& /*name*/)
{
    ThrowMsg(Exception::ParseError, "There must not be any subelement");
}

void DenyAllParser::Accept(const Element& /*element*/)
{
    ThrowMsg(Exception::ParseError, "There must not be any element");
}

void DenyAllParser::Accept(const XmlAttribute& /*attribute*/)
{
    ThrowMsg(Exception::ParseError, "There must not be any attribute");
}

void DenyAllParser::Accept(const Text& /*text*/)
{
    ThrowMsg(Exception::ParseError, "There must not be any text element");
}
