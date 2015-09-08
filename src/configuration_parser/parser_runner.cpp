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
 * @file        parser_runner.cpp
 * @author      Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @version     0.1
 * @brief
 */
#include "parser_runner.h"
#include "root_parser.h"

#include "stdio.h"

#include <stack>
#include <libxml/xmlreader.h>
#include <dpl/binary_queue.h>
#include <dpl/assert.h>
#include <dpl/file_input.h>

#include <installer_log.h>

class ParserRunner::Impl
{
    static void logErrorLibxml2(void *, const char *msg, ...)
    {
        char buffer[300];
        va_list args;
        va_start(args, msg);
        vsnprintf(buffer, 300, msg, args);
        va_end(args);
        _E("%s", buffer);
    }

    static void logWarningLibxml2(void *, const char *msg, ...)
    {
        char buffer[300];
        va_list args;
        va_start(args, msg);
        vsnprintf(buffer, 300, msg, args);
        va_end(args);
        _W("%s", buffer);
    }

  public:
    bool Validate(const std::string& filename, const std::string& schema)
    {
        int ret = -1;
        xmlSchemaParserCtxtPtr ctx;
        xmlSchemaValidCtxtPtr vctx;
        xmlSchemaPtr xschema;
        ctx = xmlSchemaNewParserCtxt(schema.c_str());
        if (ctx == NULL) {
            _E("xmlSchemaNewParserCtxt() Failed");
            return false;
        }
        xschema = xmlSchemaParse(ctx);
        if (xschema == NULL) {
            _E("xmlSchemaParse() Failed");
            return false;
        }
        vctx = xmlSchemaNewValidCtxt(xschema);
        if (vctx == NULL) {
            _E("xmlSchemaNewValidCtxt() Failed");
            return false;
        }
        xmlSchemaSetValidErrors(vctx, (xmlSchemaValidityErrorFunc)&logErrorLibxml2, (xmlSchemaValidityWarningFunc) &logWarningLibxml2, NULL);
        ret = xmlSchemaValidateFile(vctx, filename.c_str(), 0);
        if (ret == -1) {
            _E("xmlSchemaValidateFile() failed");
            return false;
        } else if (ret == 0) {
            _E("Config is Valid");
            return true;
        } else {
            _E("Config Validation Failed with error code %d", ret);
            return false;
        }
        return true;
    }

    void Parse(const std::string& filename,
               const ElementParserPtr& root)
    {
        DPL::FileInput input(filename);
        Parse(&input, root);
    }

    void Parse (DPL::AbstractInput *input,
                const ElementParserPtr& root)
    {
        Try
        {
            m_reader = xmlReaderForIO(&IoRead,
                                      &IoClose,
                                      input,
                                      NULL,
                                      NULL,
                                      XML_PARSE_NOENT);

            if (m_reader == NULL) {
                _E("xmlReaderForIO() is failed");
                ThrowMsg(ElementParser::Exception::ParseError, "xmlReaderForIO() is failed");
            }

            xmlTextReaderSetErrorHandler(m_reader,
                                         &xmlTextReaderErrorHandler,
                                         this);
            xmlTextReaderSetStructuredErrorHandler(
                m_reader,
                &xmlTextReaderStructuredErrorHandler,
                this);
            SetCurrentElementParser(root);

            while (xmlTextReaderRead(m_reader) == 1) {
                switch (xmlTextReaderNodeType(m_reader)) {
                case XML_READER_TYPE_END_ELEMENT:
                    VerifyAndRemoveCurrentElementParser();
                    break;

                case XML_READER_TYPE_ELEMENT:
                {
                    // Elements without closing tag don't receive
                    // XML_READER_TYPE_END_ELEMENT event.
                    if (IsNoClosingTagElementLeft()) {
                        VerifyAndRemoveCurrentElementParser();
                    }

                    DPL::String elementName = GetNameWithoutNamespace();
                    DPL::String nameSpace = GetNamespace();
                    ElementParserPtr parser = GetCurrentElementParser();
                    parser = parser->GetElementParser(nameSpace,
                                                      elementName) ();
                    Assert(!!parser);
                    SetCurrentElementParser(parser);
                    ParseNodeElement(parser);
                    break;
                }
                case XML_READER_TYPE_TEXT:
                case XML_READER_TYPE_CDATA:
                {
                    ParseNodeText(GetCurrentElementParser());
                    break;
                }
                default:
                    _W("Ignoring Node of Type: %d", xmlTextReaderNodeType(m_reader));
                    break;
                }

                if (m_parsingError) {
                    _E("Parsing error occured: %ls", m_errorMsg.c_str());
                    ThrowMsg(ElementParser::Exception::ParseError, m_errorMsg);
                }
            }

            if (m_parsingError) {
                _E("Parsing error occured: %ls", m_errorMsg.c_str());
                ThrowMsg(ElementParser::Exception::ParseError, m_errorMsg);
            }

            while (!m_stack.empty()) {
                VerifyAndRemoveCurrentElementParser();
            }
        }
        Catch(ElementParser::Exception::Base)
        {
            CleanupParserRunner();
            _E("%s", _rethrown_exception.DumpToString().c_str());
            ReThrow(ElementParser::Exception::ParseError);
        }
        CleanupParserRunner();
    }

