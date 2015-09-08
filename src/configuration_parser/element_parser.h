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
 * @file        element_parser.h
 * @author      Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @version     0.1
 * @brief
 */
#ifndef ELEMENT_PARSER_H_
#define ELEMENT_PARSER_H_

#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include <dpl/exception.h>
#include <dpl/string.h>
#include <dpl/shared_ptr.h>
#include <dpl/enable_shared_from_this.h>

struct XmlAttribute
{
    DPL::String prefix;
    DPL::String name;
    DPL::String value;
    DPL::String ns;
    DPL::String lang;
};

struct Element
{
    DPL::String name;
    DPL::String value;
    DPL::String ns;
    DPL::String lang;
};

struct Text
{
    DPL::String value;
    DPL::String ns;
    DPL::String lang;
};

class ElementParser;

typedef std::shared_ptr<ElementParser> ElementParserPtr;

class ElementParser : public std::enable_shared_from_this<ElementParser>
{
  public:
    class Exception
    {
      public:
        DECLARE_EXCEPTION_TYPE(DPL::Exception, Base)
        DECLARE_EXCEPTION_TYPE(Base, ParseError)
    };
    typedef std::function<ElementParserPtr(void)> ActionFunc;
    typedef std::map<DPL::String, ActionFunc> FuncMap;

    virtual void Accept(const Element&) = 0;
    virtual void Accept(const XmlAttribute&) = 0;
    virtual void Accept(const Text&) = 0;
    virtual void Verify() = 0;
    virtual ActionFunc GetElementParser(const DPL::String &ns,
                                        const DPL::String &name) = 0;
    virtual ~ElementParser()
    {}

  protected:
    ElementParser()
    {}
};

#endif // ELEMENT_PARSER_H_
