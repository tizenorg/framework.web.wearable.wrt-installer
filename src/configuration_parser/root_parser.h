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
 * @file        root_parser.h
 * @author      Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @version     0.1
 * @brief
 */
#ifndef _WRT_ENGINE_SRC_INSTALLERCORE_CONFIGURATION_PARSER_ROOT_PARSER_H_
#define _WRT_ENGINE_SRC_INSTALLERCORE_CONFIGURATION_PARSER_ROOT_PARSER_H_

#include <functional>

#include "element_parser.h"

template<typename ta_Parser>
class RootParser : public ElementParser
{
  public:
    typedef typename ta_Parser::Data Data;
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& name)
    {
        if (name == m_tag) {
            return std::bind(&RootParser<ta_Parser>::OnWidgetElement, this);
        } else {
            ThrowMsg(Exception::ParseError,
                     name << " != " << m_tag);
        }
    }

    RootParser(Data data, const DPL::String& tag) :
        m_data(data),
        m_tag(tag)
    {}

    virtual ~RootParser() {}

    virtual void Accept(const Element& /*element*/) { }

    virtual void Accept(const XmlAttribute& /*attribute*/) { }

    virtual void Accept(const Text& /*text*/) { }

    virtual void Verify() { }

  private:

    ElementParserPtr OnWidgetElement()
    {
        typedef ta_Parser Parser;
        return ElementParserPtr(new Parser(this->m_data));
    }

    Data m_data;
    const DPL::String& m_tag;
};

#endif // _WRT_ENGINE_SRC_INSTALLERCORE_CONFIGURATION_PARSER_ROOT_PARSER_H_