    Impl() :
        m_reader(NULL),
        m_parsingError(false)
    {}

    ~Impl()
    {
        CleanupParserRunner();
    }

  private:
    typedef std::stack<ElementParserPtr> ElementStack;

  private:
    static void xmlTextReaderErrorHandler(void* arg,
                                          const char* msg,
                                          xmlParserSeverities /* severity */,
                                          xmlTextReaderLocatorPtr /* locator */)
    {
        ParserRunner::Impl* impl = static_cast<ParserRunner::Impl*>(arg);
        impl->ErrorHandler(DPL::FromASCIIString(msg));
    }

    static void xmlTextReaderStructuredErrorHandler(void* arg,
                                                    xmlErrorPtr error)
    {
        ParserRunner::Impl* impl = static_cast<ParserRunner::Impl*>(arg);
        impl->StructuredErrorHandler(error);
    }

    static int XMLCALL IoRead(void *context,
                              char *buffer,
                              int len)
    {
        DPL::AbstractInput *input = static_cast<DPL::AbstractInput *>(context);
        DPL::BinaryQueueAutoPtr data = input->Read(static_cast<size_t>(len));
        if (!data.get()) {
            return -1;
        }
        data->Flatten(buffer, data->Size());
        return static_cast<int>(data->Size());
    }

    static int XMLCALL IoClose(void */* context */)
    {
        // NOOP
        return 0;
    }

  private:
    void SetCurrentElementParser(const ElementParserPtr& elementParser)
    {
        Assert(elementParser);

        m_stack.push(elementParser);
    }

    const ElementParserPtr& GetCurrentElementParser() const
    {
        Assert(!m_stack.empty());

        return m_stack.top();
    }

    void VerifyAndRemoveCurrentElementParser()
    {
        Assert(!m_stack.empty());

        m_stack.top()->Verify();
        m_stack.pop();
    }

    bool IsNoClosingTagElementLeft() const
    {
        Assert(m_reader);

        int depth = xmlTextReaderDepth(m_reader);
        return (static_cast<int>(m_stack.size()) - 2 == depth);
    }

    void ParseNodeElement(const ElementParserPtr& parser)
    {
        Assert(m_reader);

        Element element;
        element.name = GetName();
        element.value = GetValue();
        element.lang = GetLanguageTag();
        element.ns = GetNamespace();

        _D("value: %ls, lang: %ls, ns: %ls)",
            element.value.c_str(), element.lang.c_str(), element.ns.c_str());

        parser->Accept(element);
        ParseNodeElementAttributes(parser);
    }

    void ParseNodeElementAttributes(const ElementParserPtr& parser)
    {
        Assert(m_reader);
        int count = xmlTextReaderAttributeCount(m_reader);
        for (int i = 0; i < count; ++i) {
            xmlTextReaderMoveToAttributeNo(m_reader, i);

            XmlAttribute attribute;
            attribute.ns = GetAttributeNamespace();
            attribute.prefix = GetNamePrefix();
            attribute.name = GetNameWithoutNamespace();
            attribute.value = GetValue();
            attribute.lang = GetLanguageTag();
            _D("Attribute name: %ls, value: %ls, prefix: %ls, namespace: %ls, lang: %ls",
                attribute.name.c_str(), attribute.value.c_str(), attribute.prefix.c_str(),
                attribute.ns.c_str(), attribute.lang.c_str());
            parser->Accept(attribute);
        }
    }

    void ParseNodeText(const ElementParserPtr& parser)
    {
        Text text;
        text.value = GetValue();
        text.lang = GetLanguageTag();
        parser->Accept(text);
    }

    DPL::String GetValue() const
    {
        DPL::String ret_value;
        const xmlChar* value = xmlTextReaderConstValue(m_reader);
        if (value) {
            ret_value =
                DPL::FromUTF8String(reinterpret_cast<const char*>(value));
        }

        return ret_value;
    }

    DPL::String GetAttributeValue(int pos) const
    {
        DPL::String ret_value;
        const xmlChar* value = xmlTextReaderGetAttributeNo(m_reader, pos);
        if (value) {
            ret_value =
                DPL::FromUTF8String(reinterpret_cast<const char*>(value));
        }
        xmlFree(const_cast<xmlChar*>(value));

        return ret_value;
    }

    DPL::String GetAttributeNamespace() const
    {
        DPL::String ret_value;
        const xmlChar* value = xmlTextReaderLookupNamespace(m_reader, NULL);
        if (value) {
            ret_value =
                DPL::FromUTF8String(reinterpret_cast<const char*>(value));
        }
        xmlFree(const_cast<xmlChar*>(value));

        return ret_value;
    }

    DPL::String GetName() const
    {
        DPL::String ret_value;
        const xmlChar* value = xmlTextReaderConstName(m_reader);
        if (value) {
            ret_value =
                DPL::FromUTF8String(reinterpret_cast<const char*>(value));
        }

        return ret_value;
    }

    DPL::String GetNamePrefix() const
    {
        DPL::String ret_value;
        const xmlChar* value = xmlTextReaderPrefix(m_reader);
        if (value) {
            ret_value =
                DPL::FromUTF8String(reinterpret_cast<const char*>(value));
        }

        return ret_value;
    }

    DPL::String GetNameWithoutNamespace() const
    {
        DPL::String ret_value;
        const xmlChar* value = xmlTextReaderLocalName(m_reader);
        if (value) {
            ret_value =
                DPL::FromUTF8String(reinterpret_cast<const char*>(value));
        }

        return ret_value;
    }

    DPL::String GetNamespace() const
    {
        DPL::String ret_value;

        const xmlChar* value = xmlTextReaderConstNamespaceUri(m_reader);
        if (value) {
            ret_value =
                DPL::FromUTF8String(reinterpret_cast<const char*>(value));
        }

        return ret_value;
    }

    DPL::String GetLanguageTag() const
    {
        DPL::String ret_value;
        const xmlChar* value = xmlTextReaderConstXmlLang(m_reader);
        if (value) {
            ret_value =
                DPL::FromUTF8String(reinterpret_cast<const char*>(value));
        }

        return ret_value;
    }

    void ErrorHandler(const DPL::String& msg)
    {
        _E("LibXML:  %ls", msg.c_str());
        m_parsingError = true;
        m_errorMsg = m_errorMsg + DPL::FromASCIIString("\n");
        m_errorMsg = m_errorMsg + msg;
    }

    void StructuredErrorHandler(xmlErrorPtr error)
    {
        _E("LibXML:  %s", error->message);
        m_parsingError = true;
        m_errorMsg = m_errorMsg + DPL::FromASCIIString("\n");
        m_errorMsg = m_errorMsg + DPL::FromUTF8String(error->message);
    }

    void CleanupParserRunner()
    {
        while (!m_stack.empty()) {
            m_stack.pop();
        }
        if (m_reader) {
            xmlFreeTextReader(m_reader);
        }
        m_reader = NULL;
    }

  private:
    xmlTextReaderPtr m_reader;
    ElementStack m_stack;
    bool m_parsingError;
    DPL::String m_errorMsg;
};

ParserRunner::ParserRunner() :
    m_impl(new ParserRunner::Impl())
{}

bool ParserRunner::Validate(const std::string& filename, const std::string& schema)
{
    return m_impl->Validate(filename, schema);
}

void ParserRunner::Parse(const std::string& filename,
                         ElementParserPtr root)
{
    m_impl->Parse(filename, root);
}

void ParserRunner::Parse(DPL::AbstractInput *input,
                         ElementParserPtr root)
{
    m_impl->Parse(input, root);
}

ParserRunner::~ParserRunner()
{
    delete m_impl;
}
