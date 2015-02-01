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
 * This file  have been implemented in compliance with  W3C WARP SPEC.
 * but there are some patent issue between  W3C WARP SPEC and APPLE.
 * so if you want to use this file, refer to the README file in root directory
 */
/**
 * @file        widget_parser.cpp
 * @author      Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @version     0.1
 * @brief
 */

#include <widget_parser.h>
#include "ignoring_parser.h"
#include "deny_all_parser.h"
#include <dpl/wrt-dao-ro/config_parser_data.h>
#include "libiriwrapper.h"
#include "wrt-commons/i18n-dao-ro/i18n_dao_read_only.h"
#include <dpl/utils/warp_iri.h>
#include <dpl/utils/mime_type_utils.h>
#include <language_subtag_rst_tree.h>
#include <algorithm>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <functional>
#include <locale>
#include <memory>
#include <string>
#include <boost/optional.hpp>

#include <dpl/foreach.h>
#include <dpl/platform.h>
#include <dpl/utils/warp_iri.h>
#include <dpl/utils/mime_type_utils.h>
#include <dpl/wrt-dao-ro/config_parser_data.h>
#include <iri.h>
#include <pcrecpp.h>

#include <deny_all_parser.h>
#include <ignoring_parser.h>
#include <installer_log.h>
#include <language_subtag_rst_tree.h>
#include <libiriwrapper.h>

using namespace WrtDB;

#ifdef ELEMENT_ATTR_MAX_LENGTH
namespace {
const unsigned int MAX_ATTR_ELEMENT_LENGTH = 2048;
const unsigned int MAX_NAME_KEY_LENGTH = 80;
const unsigned int MAX_NAME_KEY_VALUE_LENGTH = 8192;
} // namespace anonymous
#endif // ELEMENT_ATTR_MAX_LENGTH

namespace{
const unsigned int MAX_APPLICATION_ID_LENGTH = 63;
}

namespace Unicode {
static const DPL::String UTF_LRE = L"\x0202a";
static const DPL::String UTF_LRO = L"\x0202d";
static const DPL::String UTF_RLE = L"\x0202b";
static const DPL::String UTF_RLO = L"\x0202e";
static const DPL::String UTF_PDF = L"\x0202c";

Direction ParseDirAttribute(const XmlAttribute& attribute)
{
    Assert(L"dir" == attribute.name);
    if (L"ltr" == attribute.value) {
        return LRE;
    } else if (L"rtl" == attribute.value) {
        return RLE;
    } else if (L"lro" == attribute.value) {
        return LRO;
    } else if (L"rlo" == attribute.value) {
        return RLO;
    } else {
        _W("dir attribute has wrong value: %ls ", attribute.value.c_str());
        return EMPTY;
    }
}

void UpdateTextWithDirectionMark(Direction direction,
                                 DPL::String* text)
{
    Assert(text);
    switch (direction) {
    case RLO:
        *text = UTF_RLO + *text + UTF_PDF;
        break;
    case RLE:
        *text = UTF_RLE + *text + UTF_PDF;
        break;
    case LRE:
        *text = UTF_LRE + *text + UTF_PDF;
        break;
    case LRO:
        *text = UTF_LRO + *text + UTF_PDF;
        break;
    case EMPTY:
        break;
    default:
        Assert(false);
        break;
    }
}
} // namespace Unicode

#ifdef ELEMENT_ATTR_MAX_LENGTH
void NormalizeString(DPL::String& str, const unsigned int length, bool showEllipsis)
{
    bool hasExceededMaxLength = false;
    if (str.size() > length) {
        hasExceededMaxLength = true;
        str.resize(length);
    }

    DPL::Optional<DPL::String> opt = str;
    NormalizeString(opt);
    str = *opt;
#ifdef ADD_ELLIPSIS
    if (showEllipsis && hasExceededMaxLength && (str.size() == length)) {
        str = DPL::FromUTF8String(DPL::ToUTF8String(str).append("..."));
    }
#endif
}

void NormalizeString(DPL::Optional<DPL::String>& str, const unsigned int length, bool showEllipsis)
{
    bool hasExceededMaxLength = false;
    if (!!str) {
        if ((*str).size() > length) {
            hasExceededMaxLength = true;
            (*str).resize(length);
        }

        NormalizeString(str);
#ifdef ADD_ELLIPSIS
        if (showEllipsis && hasExceededMaxLength && ((*str).size() == length)) {
            str = DPL::FromUTF8String(DPL::ToUTF8String(*str).append("..."));
        }
#endif
    }
}

void NormalizeAndTrimSpaceString(DPL::OptionalString& str, const unsigned int length)
{
    if (!!str) {
        if ((*str).size() > length) {
            (*str).resize(length);
        }
        NormalizeString(str, true);
    }
}
#endif //ELEMENT_ATTR_MAX_LENGTH

class InnerElementsParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return std::bind(&InnerElementsParser::Other, this);
    }

    virtual void Accept(const Element& /*element*/)
    {}

    virtual void Accept(const Text& text)
    {
        if (!m_text) {
            m_text = text;
        } else {
            m_text->value += text.value;
        }
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (attribute.name == L"dir") {
            m_textDirection = Unicode::ParseDirAttribute(attribute);
        }
    }

    virtual void Verify()
    {
        if (!!m_text) {
            Unicode::UpdateTextWithDirectionMark(m_textDirection,
                                                 &m_text->value);
            m_parentParser->Accept(*m_text);
        }
    }

    InnerElementsParser(ElementParserPtr parent) :
        m_parentParser(parent),
        m_textDirection(Unicode::EMPTY)
    {}

    ElementParserPtr Other()
    {
        return ElementParserPtr(new InnerElementsParser(
                                    std::static_pointer_cast<ElementParser>(
                                        shared_from_this())));
    }

  private:
    boost::optional<Text> m_text;
    ElementParserPtr m_parentParser;
    Unicode::Direction m_textDirection;
};

class NameParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return std::bind(&NameParser::Other, this);
    }

    virtual void Accept(const Element& element)
    {
        m_lang = element.lang;
        m_name = L"";
    }

    virtual void Accept(const Text& text)
    {
        if (!m_name) {
            m_name = text.value;
        } else {
            *m_name += text.value;
        }
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (attribute.name == L"short") {
            if (!m_shortName) {
                m_shortName = attribute.value;
            }
        } else if (attribute.name == L"dir") {
            m_textDirection = Unicode::ParseDirAttribute(attribute);
        }
    }

    virtual void Verify()
    {
        ConfigParserData::LocalizedData& data = m_data.localizedDataSet[m_lang];
        if (!data.name) {
            if (!!m_name) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
                NormalizeString(m_name, MAX_ATTR_ELEMENT_LENGTH, true);
#else
                NormalizeString(m_name);
#endif
                Unicode::UpdateTextWithDirectionMark(m_textDirection, &*m_name);
            }
            data.name = m_name;
            if (!!m_shortName) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
                NormalizeString(m_shortName, MAX_ATTR_ELEMENT_LENGTH, true);
#else
                NormalizeString(m_shortName);
#endif
                Unicode::UpdateTextWithDirectionMark(m_textDirection,
                                                     &*m_shortName);
                data.shortName = m_shortName;
            }
        }
    }

    NameParser(Unicode::Direction direction,
               ConfigParserData& data) :
        m_data(data),
        m_textDirection(direction)
    {}

    ElementParserPtr Other()
    {
        return ElementParserPtr(new InnerElementsParser(
                                    std::static_pointer_cast<ElementParser>(
                                        shared_from_this())));
    }

  private:
    ConfigParserData& m_data;
    DPL::OptionalString m_name;
    DPL::OptionalString m_shortName;
    DPL::OptionalString m_dir;
    DPL::String m_lang;
    Unicode::Direction m_textDirection;
};

class AccessParser : public ElementParser
{
  public:
    enum StandardType
    {
        STANDARD_TYPE_NONE,
        STANDARD_TYPE_WARP
    };

    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return std::bind(&AccessParser::Other, this);
    }

    virtual void Accept(const Element& element)
    {
        // for tizen web apps WARP should be used
        if (element.ns == ConfigurationNamespace::W3CWidgetNamespaceName ||
            element.ns == ConfigurationNamespace::TizenWebAppNamespaceName)
        {
            m_standardType = STANDARD_TYPE_WARP;
        }
    }

    virtual void Accept(const Text& /*text*/)
    {}

    void AcceptWac(const XmlAttribute& attribute)
    {
        if (attribute.name == L"origin") {
            m_strIRIOrigin = attribute.value;
        } else if (attribute.name == L"subdomains") {
            DPL::String normalizedValue = attribute.value;
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
            NormalizeString(normalizedValue, MAX_ATTR_ELEMENT_LENGTH);
#else
            NormalizeString(normalizedValue);
#endif
            if (normalizedValue == L"true") {
                m_bSubDomainAccess = true;
            } else {
                m_bSubDomainAccess = false;
            }
        }
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        switch (m_standardType) {
        case STANDARD_TYPE_WARP:
            AcceptWac(attribute);
            break;
        default:
            _E("Error in Access tag - unknown standard.");
            break;
        }
    }

    void VerifyWac()
    {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
        NormalizeString(m_strIRIOrigin, MAX_ATTR_ELEMENT_LENGTH);
#else
        NormalizeString(m_strIRIOrigin);
#endif

        WarpIRI iri;
        iri.set(m_strIRIOrigin, false);

        if (!iri.isAccessDefinition()) {
            _W("Access list element: %ls is not acceptable by WARP standard and will be ignored!",
                m_strIRIOrigin.c_str());
            return;
        }

        if(m_strIRIOrigin == L"*") //wildcard match means yes for subdomains
        {
            m_bSubDomainAccess = true;
        }

        ConfigParserData::AccessInfo accessInfo(m_strIRIOrigin,
                                                m_bSubDomainAccess);
        //std::pair <ConfigParserData::AccessInfoSet::iterator, bool> ret =
        m_data.accessInfoSet.insert(accessInfo);
    }

    virtual void Verify()
    {
        switch (m_standardType) {
        case STANDARD_TYPE_WARP:
            VerifyWac();
            break;
        default:
            _E("Error in Access tag - unknown standard.");
            break;
        }
    }

    AccessParser(ConfigParserData& data) :
        ElementParser(),
        m_bSubDomainAccess(false),
        m_standardType(STANDARD_TYPE_NONE),
        m_network(false),
        m_data(data)
    {}

    ElementParserPtr Other()
    {
        return ElementParserPtr(new InnerElementsParser(
                                    ElementParserPtr(shared_from_this())));
    }

  private:
    DPL::String m_strIRIOrigin;
    bool m_bSubDomainAccess;
    StandardType m_standardType;
    bool m_network;
    ConfigParserData& m_data;
};

class DescriptionParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return std::bind(&DescriptionParser::Other, this);
    }

    virtual void Accept(const Element& element)
    {
        m_lang = element.lang;
        m_description = L"";
    }

    ElementParserPtr Other()
    {
        return ElementParserPtr(new InnerElementsParser(
                                    std::static_pointer_cast<ElementParser>(
                                        shared_from_this())));
    }

    virtual void Accept(const Text& text)
    {
        if (!m_description) {
            m_description = text.value;
        } else {
            *m_description += text.value;
        }
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (attribute.name == L"dir") {
            m_textDirection = Unicode::ParseDirAttribute(attribute);
        }
    }

    virtual void Verify()
    {
        ConfigParserData::LocalizedData& data = m_data.localizedDataSet[m_lang];
        if (!data.description) {
            if (!!m_description) {
                Unicode::UpdateTextWithDirectionMark(m_textDirection,
                                                     &*m_description);
            }
            data.description = m_description;
        }
    }

    DescriptionParser(Unicode::Direction direction,
                      ConfigParserData& data) :
        m_data(data),
        m_lang(),
        m_description(),
        m_textDirection(direction)
    {}

  private:
    ConfigParserData& m_data;
    DPL::String m_lang;
    DPL::OptionalString m_description;
    Unicode::Direction m_textDirection;
};

class AuthorParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return std::bind(&AuthorParser::Other, this);
    }

    AuthorParser(Unicode::Direction direction,
                 ConfigParserData& data) :
        m_data(data),
        m_textDirection(direction)
    {}

    virtual void Accept(const Element& /*element*/)
    {
        m_authorName = L"";
    }

    virtual void Accept(const Text& text)
    {
        *(m_authorName) += text.value;
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (attribute.name == L"href") {
            //Validate href IRI and ignore it if invalid
            //See also test: ta-argMozRiC-an
            LibIri::Wrapper iri(DPL::ToUTF8String(attribute.value).c_str());
            if (iri.Validate()) {
                m_authorHref = attribute.value;
            }
        } else if (attribute.name == L"email") {
            m_authorEmail = attribute.value;
        } else if (attribute.name == L"dir") {
            m_textDirection = Unicode::ParseDirAttribute(attribute);
        }
    }

    virtual void Verify()
    {
        if (!m_data.authorName && !m_data.authorHref && !m_data.authorEmail) {
            if (!!m_authorName) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
                NormalizeString(m_authorName, MAX_ATTR_ELEMENT_LENGTH, true);
#else
                NormalizeString(m_authorName);
#endif
                Unicode::UpdateTextWithDirectionMark(m_textDirection,
                                                     &*m_authorName);
                m_data.authorName = m_authorName;
            }
            if (!!m_authorHref) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
                NormalizeString(m_authorHref, MAX_ATTR_ELEMENT_LENGTH, true);
#else
                NormalizeString(m_authorHref);
#endif
                m_data.authorHref = m_authorHref;
            }
            if (!!m_authorEmail) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
                NormalizeString(m_authorEmail, MAX_ATTR_ELEMENT_LENGTH, true);
#else
                NormalizeString(m_authorEmail);
#endif
                m_data.authorEmail = m_authorEmail;
            }
        }
    }

    ElementParserPtr Other()
    {
        return ElementParserPtr(new InnerElementsParser(
                                    std::static_pointer_cast<ElementParser>(
                                        shared_from_this())));
    }

  private:
    ConfigParserData& m_data;
    DPL::OptionalString m_authorEmail;
    DPL::OptionalString m_authorHref;
    DPL::OptionalString m_authorName;
    Unicode::Direction m_textDirection;
};

class LicenseParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return std::bind(&LicenseParser::Other, this);
    }

    LicenseParser(Unicode::Direction direction,
                  ConfigParserData& data) :
        m_data(data),
        m_ignore(true),
        m_textDirection(direction)
    {}

    virtual void Accept(const Element& element)
    {
        if (!m_license) {
            m_lang = element.lang;
            m_license = L"";
            m_ignore = false;
        }
    }

    virtual void Accept(const Text& text)
    {
        if (!m_ignore) {
            *m_license += text.value;
        }
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (!m_ignore) {
            if (attribute.name == L"href" && !m_licenseHref) {
                m_licenseHref = attribute.value;
            } else if (attribute.name == L"file" && !m_licenseFile) {
                m_licenseFile = attribute.value;
            } else if (attribute.name == L"dir") {
                m_textDirection = Unicode::ParseDirAttribute(attribute);
            }
        }
    }

    virtual void Verify()
    {
        ConfigParserData::LocalizedData& data = m_data.localizedDataSet[m_lang];
        if (!data.license) {
            if (!!m_license) {
                Unicode::UpdateTextWithDirectionMark(m_textDirection,
                                                     &*m_license);
            }
            data.license = m_license;
            data.licenseHref = m_licenseHref;
            data.licenseFile = m_licenseFile;
        }
    }

    ElementParserPtr Other()
    {
        return ElementParserPtr(new InnerElementsParser(
                                    ElementParserPtr(shared_from_this())));
    }

  private:
    ConfigParserData& m_data;
    DPL::String m_lang;
    bool m_ignore;

    DPL::OptionalString m_license;
    DPL::OptionalString m_licenseFile;
    DPL::OptionalString m_licenseHref;
    Unicode::Direction m_textDirection;
};

class IconParser : public ElementParser
{
    DECLARE_EXCEPTION_TYPE(ElementParser::Exception::ParseError, BadSrcError)

  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }

    IconParser(ConfigParserData& data) : ElementParser(),
        m_data(data), m_isSmall(false)
    {}

    IconParser(ConfigParserData& data, bool isSmall) : ElementParser(),
        m_data(data), m_isSmall(isSmall)
    {}

    virtual void Accept(const Element& /*element*/) { }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (attribute.name == L"src") {
            if (attribute.value.size() > 0) {
                m_src = attribute.value;
            }
        } else if (attribute.name == L"width") {
            m_width = ParseSizeAttributeValue(attribute.value);
        } else if (attribute.name == L"height") {
            m_height = ParseSizeAttributeValue(attribute.value);
        }
    }

    virtual void Accept(const Text& /*text*/)
    {
        ThrowMsg(Exception::ParseError, "Icon element must be empty");
    }

    virtual void Verify()
    {
        if (!m_src) {
            _W("src attribute of icon element is mandatory - ignoring");
            return;
        }

        Try
        {
            ConfigParserData::Icon icon(delocalizeSrcPath(*m_src));
            icon.width = m_width;
            icon.height = m_height;
            icon.isSmall = m_isSmall;

            ConfigParserData::IconsList::iterator it = std::find(
                    m_data.iconsList.begin(), m_data.iconsList.end(), icon);
            if (it == m_data.iconsList.end()) {
                m_data.iconsList.push_front(icon);
            }
        }
        Catch(BadSrcError)
        {
            _W("src attribute is invalid: %ls", (*m_src).c_str());
        }
    }

  private:
    ConfigParserData& m_data;
    DPL::OptionalString m_src;
    DPL::OptionalInt m_width;
    DPL::OptionalInt m_height;
    bool m_isSmall;

    static DPL::OptionalInt ParseSizeAttributeValue(const DPL::String& value)
    {
        DPL::OptionalString normalizedValue = value;
        if (!(*normalizedValue).empty()) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
            NormalizeString(normalizedValue, MAX_ATTR_ELEMENT_LENGTH);
#else
            NormalizeString(normalizedValue);
#endif
            char* reterr = NULL;
            errno = 0;
            long int valueInt =
                strtol(DPL::ToUTF8String(value).c_str(), &reterr, 10);
            if (errno != 0 ||
                std::string(reterr) == DPL::ToUTF8String(value) ||
                valueInt <= 0)
            {
                return DPL::OptionalInt();
            } else {
                return valueInt;
            }
        }
        return DPL::OptionalInt();
    }

    /**
     * @brief delocalizePath removes locales folder from relative path if
     * neccessary
     * @param source source string
     *
     * @throw BadSrcError if string is bad value of src attribute
     *
     * @return corrected string
     */
    static DPL::String delocalizeSrcPath(const DPL::String & source)
    {
        static const DPL::String localeFolder(L"locales/");
        static const int index = localeFolder.size();

        DPL::String result = source;

        if (source.substr(0, index) == localeFolder) {
            size_t pos = result.find_first_of('/', index);
            if (pos != std::string::npos && pos + 1 < source.size()) {
                result = result.substr(pos + 1, source.size());
            } else {
                Throw(BadSrcError);
            }
        }
        return result;
    }
};

class ContentParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }

    ContentParser(ConfigParserData& data) :
        ElementParser(),
        m_data(data)
    {}

    virtual void Accept(const Element& element)
    {
        m_namespace = element.ns;
    }

    virtual void Accept(const Text& /*text*/)
    {}

    virtual void Accept(const XmlAttribute& attribute)
    {
        DPL::String value = attribute.value;

        if (attribute.name == L"src") {
            m_src = value;
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
            NormalizeString(m_src, MAX_ATTR_ELEMENT_LENGTH);
#else
            NormalizeString(m_src);
#endif
        } else if (attribute.name == L"type") {
            m_type = value;
            MimeTypeUtils::MimeAttributes mimeAttributes =
                MimeTypeUtils::getMimeAttributes(value);
            if ((mimeAttributes.count(L"charset") > 0) && !m_encoding)
            {
                m_encoding = mimeAttributes[L"charset"];
            }
        } else if (attribute.name == L"encoding") {
            if (!value.empty()) {
                m_encoding = value;
            }
        }
    }

    virtual void Verify()
    {
        if(!!m_data.startFileEncountered)
        {
            if(m_data.startFileNamespace == m_namespace
                || m_namespace != ConfigurationNamespace::TizenWebAppNamespaceName)
            {
                return;
            }
            //else continue -> if previous item was not in tizen namespace
        }

        m_data.startFileEncountered = true;
        m_data.startFileNamespace = m_namespace;

        if (m_namespace == ConfigurationNamespace::TizenWebAppNamespaceName &&
                (!m_src || m_src->empty())) {
            ThrowMsg(Exception::ParseError, "content element must have correct src element");
        }

        if (!!m_src) {
            m_data.startFile = m_src;
            if (!!m_type) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
                NormalizeString(m_type, MAX_ATTR_ELEMENT_LENGTH);
#else
                NormalizeString(m_type);
#endif
                m_data.startFileContentType = m_type;
            }
            if (!!m_encoding) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
                NormalizeString(m_encoding, MAX_ATTR_ELEMENT_LENGTH);
#else
                NormalizeString(m_encoding);
#endif
                m_data.startFileEncoding = m_encoding;
            } else {
                m_data.startFileEncoding = L"UTF-8";
            }
        }
    }

  private:
    DPL::OptionalString m_src;
    DPL::OptionalString m_type;
    DPL::OptionalString m_encoding;
    ConfigParserData& m_data;
    DPL::String m_namespace;
};

class PreferenceParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (attribute.name == L"name") {
            m_name = attribute.value;
        } else if (attribute.name == L"value") {
            m_value = attribute.value;
        } else if (attribute.name == L"readonly") {
            if (attribute.value == L"true") {
                m_required = true;
            } else {
                m_required = false;
            }
        }
    }

    virtual void Accept(const Element& /*element*/)
    {}

    virtual void Accept(const Text& /*text*/)
    {
        ThrowMsg(Exception::ParseError, "param element must be empty");
    }

    virtual void Verify()
    {
        if (!m_name) {
            _W("preference element must have name attribute");
            return;
        }
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
        NormalizeString(m_name, MAX_NAME_KEY_LENGTH);
        NormalizeString(m_value, MAX_NAME_KEY_VALUE_LENGTH);
#else
        NormalizeString(m_name);
        NormalizeString(m_value);
#endif
        ConfigParserData::Preference preference(*m_name, m_required);
        preference.value = m_value;
        if (m_data.preferencesList.find(preference) ==
            m_data.preferencesList.end())
        {
            m_data.preferencesList.insert(preference);
        }
    }

    PreferenceParser(ConfigParserData& data) :
        ElementParser(),
        m_required(false),
        m_data(data)
    {}

  private:
    DPL::OptionalString m_name;
    DPL::OptionalString m_value;
    bool m_required;
    ConfigParserData& m_data;
};

class SettingParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }

    virtual void Accept(const Text& /*text*/)
    {}

    virtual void Accept(const Element& /*element*/)
    {}

    virtual void Accept(const XmlAttribute& attribute)
    {
        m_setting.m_name = attribute.name;
        m_setting.m_value = attribute.value;
        m_data.settingsList.insert(m_setting);
    }

    virtual void Verify()
    {
        if(m_data.serviceAppInfoList.size() > 0) {
            FOREACH(it, m_data.settingsList) {
                if (it->m_name == L"encryption" && it->m_value == L"enable") {
                    ThrowMsg(Exception::ParseError, "Service application does not support application encryption");
                }
            }
        }
    }

    SettingParser(ConfigParserData& data) :
        ElementParser(),
        m_data(data),
        m_setting(L"", L"")
    {}

  private:
    ConfigParserData& m_data;
    ConfigParserData::Setting m_setting;
};

class AppControlParser : public ElementParser
{
  public:
    struct SourceParser : public ElementParser
    {
      public:
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                            const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create; //ignore unknown according to w3c
        }

        virtual void Accept(const Text& /*text*/)
        {}

        virtual void Accept(const Element& /*element*/)
        {}

        virtual void Accept(const XmlAttribute& attribute)
        {
            if (attribute.name == L"name") {
                if (attribute.value.size() > 0) {
                    m_value = attribute.value;
                }
            } else if (attribute.name == L"on-reset") {
                if (attribute.value == L"enable") {
                    m_data.m_disposition = ConfigParserData::AppControlInfo::Disposition::UNDEFINE;
                } else if (attribute.value == L"disable") {
                    m_data.m_disposition = ConfigParserData::AppControlInfo::Disposition::WINDOW;
                } else {
                    ThrowMsg(Exception::ParseError, "Wrong boolean value");
                }
            }
        }

        virtual void Verify()
        {
            if (!m_value || *m_value == L"") {
                return;
            }
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
            NormalizeString(m_value, MAX_ATTR_ELEMENT_LENGTH);
#else
            NormalizeString(m_value);
#endif
            m_data.m_src = *m_value;
        }

        SourceParser(ConfigParserData::AppControlInfo& data) :
            ElementParser(),
            m_properNamespace(false),
            m_data(data)
        {}

      private:
        bool m_properNamespace;
        DPL::OptionalString m_value;
        ConfigParserData::AppControlInfo& m_data;
    };

    struct OperationParser : public ElementParser
    {
      public:
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                            const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create; //ignore unknown according to w3c
        }

        virtual void Accept(const Text& /*text*/)
        {}

        virtual void Accept(const Element& /*element*/)
        {}

        virtual void Accept(const XmlAttribute& attribute)
        {
            if (attribute.name == L"name") {
                if (attribute.value.size() > 0) {
                    m_value = attribute.value;
                }
            }
        }

        virtual void Verify()
        {
            if (!m_value || *m_value == L"") {
                return;
            }
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
            NormalizeString(m_value, MAX_ATTR_ELEMENT_LENGTH);
#else
            NormalizeString(m_value);
#endif
            m_data.m_operation = *m_value;
        }

        OperationParser(ConfigParserData::AppControlInfo& data) :
            ElementParser(),
            m_properNamespace(false),
            m_data(data)
        {}

      private:
        bool m_properNamespace;
        DPL::OptionalString m_value;
        ConfigParserData::AppControlInfo& m_data;
    };

    struct UriParser : public ElementParser
    {
      public:
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                            const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create; //ignore unknown according to w3c
        }

        virtual void Accept(const Text& /*text*/)
        {}

        virtual void Accept(const Element& /*element*/)
        {}

        virtual void Accept(const XmlAttribute& attribute)
        {
            if (attribute.name == L"name") {
                if (attribute.value.size() > 0) {
                    m_value = attribute.value;
                }
            }
        }

        virtual void Verify()
        {
            // exception
            DPL::String ignoreUri(L"file");

            if (!!m_value && *m_value == ignoreUri)
            {
                _D("exception : '%ls' scheme will be ignored.", (*m_value).c_str());
                m_value = DPL::OptionalString();
            }

            if (!m_value || *m_value == L"") {
                return;
            }
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
            NormalizeString(m_value, MAX_ATTR_ELEMENT_LENGTH);
#else
            NormalizeString(m_value);
#endif
            DPL::String wildString(L"*/*");
            if ((m_data.m_uriList.find(wildString) == m_data.m_uriList.end())
                && (m_data.m_uriList.find(*m_value) == m_data.m_uriList.end()))
            {
                m_data.m_uriList.insert(*m_value);
            } else {
                _D("Ignoring uri with name %ls", (*m_value).c_str());
            }
        }

        UriParser(ConfigParserData::AppControlInfo& data) :
            ElementParser(),
            m_properNamespace(false),
            m_data(data)
        {}

      private:
        bool m_properNamespace;
        DPL::OptionalString m_value;
        ConfigParserData::AppControlInfo& m_data;
    };

    struct MimeParser : public ElementParser
    {
      public:
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                            const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create; //ignore unknown according to w3c
        }

        virtual void Accept(const Text& /*text*/)
        {}

        virtual void Accept(const Element& /*element*/)
        {}

        virtual void Accept(const XmlAttribute& attribute)
        {
            if (attribute.name == L"name") {
                if (attribute.value.size() > 0) {
                    m_value = attribute.value;
                }
            }
        }

        virtual void Verify()
        {
            if (!m_value || *m_value == L"") {
                return;
            }
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
            NormalizeString(m_value, MAX_ATTR_ELEMENT_LENGTH);
#else
            NormalizeString(m_value);
#endif
            DPL::String wildString(L"*/*");
            if ((m_data.m_mimeList.find(wildString) ==
                 m_data.m_mimeList.end())
                && (m_data.m_mimeList.find(*m_value) ==
                    m_data.m_mimeList.end()))
            {
                m_data.m_mimeList.insert(*m_value);
            } else {
                _D("Ignoring mime with name %ls", (*m_value).c_str());
            }
        }

        MimeParser(ConfigParserData::AppControlInfo& data) :
            ElementParser(),
            m_properNamespace(false),
            m_data(data)
        {}

      private:
        bool m_properNamespace;
        DPL::OptionalString m_value;
        ConfigParserData::AppControlInfo& m_data;
    };

    struct DispositionParser : public ElementParser
    {
      public:
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                            const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create;
        }

        virtual void Accept(const Text& /*text*/)
        {}

        virtual void Accept(const Element& /*element*/)
        {}

        virtual void Accept(const XmlAttribute& attribute)
        {
            if (attribute.name == L"name") {
                if (attribute.value.size() > 0) {
                    m_value = attribute.value;
                    NormalizeString(m_value);
                }
            }
        }

        virtual void Verify()
        {
            if (!m_value || *m_value == L"") {
                return;
            }

            DPL::String windowString(L"window");
            DPL::String inlineString(L"inline");

            if (*m_value == L"window") {
                m_data.m_disposition =
                    ConfigParserData::AppControlInfo::Disposition::WINDOW;
            } else if (*m_value == L"inline") {
                m_data.m_disposition =
                    ConfigParserData::AppControlInfo::Disposition::INLINE;
            } else {
                _D("Ignoring dispostion value %ls", (*m_value).c_str());
            }
        }

        DispositionParser(ConfigParserData::AppControlInfo& data) :
            ElementParser(),
            m_properNamespace(false),
            m_data(data)
        {}

      private:
        bool m_properNamespace;
        DPL::OptionalString m_value;
        ConfigParserData::AppControlInfo& m_data;
    };

    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& name)
    {
        if (name == L"src") {
            return std::bind(&AppControlParser::OnSourceElement, this);
        } else if (name == L"operation") {
            return std::bind(&AppControlParser::OnOperationElement, this);
        } else if (name == L"uri") {
            return std::bind(&AppControlParser::OnUriElement, this);
        } else if (name == L"mime") {
            return std::bind(&AppControlParser::OnMimeElement, this);
        } else if (name == L"disposition") {
            return std::bind(&AppControlParser::OnDispositionElement, this);
        } else {
            return &IgnoringParser::Create; //ignore unknown according to w3c
        }
    }

    virtual void Accept(const XmlAttribute& /*attribute*/)
    {}

    virtual void Accept(const Element& element)
    {
        _W("namespace for app service = %ls", element.ns.c_str());
        if (element.ns == ConfigurationNamespace::W3CWidgetNamespaceName) {
            ThrowMsg(Exception::ParseError,
                     "Wrong xml namespace for widget element");
        }
    }

    virtual void Accept(const Text& /*text*/)
    {
        ThrowMsg(Exception::ParseError, "param element must be empty");
    }

    virtual void Verify()
    {
        if (m_appControl.m_src == L"") {
            ThrowMsg(Exception::ParseError, "service element must have src element");
        }

        if (m_appControl.m_operation == L"") {
            ThrowMsg(Exception::ParseError, "service element must have operation element");
        }

        auto res = std::find(m_data.appControlList.begin(), m_data.appControlList.end(), m_appControl);
        if(res != m_data.appControlList.end()) {
            ThrowMsg(Exception::ParseError, "service element must be unique");
        }

#ifdef NFC_EXCEPTION_HANDLING_FOR_TIZEN_2_2_ONLY
        // XXX This feature should be retained to Tizen 2.2 only.
        // NFC exception handling which was requested from Tizen Device API team.

        const DPL::String exceptionNfcOperation =
                       L"http://tizen.org/appcontrol/operation/nfc/transaction";
        const DPL::String exceptionNfcUri  = L"nfc://secure/aid/";
        const DPL::String divertingNfcUri1 = L"nfc://secure/SIM1/aid/";
        const DPL::String divertingNfcUri2 = L"nfc://secure/eSE/aid/";

        if (m_appControl.m_operation == exceptionNfcOperation
            && m_appControl.m_mimeList.empty()
            && m_appControl.m_uriList.size() == 1
            && (m_appControl.m_uriList.begin())->compare(0, exceptionNfcUri.length(), exceptionNfcUri) == 0)
        {
            DPL::String originalUri = *m_appControl.m_uriList.begin();
            DPL::String newUri = originalUri;

            newUri.replace(0, exceptionNfcUri.length(), divertingNfcUri1);
            m_appControl.m_uriList.erase(m_appControl.m_uriList.begin());
            m_appControl.m_uriList.insert(newUri);
            m_data.appControlList.push_back(m_appControl);
            _D("NFC exception : %ls -> %ls", originalUri.c_str(), newUri.c_str());

            newUri = originalUri;
            newUri.replace(0, exceptionNfcUri.length(), divertingNfcUri2);
            m_appControl.m_uriList.erase(m_appControl.m_uriList.begin());
            m_appControl.m_uriList.insert(newUri);
            m_data.appControlList.push_back(m_appControl);
            _D("NFC exception : %ls -> %ls", originalUri.c_str(), newUri.c_str());

            return;
        }
#endif // NFC_EXCEPTION_HANDLING_FOR_TIZEN_2_2_ONLY

        m_data.appControlList.push_back(m_appControl);
    }

    ElementParserPtr OnSourceElement()
    {
        return ElementParserPtr(new SourceParser(m_appControl));
    }

    ElementParserPtr OnOperationElement()
    {
        return ElementParserPtr(new OperationParser(m_appControl));
    }

    ElementParserPtr OnUriElement()
    {
        return ElementParserPtr(new UriParser(m_appControl));
    }

    ElementParserPtr OnMimeElement()
    {
        return ElementParserPtr(new MimeParser(m_appControl));
    }

    ElementParserPtr OnDispositionElement()
    {
        return ElementParserPtr(new DispositionParser(m_appControl));
    }

    AppControlParser(ConfigParserData& data) :
        ElementParser(),
        m_data(data),
        m_appControl(L"")
    {}

  private:
    ConfigParserData& m_data;
    ConfigParserData::AppControlInfo m_appControl;
};

class ApplicationParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }

    virtual void Accept(const Text& /*text*/)
    {
        if (m_properNamespace) {
            ThrowMsg(Exception::ParseError, "application element must be empty");
        }
    }

    virtual void Accept(const Element& element)
    {
        if (element.ns ==
            ConfigurationNamespace::TizenWebAppNamespaceName)
        {
            m_properNamespace = true;
        }
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (m_properNamespace) {
            if (attribute.name == L"id") {
                if (attribute.value.length() > MAX_APPLICATION_ID_LENGTH) {
                    ThrowMsg(Exception::ParseError,
                             "length of application id has crossed the allowed limit");
                } else {
                    m_id = attribute.value;
                    NormalizeAndTrimSpaceString(m_id);
                }
            } else if (attribute.name == L"package") {
                m_package = attribute.value;
            } else if (attribute.name == L"required_version") {
                m_version = attribute.value;
            } else {
                ThrowMsg(Exception::ParseError,
                         "unknown attribute '" +
                         DPL::ToUTF8String(attribute.name) +
                         "' in application element");
            }
        }
    }

    virtual void Verify()
    {
        if(m_data.didFoundTizenApplicationElement)
        {
            ThrowMsg(Exception::ParseError, "tizen:application element must occur only once");
        }
        m_data.didFoundTizenApplicationElement = true;

        VerifyIdAndPackage();
        VerifyVersion();
    }

    ApplicationParser(ConfigParserData& data) :
        ElementParser(),
        m_data(data),
        m_id(DPL::OptionalString()),
        m_version(DPL::OptionalString()),
        m_properNamespace(false)
    {}

    static const char* const REGEXP_ID;

  private:
    void VerifyIdAndPackage()
    {
        if (!m_package)
        {
            ThrowMsg(Exception::ParseError,
                     "application element must have package attribute");
        }
        else
        {
            pcrecpp::RE re(REGEXP_PACKAGE);
            if (!re.FullMatch(DPL::ToUTF8String(*m_package)))
            {
                ThrowMsg(Exception::ParseError,
                         "invalid format of package attribute");
            }
        }

        if (!m_id) {
            ThrowMsg(Exception::ParseError,
                     "application element must have id attribute");
        }
        else
        {
            std::string package;
            pcrecpp::RE re(REGEXP_ID);
            if (!re.FullMatch(DPL::ToUTF8String(*m_id), &package))
            {
                ThrowMsg(Exception::ParseError,
                         "invalid format of id attribute");
            }
            if (package != DPL::ToUTF8String(*m_package))
            {
                ThrowMsg(Exception::ParseError,
                         "invalid package prefix in id attribute");
            }
        }

        m_data.tizenAppId = m_id;
        m_data.tizenPkgId = m_package;
    }

    void VerifyVersion()
    {
        if (!m_version)
        {
            ThrowMsg(Exception::ParseError,
                     "application element must have required_version attribute");
        }
        else
        {
            pcrecpp::RE re(REGEXP_VERSION);
#ifdef ELEMENT_ATTR_MAX_LENGTH
            NormalizeString(m_version, MAX_ATTR_ELEMENT_LENGTH);
#else
            NormalizeString(m_version);
#endif
            if (!re.FullMatch(DPL::ToUTF8String(*m_version)))
            {
                ThrowMsg(Exception::ParseError,
                         "invalid format of version attribute");
            }
        }

        m_data.tizenMinVersionRequired = m_version;
    }

    static const char* const REGEXP_PACKAGE;
    static const char* const REGEXP_VERSION;

    ConfigParserData& m_data;
    DPL::OptionalString m_id;
    DPL::OptionalString m_package;
    DPL::OptionalString m_version;
    bool m_properNamespace;
};

const char* const ApplicationParser::REGEXP_PACKAGE = "[0-9A-Za-z]{10}";
const char* const ApplicationParser::REGEXP_ID = "([0-9A-Za-z]{10})\\.[0-9A-Za-z]{1,52}";
const char* const ApplicationParser::REGEXP_VERSION = "\\d+\\.\\d+(\\.\\d+)*";

class SplashParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (m_properNamespace)
        {
            if (attribute.name == L"src") {
                if (attribute.value.size() > 0) {
                    m_src = attribute.value;
                }
            }
        }
    }

    virtual void Accept(const Element& element)
    {
        if (element.ns ==
            ConfigurationNamespace::TizenWebAppNamespaceName)
        {
            m_properNamespace = true;
        }
    }

    virtual void Accept(const Text& /*text*/)
    {}

    virtual void Verify()
    {
        if (!m_src)
        {
            _W("src attribute of splash element is mandatory - ignoring");
            return;
        }

        m_data.splashImgSrc = m_src;
    }

    SplashParser(ConfigParserData& data) :
        ElementParser(),
        m_data(data),
        m_properNamespace(false)
    {}

  private:
    DPL::OptionalString m_src;
    ConfigParserData& m_data;
    bool m_properNamespace;
};

class BackgroundParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (attribute.name == L"src") {
            if (attribute.value.size() > 0) {
                m_src = attribute.value;
            }
        }
    }

    virtual void Accept(const Element& /*element*/)
    {}

    virtual void Accept(const Text& /*text*/)
    {}

    virtual void Verify()
    {
        if (!m_src) {
            _W("src attribute of background element is mandatory - ignoring");
            return;
        }

        m_data.backgroundPage = m_src;
    }

    explicit BackgroundParser(ConfigParserData& data) :
        m_data(data)
    {}

  private:
    DPL::OptionalString m_src;
    ConfigParserData& m_data;
};

class PrivilegeParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }

    virtual void Accept(const Text& /*text*/)
    {}

    virtual void Accept(const Element& element)
    {
        if (element.ns ==
            ConfigurationNamespace::TizenWebAppNamespaceName)
        {
            m_properNamespace = true;
        }
        _D("element");
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (m_properNamespace) {
            if (attribute.name == L"name") {
                m_feature.name = attribute.value;
                m_privilege.name = attribute.value;
            }
        }
    }

    virtual void Verify()
    {
        LibIri::Wrapper iri(DPL::ToUTF8String(m_feature.name).c_str());

        if (m_feature.name != L"") {
            if (iri.Validate()) {
                if (m_data.featuresList.find(m_feature) ==
                    m_data.featuresList.end())
                {
                    m_data.featuresList.insert(m_feature);
                } else {
                    _D("Ignoring feature with name %ls", m_feature.name.c_str());
                }
            }
        }

        LibIri::Wrapper iriPrivilege(
            DPL::ToUTF8String(m_privilege.name).c_str());

        if (m_privilege.name != L"") {
            if (iriPrivilege.Validate()) {
                if (m_data.privilegeList.find(m_privilege) ==
                    m_data.privilegeList.end())
                {
                    m_data.privilegeList.insert(m_privilege);
                } else {
                    _D("Ignoring privilege with name %ls", m_privilege.name.c_str());
                }
            }
        }
    }

    PrivilegeParser(ConfigParserData& data) :
        ElementParser(),
        m_data(data),
        m_feature(L""),
        m_privilege(L""),
        m_properNamespace(false)
    {}

  private:
    ConfigParserData& m_data;
    ConfigParserData::Feature m_feature;
    ConfigParserData::Privilege m_privilege;
    bool m_properNamespace;
};

class CategoryParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }

    virtual void Accept(const Element& element)
    {
        if (element.ns ==
            ConfigurationNamespace::TizenWebAppNamespaceName)
        {
            m_properNamespace = true;
        }
        LogDebug("element");
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (m_properNamespace) {
            if (attribute.name == L"name") {
                if (attribute.value.size() > 0) {
                    m_name = attribute.value;
                }
            }
        }
    }

    virtual void Accept(const Text& /*text*/)
    {}

    virtual void Verify()
    {
        if (!m_name) {
            _W("name attribute of category element is mandatory - ignoring");
            return;
        }

        if (m_data.categoryList.find(*m_name) ==
            m_data.categoryList.end())
        {
            m_data.categoryList.insert(*m_name);
        }

    }

    explicit CategoryParser(ConfigParserData& data) :
        m_data(data),
        m_properNamespace(false)
    {}

  private:
    DPL::OptionalString m_name;
    ConfigParserData& m_data;
    bool m_properNamespace;
};

#ifdef DBOX_ENABLED
class AppWidgetParser : public ElementParser
{
  public:

    struct BoxLabelParser : public ElementParser
    {
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                            const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create; //ignore unknown according to w3c
        }

        virtual void Accept(const XmlAttribute& attribute)
        {
            if (m_properNamespace) {
                 m_lang = attribute.lang;
            }
        }
        virtual void Accept(const Element& element)
        {
            if (element.ns ==
                ConfigurationNamespace::TizenWebAppNamespaceName)
            {
                m_properNamespace = true;
            }
        }

        virtual void Accept(const Text& text)
        {
            if (m_properNamespace) {
                m_label = text.value;
            }
        }

        virtual void Verify()
        {
            std::pair<DPL::String, DPL::String> boxLabel;
            if (m_label.empty()) {
                _W("box-label element is empty");
                boxLabel.first = DPL::FromUTF8String("");
                boxLabel.second = DPL::FromUTF8String("");
                m_data.m_label.push_back(boxLabel);
            }
            else {
                boxLabel.first = m_lang;
                boxLabel.second = m_label;
                m_data.m_label.push_back(boxLabel);
            }
        }

        BoxLabelParser(ConfigParserData::LiveboxInfo& data) :
            ElementParser(),
            m_properNamespace(false),
            m_data(data)
        {}

      private:
        DPL::String m_lang;
        DPL::String m_label;
        bool m_properNamespace;
        ConfigParserData::LiveboxInfo& m_data;
    };

    struct BoxIconParser : public ElementParser
    {
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                            const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create; //ignore unknown according to w3c
        }

        virtual void Accept(const XmlAttribute& attribute)
        {
            if (m_properNamespace) {
                if (attribute.name == L"src") {
                    m_icon = attribute.value;
                }
            }
        }

        virtual void Accept(const Element& element)
        {
            if (element.ns ==
                ConfigurationNamespace::TizenWebAppNamespaceName)
            {
                m_properNamespace = true;
            }
        }

        virtual void Accept(const Text& /*text*/)
        {}

        virtual void Verify()
        {
            if (m_icon.empty()) {
                ThrowMsg(Exception::ParseError,
                    "src attribute of box-icon element is mandatory - ignoring");
            }
            if (!m_data.m_icon.empty()) {
                ThrowMsg(Exception::ParseError,
                    "<tizen:box-icon /> element should occur as 0 or 1 time");
            }
            m_data.m_icon = m_icon;
        }

        explicit BoxIconParser(ConfigParserData::LiveboxInfo& data) :
            ElementParser(),
            m_properNamespace(false),
            m_data(data)
        {}

      private:
        DPL::String m_icon;
        bool m_properNamespace;
        ConfigParserData::LiveboxInfo& m_data;
    };

    struct BoxContentParser : public ElementParser
    {
        struct BoxSizeParser : public ElementParser
        {
            virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                                const DPL::String& /*name*/)
            {
                return &IgnoringParser::Create; //ignore unknown according to w3c
            }

            virtual void Accept(const XmlAttribute& attribute)
            {
                if (m_properNamespace) {
                    if (attribute.name == L"preview") {
                        m_preview = attribute.value;
                    }
                    if (attribute.name == L"use-decoration") {
                        m_useDecoration = attribute.value;
                    }
                }
            }

            virtual void Accept(const Element& element)
            {
                if (element.ns ==
                    ConfigurationNamespace::TizenWebAppNamespaceName)
                {
                    m_properNamespace = true;
                }
            }

            virtual void Accept(const Text& text)
            {
                if (m_properNamespace) {
                    m_size = text.value;
                }
            }

            virtual void Verify()
            {
                if(m_size.empty()) {
                    ThrowMsg(Exception::ParseError,
                        "size is mandatory - ignoring");
                }

                if (m_useDecoration.empty() || CheckIfNotTrueNorFalse(m_useDecoration)) {
                    m_useDecoration = L"true"; // default value
                }

                ConfigParserData::LiveboxInfo::BoxSizeInfo boxSizeInfo;
                boxSizeInfo.m_size = m_size;
                boxSizeInfo.m_preview = m_preview;
                boxSizeInfo.m_useDecoration = m_useDecoration;
                m_data.m_boxSize.push_back(boxSizeInfo);
            }

            explicit BoxSizeParser(
                ConfigParserData::LiveboxInfo::BoxContentInfo& data) :
                ElementParser(),
                m_properNamespace(false),
                m_data(data)
            {}

          private:
            DPL::String m_size;
            DPL::String m_preview;
            DPL::String m_useDecoration;
            bool m_properNamespace;
            ConfigParserData::LiveboxInfo::BoxContentInfo& m_data;
        };

        struct PdParser : public ElementParser
        {
            virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                                const DPL::String& /*name*/)
            {
                return &IgnoringParser::Create; //ignore unknown according to w3c
            }

            virtual void Accept(const XmlAttribute& attribute)
            {
                if (m_properNamespace) {
                    if (attribute.name == L"src") {
                        m_src = attribute.value;
                    } else if (attribute.name == L"width") {
                        m_width = attribute.value;
                    } else if (attribute.name == L"height") {
                        m_height = attribute.value;
                    } else if (attribute.name == L"fast-open") {
                        m_fastOpen= attribute.value;
                    }
                }
            }

            virtual void Accept(const Element& element)
            {
                if (element.ns ==
                    ConfigurationNamespace::TizenWebAppNamespaceName)
                {
                    m_properNamespace = true;
                }
            }

            virtual void Accept(const Text& /*text*/)
            {}

            virtual void Verify()
            {
                if (m_src.empty()) {
                    ThrowMsg(Exception::ParseError,
                        "src attribute of pd element is mandatory - ignoring");
                }

                if (m_width.empty()) {
                    ThrowMsg(Exception::ParseError,
                        "width attribute of pd element is mandatory - ignoring");
                }

                if (m_height.empty()) {
                    ThrowMsg(Exception::ParseError,
                        "height attribute of pd element is mandatory - ignoring");
                }

                if (!ConvertToInt(m_width)) {
                    ThrowMsg(Exception::ParseError,
                        "width attribute of pd element cannot be converted to int - ignoring. value: " << m_width);
                }


                DPL::OptionalInt height = ConvertToInt(m_height);

                if (!height) {
                    ThrowMsg(Exception::ParseError,
                        "height attribute of pd element cannot be converted to int - ignoring. value: " << m_height);
                }

                if (*height < 1) {
                    m_height = L"1";
                    _D("height attribute of pd element shouldn't be less than 1. Changed to 1 from %d", *height);
                } else if (*height > 380){
                    m_height = L"380";
                    _D("height attribute of pd element shouldn't be greater than 380. Changed to 380 from %d", *height);
                }

                if (!m_data.m_pdSrc.empty()) {
                    ThrowMsg(Exception::ParseError, "<tizen:pd> element should occur as 0 or 1 time");
                }

                m_data.m_pdSrc = m_src;
                m_data.m_pdWidth = m_width;
                m_data.m_pdHeight = m_height;
                m_data.m_pdFastOpen = m_fastOpen;
            }

            explicit PdParser(
                ConfigParserData::LiveboxInfo::BoxContentInfo& data) :
                ElementParser(),
                m_properNamespace(false),
                m_data(data)
            {}

          private:
            DPL::OptionalInt ConvertToInt(const DPL::String& intAsString)
            {
                char * endptr;
                std::string tempStr = DPL::ToUTF8String(intAsString);
                const char * intAsString_c = tempStr.c_str();
                errno = 0;
                long int intAsString_i = strtol(intAsString_c, &endptr, 10);

                if ((errno == ERANGE && (intAsString_i == LONG_MAX || intAsString_i == LONG_MIN))
                        || intAsString_i > INT_MAX || intAsString_i < INT_MIN
                        || *endptr != '\0') {
                    return DPL::OptionalInt();
                }

                return static_cast<int>(intAsString_i);
            }

            DPL::String m_src;
            DPL::String m_width;
            DPL::String m_height;
            DPL::String m_fastOpen;

            bool m_properNamespace;
            ConfigParserData::LiveboxInfo::BoxContentInfo& m_data;
        };

        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                            const DPL::String& name)
        {
            if (name == L"box-size") {
                return std::bind(&AppWidgetParser::BoxContentParser::OnBoxSizeElement, this);
            } else if (name == L"pd") {
                return std::bind(&AppWidgetParser::BoxContentParser::OnPdElement, this);
            } else {
                ThrowMsg(Exception::ParseError,
                         "No element parser for name: " << name);
            }
        }

        virtual void Accept(const XmlAttribute& attribute)
        {
            if (m_properNamespace) {
                if (attribute.name == L"src") {
                    m_box.m_boxSrc = attribute.value;
                }
                if (attribute.name == L"mouse-event") {
                    m_box.m_boxMouseEvent = attribute.value;
                }
                if (attribute.name == L"touch-effect") {
                    m_box.m_boxTouchEffect = attribute.value;
                }
            }
        }

        virtual void Accept(const Element& element)
        {
            if (element.ns ==
                ConfigurationNamespace::TizenWebAppNamespaceName)
            {
                m_properNamespace = true;
            }
        }

        virtual void Accept(const Text& /*text*/)
        {}

        virtual void Verify()
        {
            if (m_box.m_boxSrc.empty()) {
                ThrowMsg(Exception::ParseError,
                    "src attribute of box-content element is mandatory - ignoring");
            }

            if (m_box.m_boxMouseEvent.empty() || CheckIfNotTrueNorFalse(m_box.m_boxMouseEvent)) {
                m_box.m_boxMouseEvent = L"false"; // default value
            }

            if (m_box.m_boxTouchEffect.empty() || CheckIfNotTrueNorFalse(m_box.m_boxTouchEffect)) {
                m_box.m_boxTouchEffect = L"true"; // default value
            }

            if (m_box.m_boxSize.empty()) {
                ThrowMsg(Exception::ParseError,
                    "box-size element of box-content element not found - ignoring");
            }

            if (!m_data.m_boxInfo.m_boxSrc.empty()) {
                ThrowMsg(Exception::ParseError, "<tizen:box-content> element must occur exactly 1 time");
            }

            m_data.m_boxInfo = m_box;
        }

        explicit BoxContentParser(ConfigParserData::LiveboxInfo& data) :
            ElementParser(),
            m_properNamespace(false),
            m_data(data)
        {}

        ElementParserPtr OnBoxSizeElement()
        {
            return ElementParserPtr(new BoxSizeParser(m_box));
        }

        ElementParserPtr OnPdElement()
        {
            return ElementParserPtr(new PdParser(m_box));
        }

      private:
        bool m_properNamespace;
        ConfigParserData::LiveboxInfo& m_data;
        ConfigParserData::LiveboxInfo::BoxContentInfo m_box;
    };

    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& name)
    {
        if (name == L"box-label") {
            return std::bind(&AppWidgetParser::OnBoxLabelElement, this);
        } else if (name == L"box-icon") {
            return std::bind(&AppWidgetParser::OnBoxIconElement, this);
        } else if (name == L"box-content") {
            return std::bind(&AppWidgetParser::OnBoxContentElement, this);
        } else {
            return &IgnoringParser::Create; //ignore unknown according to w3c
        }
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (m_properNamespace) {
            if (attribute.name == L"id") {
                m_liveboxId = attribute.value;
            } else if (attribute.name == L"primary") {
                m_primary = attribute.value;
            } else if (attribute.name == L"auto-launch") {
                m_autoLaunch = attribute.value;
            } else if (attribute.name == L"update-period") {
                m_updatePeriod = attribute.value;
            } else if (attribute.name == L"type") {
                m_type = attribute.value;
            }
        }
    }

    virtual void Accept(const Element& element)
    {
        if (element.ns ==
            ConfigurationNamespace::TizenWebAppNamespaceName)
        {
            m_properNamespace = true;
        }
    }

    virtual void Accept(const Text& /*text*/)
    {}

    virtual void Verify()
    {
        if (m_liveboxId.empty()) {
            ThrowMsg(Exception::ParseError,
                 "app-widget element must have id attribute");
        }
        else
        {
            pcrecpp::RE re(REGEXP_ID_STRING.c_str());
            if (!re.FullMatch(DPL::ToUTF8String(m_liveboxId)))
            {
                ThrowMsg(Exception::ParseError,
                     "invalid format of app-widget id attribute");
            }
        }

        if (m_primary.empty() || CheckIfNotTrueNorFalse(m_primary))
        {
            m_primary = L"true"; // default value
        }

        if (!m_updatePeriod.empty())
        {
            char * endptr;
            errno = 0;
            std::string tempStr = DPL::ToUTF8String(m_updatePeriod);

            //set standard locale to fix decimal point mark - '.'
            std::string currentLocale = setlocale(LC_NUMERIC, NULL);
            if (NULL == setlocale(LC_NUMERIC, "C"))
                _W("Failed to change locale to \"C\"");
            double updatePeriod = strtod(tempStr.c_str(), &endptr);

            //go back to previous locale
            if (NULL == setlocale(LC_NUMERIC, currentLocale.c_str()))
                _W("Failed to set previous locale");

            if ((errno == ERANGE && (updatePeriod == -HUGE_VAL || updatePeriod == HUGE_VAL))
                    || *endptr != '\0') {
                ThrowMsg(Exception::ParseError,
                    "update-period attribute of app-widget element should be a number - ignoring. current value: " << m_updatePeriod);
            } else if (updatePeriod < 1800.0) {
                _D("update-period attribute of app-widget element shouldn't be less than 1800.0 - changed to 1800 from value: %ls", m_updatePeriod.c_str());
                m_updatePeriod = L"1800.0";
            }
        }

        if (m_autoLaunch.empty() || CheckIfNotTrueNorFalse(m_autoLaunch))
        {
            m_autoLaunch = L"false"; // default value
        }

        if(m_livebox.m_label.empty()) {
            ThrowMsg(Exception::ParseError,
                "box-label element of app-widget element not found - ignoring");
        }

        if(!m_boxContentFound) {
            ThrowMsg(Exception::ParseError,
                "box-content element of app-widget element not found - ignoring");
        }

        m_livebox.m_liveboxId = m_liveboxId;
        m_livebox.m_primary = m_primary;
        m_livebox.m_autoLaunch = m_autoLaunch;
        m_livebox.m_updatePeriod = m_updatePeriod;
        m_livebox.m_type = m_type;

        m_data.m_livebox.push_back(m_livebox);
    }

    explicit AppWidgetParser(ConfigParserData& data) :
        ElementParser(),
        m_data(data),
        m_properNamespace(false),
        m_boxContentFound(false)
    {
        m_livebox = ConfigParserData::LiveboxInfo();
    }

    ElementParserPtr OnBoxLabelElement()
    {

        return ElementParserPtr(new BoxLabelParser(m_livebox));
    }

    ElementParserPtr OnBoxIconElement()
    {
        return ElementParserPtr(new BoxIconParser(m_livebox));
    }

    ElementParserPtr OnBoxContentElement()
    {
        m_boxContentFound = true;
        return ElementParserPtr(new BoxContentParser(m_livebox));
    }

  private:
    static std::string REGEXP_ID_STRING;
    ConfigParserData& m_data;
    ConfigParserData::LiveboxInfo m_livebox;
    DPL::String m_liveboxId;
    DPL::String m_primary;
    DPL::String m_autoLaunch;
    DPL::String m_updatePeriod;
    DPL::String m_type;
    bool m_properNamespace;
    bool m_boxContentFound;

    static bool CheckIfNotTrueNorFalse(const DPL::String &stringToCheck)
    {
        return stringToCheck.compare(L"true") != 0 && stringToCheck.compare(L"false") != 0;
    }
};

std::string AppWidgetParser::REGEXP_ID_STRING = std::string(ApplicationParser::REGEXP_ID) + "\\.[0-9A-Za-z]+";
#endif

class AllowNavigationParser : public ElementParser
{
  public:
    AllowNavigationParser(ConfigParserData& data) :
      ElementParser(),
      m_data(data),
      m_properNamespace(false)
    {}

    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }

    virtual void Accept(const Element& element)
    {
        if (element.ns == ConfigurationNamespace::TizenWebAppNamespaceName) {
            m_properNamespace = true;
        }
    }

    virtual void Accept(const Text& text)
    {
        if (m_properNamespace)
        {
            m_origin = text.value;
        }
    }

    virtual void Accept(const XmlAttribute& /*attribute*/)
    {
    }

    virtual void Verify()
    {
        if (m_data.allowNavigationEncountered || !m_properNamespace)
        {
            return;
        }
        m_data.allowNavigationEncountered = true;

        if (!m_origin) {
            _W("data is empty");
            return;
        }

        char* data = strdup(DPL::ToUTF8String(*m_origin).c_str());
        char* ptr = strtok(data," \n\r\t");
        while (ptr != NULL) {
            std::string origin = ptr;
            ptr = strtok(NULL," \n\r\t");
            if(origin == "*") {
                ConfigParserData::AllowNavigationInfo info(L"*", L"*");
                m_data.allowNavigationInfoList.push_back(info);
                continue;
            }

            std::unique_ptr<iri_t, decltype(&iri_destroy)> iri(iri_parse(origin.c_str()), iri_destroy);
            if (!iri->host || strlen(iri->host) == 0) {
                // input origin should has schem and host
                // in case of file scheme path is filled
                // "http://"
                _W("input origin isn't verified");
                continue;
            }
            DPL::String scheme = L"*";
            if (iri->scheme && strlen(iri->scheme) != 0) {
                scheme = DPL::FromUTF8String(iri->scheme);
            }
            ConfigParserData::AllowNavigationInfo info(
                scheme,
                DPL::FromUTF8String(iri->host));
            m_data.allowNavigationInfoList.push_back(info);
        }
        free(data);
    }

  private:
    DPL::OptionalString m_origin;
    ConfigParserData& m_data;
    bool m_properNamespace;
};

class CspParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }

    CspParser(ConfigParserData& data) :
        ElementParser(),
        m_data(data),
        m_properNamespace(false)
    {}

    virtual void Accept(const Element& element)
    {
        if (element.ns == ConfigurationNamespace::TizenWebAppNamespaceName) {
            m_properNamespace = true;
        }
    }

    virtual void Accept(const XmlAttribute& /*attribute*/)
    {}

    virtual void Accept(const Text& text)
    {
        if (m_properNamespace) {
            m_policy = text.value;
        }
    }

    virtual void Verify()
    {
        if (m_data.cspPolicyEncountered) {
            return;
        }
        m_data.cspPolicyEncountered = true;

        if (!!m_policy) {
            m_data.cspPolicy = *m_policy;
        }
    }

  private:
    ConfigParserData& m_data;
    bool m_properNamespace;
    DPL::OptionalString m_policy;
};

class CspReportOnlyParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }

    CspReportOnlyParser(ConfigParserData& data) :
        ElementParser(),
        m_data(data),
        m_properNamespace(false)
    {}

    virtual void Accept(const Element& element)
    {
        if (element.ns == ConfigurationNamespace::TizenWebAppNamespaceName) {
            m_properNamespace = true;
        }
    }

    virtual void Accept(const XmlAttribute& /*attribute*/)
    {}

    virtual void Accept(const Text& text)
    {
        if (m_properNamespace) {
            m_policy = text.value;
        }
    }

    virtual void Verify()
    {
        if (m_data.cspPolicyReportOnlyEncountered) {
            return;
        }
        m_data.cspPolicyReportOnlyEncountered = true;

        if (!!m_policy) {
            m_data.cspPolicyReportOnly = *m_policy;
        }
    }

  private:
    ConfigParserData& m_data;
    bool m_properNamespace;
    DPL::OptionalString m_policy;
};

class AccountParser : public ElementParser
{
  public:
      struct IconParser : public ElementParser
    {
        public:
            virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                    const DPL::String& /*name*/)
            {
                return &IgnoringParser::Create; //ignore unknown according to w3c
            }

            virtual void Accept(const Text& text)
            {
                if (text.value == L"") {
                    return;
                }
                m_value = text.value;
            }

            virtual void Accept(const Element& /*element*/)
            {}

            virtual void Accept(const XmlAttribute& attribute)
            {
                if (attribute.name == L"section") {
                    if (attribute.value == L"Account") {
                        m_type = ConfigParserData::IconSectionType::DefaultIcon;
                    } else if (attribute.value == L"AccountSmall") {
                        m_type = ConfigParserData::IconSectionType::SmallIcon;
                    }
                }
            }

            virtual void Verify()
            {
                if (!m_value || *m_value == L"") {
                    return;
                }

                std::pair<ConfigParserData::IconSectionType, DPL::String> icon;
                icon.first = m_type;
                icon.second = *m_value;

                m_data.m_iconSet.insert(icon);
            }

            IconParser(ConfigParserData::AccountProvider& data) :
                ElementParser(),
                m_properNamespace(false),
                m_type(ConfigParserData::DefaultIcon),
                m_data(data)
        {}

        private:
            bool m_properNamespace;
            ConfigParserData::IconSectionType m_type;
            ConfigParserData::AccountProvider& m_data;
            DPL::OptionalString m_value;
    };

      struct DisplayNameParser : public ElementParser
    {
        public:
            virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                    const DPL::String& /*name*/)
            {
                return &IgnoringParser::Create; //ignore unknown according to w3c
            }

            virtual void Accept(const Text& text)
            {
                if (text.value == L"") {
                    return;
                }
                m_value = text.value;
            }

            virtual void Accept(const Element& element)
            {
                m_lang = element.lang;
                m_value= L"";
            }

            virtual void Accept(const XmlAttribute& /*attribute*/)
            {}

            virtual void Verify()
            {
                if (!m_value || *m_value == L"") {
                    return;
                }

                std::pair<DPL::String, DPL::String> name;
                name.first = *m_lang;
                name.second = *m_value;

                m_data.m_displayNameSet.insert(name);
            }

            DisplayNameParser(ConfigParserData::AccountProvider& data) :
                ElementParser(),
                m_properNamespace(false),
                m_data(data)
        {}

        private:
            bool m_properNamespace;
            DPL::OptionalString m_lang;
            DPL::OptionalString m_value;
            ConfigParserData::AccountProvider& m_data;
    };

      struct CapabilityParser : public ElementParser
    {
        public:
            virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                    const DPL::String& /*name*/)
            {
                return &IgnoringParser::Create; //ignore unknown according to w3c
            }

            virtual void Accept(const Text& text)
            {
                if (text.value == L"") {
                    return;
                }
                m_value = text.value;
            }

            virtual void Accept(const Element& /*element*/)
            {}

            virtual void Accept(const XmlAttribute& /*attribute*/)
            {}

            virtual void Verify()
            {
                if (!m_value || *m_value == L"") {
                    return;
                }
                m_data.m_capabilityList.push_back(*m_value);
            }

            CapabilityParser(ConfigParserData::AccountProvider& data) :
                ElementParser(),
                m_properNamespace(false),
                m_data(data)
        {}

        private:
            bool m_properNamespace;
            DPL::OptionalString m_value;
            ConfigParserData::AccountProvider& m_data;
    };
      virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
              const DPL::String& name)
      {
          if (name == L"icon") {
              return std::bind(&AccountParser::OnIconElement, this);
          } else if (name == L"display-name") {
              return std::bind(&AccountParser::OnDisplayNameElement, this);
          } else if (name == L"capability") {
              return std::bind(&AccountParser::OnCapabilityElement, this);
          } else {
              return &IgnoringParser::Create; //ignore unknown according to w3c
          }
      }

      virtual void Accept(const Text& /*text*/)
      {}

      virtual void Accept(const Element& /*element*/)
      {}

      virtual void Accept(const XmlAttribute& attribute)
      {
          if (attribute.name == L"multiple-account-support") {
              if (attribute.value == L"true") {
                  m_account.m_multiAccountSupport = true;
              }
          }
      }

      virtual void Verify()
      {
      }

      ElementParserPtr OnIconElement()
      {
          return ElementParserPtr(new IconParser(m_account));
      }

      ElementParserPtr OnDisplayNameElement()
      {
          return ElementParserPtr(new DisplayNameParser(m_account));
      }

      ElementParserPtr OnCapabilityElement()
      {
          return ElementParserPtr(new CapabilityParser(m_account));
      }

      AccountParser(ConfigParserData& data) :
          ElementParser(),
          m_properNamespace(false),
          m_multiSupport(false),
          m_data(data),
          m_account(data.accountProvider)
    {
    }

  private:
      bool m_properNamespace;
      bool m_multiSupport;
      ConfigParserData& m_data;
      ConfigParserData::AccountProvider& m_account;
};

class MetadataParser : public ElementParser
{
  public:
    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& /*name*/)
    {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (m_properNamespace) {
            if (attribute.name == L"key") {
                m_key = attribute.value;
            } else if (attribute.name == L"value") {
                m_value = attribute.value;
            }
        }
    }

    virtual void Accept(const Element& element)
    {
        if (element.ns == ConfigurationNamespace::TizenWebAppNamespaceName) {
            m_properNamespace = true;
        }
    }

    virtual void Accept(const Text& /*text*/)
    {
        ThrowMsg(Exception::ParseError, "param element must be empty");
    }

    virtual void Verify()
    {
        if (!m_key) {
            _W("metadata element must have key attribute");
            return;
        }
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
        NormalizeString(m_key, MAX_NAME_KEY_LENGTH);
        NormalizeString(m_value, MAX_NAME_KEY_VALUE_LENGTH);
#else
        NormalizeString(m_key);
        NormalizeString(m_value);
#endif
        ConfigParserData::Metadata metaData(m_key, m_value);
        FOREACH(it, m_data.metadataList) {
            if (!DPL::StringCompare(*it->key, *m_key)) {
                _E("Key isn't unique");
                return;
            }
        }
        m_data.metadataList.push_back(metaData);
    }

    MetadataParser(ConfigParserData& data) :
        ElementParser(),
        m_data(data),
        m_properNamespace(false)
    {}

  private:
    DPL::OptionalString m_key;
    DPL::OptionalString m_value;
    ConfigParserData& m_data;
    bool m_properNamespace;
};

#ifdef IME_ENABLED
class ImeParser : public ElementParser
{
  public:
    struct UuidParser : public ElementParser
    {
      public:
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                            const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create;
        }

        virtual void Accept(const XmlAttribute& attribute)
        {}

        virtual void Accept(const Element& element)
        {}

        virtual void Accept(const Text& text)
        {
            if (m_uuid.empty()) {
                m_uuid = text.value;
            } else {
                m_uuid += text.value;
            }
        }

        virtual void Verify()
        {
            if (m_uuid.empty()) {
                ThrowMsg(Exception::ParseError, "uuid text is empty");
            }
            m_imeAppInfo.uuid = m_uuid;
        }

        UuidParser(ConfigParserData::ImeAppInfo& data) :
            ElementParser(),
            m_imeAppInfo(data)
        {}

      private:
          ConfigParserData::ImeAppInfo& m_imeAppInfo;
          DPL::String m_uuid;
    };

    struct LanguagesParser : public ElementParser
    {
      public:
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                            const DPL::String& name)
        {
            if (name == L"language") {
                return std::bind(&LanguagesParser::OnLanguageElement, this);
            } else {
                return &IgnoringParser::Create;
            }
        }

        virtual void Accept(const XmlAttribute& attribute)
        {}

        virtual void Accept(const Element& element)
        {}

        virtual void Accept(const Text& text)
        {
            ThrowMsg(Exception::ParseError, "param element must be empty");
        }

        virtual void Verify()
        {}

        LanguagesParser(ConfigParserData::ImeAppInfo& data) :
            ElementParser(),
            m_imeAppInfo(data)
        {}

        ElementParserPtr OnLanguageElement()
        {
            return ElementParserPtr(new LanguageParser(m_imeAppInfo));
        }

      private:
          ConfigParserData::ImeAppInfo& m_imeAppInfo;
    };

    struct LanguageParser : public ElementParser
    {
      public:
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                                    const DPL::String& name)
        {
            return &IgnoringParser::Create;
        }

        virtual void Accept(const XmlAttribute& attribute)
        {}

        virtual void Accept(const Element& element)
        {}

        virtual void Accept(const Text& text)
        {
            if (m_language.empty()) {
                m_language = text.value;
            } else {
                m_language += text.value;
            }
        }

        virtual void Verify()
        {
            if (m_language.empty()) {
                ThrowMsg(Exception::ParseError, "language text is empty");
            }
            m_imeAppInfo.languageList.insert(m_language);
        }

        LanguageParser(ConfigParserData::ImeAppInfo& data) :
            ElementParser(),
            m_imeAppInfo(data)
        {}

      private:
          ConfigParserData::ImeAppInfo& m_imeAppInfo;
          DPL::String m_language;
    };

    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& name)
    {
        if (name == L"uuid") {
            return std::bind(&ImeParser::OnUuidElement, this);
        } else if (name == L"languages") {
            return std::bind(&ImeParser::OnLanguagesElement, this);
        } else {
            return &IgnoringParser::Create;
        }
    }

    virtual void Accept(const Text& /*text*/)
    {
        ThrowMsg(Exception::ParseError, "param element must be empty");
    }

    virtual void Accept(const Element& element)
    {}

    virtual void Accept(const XmlAttribute& attribute)
    {}

    virtual void Verify()
    {
        if (m_imeAppInfo.uuid.empty()) {
            ThrowMsg(Exception::ParseError, "ime element must have uuid element");
            return;
        }

        if (m_imeAppInfo.languageList.empty()) {
            ThrowMsg(Exception::ParseError, "ime element must have language element");
            return;
        }
        m_data.imeAppInfoList.push_back(m_imeAppInfo);
    }

    ImeParser(ConfigParserData& data) :
        ElementParser(),
        m_data(data),
        m_imeAppInfo()
    {}

    ElementParserPtr OnLanguagesElement()
    {
        return ElementParserPtr(new LanguagesParser(m_imeAppInfo));
    }

    ElementParserPtr OnUuidElement()
    {
        return ElementParserPtr(new UuidParser(m_imeAppInfo));
    }

  private:
    ConfigParserData& m_data;
    ConfigParserData::ImeAppInfo m_imeAppInfo;
};
#endif

#ifdef SERVICE_ENABLED
class ServiceAppParser : public ElementParser
{
  public:
    struct ServiceContentParser : public ElementParser
    {
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                            const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create; //ignore unknown according to w3c
        }

        virtual void Accept(const XmlAttribute& attribute)
        {
           if (attribute.name == L"src") {
               m_src = attribute.value;
           }
        }

        virtual void Accept(const Element& element)
        {}

        virtual void Accept(const Text& /*text*/)
        {
            ThrowMsg(Exception::ParseError, "param element must be empty");
        }

        virtual void Verify()
        {
            if (m_src.empty()) {
                ThrowMsg(Exception::ParseError, "src attribute of content element is mandatory");
            }

            if (!m_serviceAppInfo.serviceContent.empty()) {
                ThrowMsg(Exception::ParseError, "content element occurs more than 1 time");
            }
            m_serviceAppInfo.serviceContent = m_src;
        }

        explicit ServiceContentParser(ConfigParserData::ServiceAppInfo& data) :
            ElementParser(),
            m_serviceAppInfo(data)
        {}

      private:
        DPL::String m_src;
        ConfigParserData::ServiceAppInfo& m_serviceAppInfo;
    };

    struct ServiceNameParser : public ElementParser
    {
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                            const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create;
        }

        virtual void Accept(const XmlAttribute& attribute)
        {
            m_lang = attribute.lang;
        }

        virtual void Accept(const Element& element)
        {}

        virtual void Accept(const Text& text)
        {
            m_name = text.value;
        }

        virtual void Verify()
        {
            ConfigParserData::LocalizedData& data = m_serviceAppInfo.m_localizedDataSet[m_lang];
            if (!data.name) {
                if (!!m_name) {
#ifdef ELEMENT_ATTR_MAX_LENGTH
                    NormalizeString(m_name, MAX_ATTR_ELEMENT_LENGTH, true);
#else
                    NormalizeString(m_name);
#endif
                }
                data.name = m_name;
            }
        }

        ServiceNameParser(ConfigParserData::ServiceAppInfo& data) :
            ElementParser(),
            m_serviceAppInfo(data)
        {}

      private:
        DPL::String m_lang;
        DPL::OptionalString m_name;
        ConfigParserData::ServiceAppInfo& m_serviceAppInfo;
    };

    struct ServiceIconParser : public ElementParser
    {
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                            const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create;
        }

        virtual void Accept(const XmlAttribute& attribute)
        {
            if (attribute.name == L"src") {
                m_icon.src = attribute.value;
            }
            else if (attribute.name == L"width") {
                m_icon.width = ParseSizeAttributeValue(attribute.value);
            }
            else if (attribute.name == L"height") {
                m_icon.height = ParseSizeAttributeValue(attribute.value);
            }
        }

        virtual void Accept(const Element& element)
        {}

        virtual void Accept(const Text& /*text*/)
        {
            ThrowMsg(Exception::ParseError, "param element must be empty");
        }

        virtual void Verify()
        {
            if (m_icon.src.empty()) {
                ThrowMsg(Exception::ParseError,
                    "src attribute of icon element is mandatory - ignoring");
            }
             m_serviceAppInfo.m_iconsList.push_back(m_icon);
        }

        explicit ServiceIconParser(ConfigParserData::ServiceAppInfo& data) :
            ElementParser(),
            m_icon(L""),
            m_serviceAppInfo(data)
        {}

    private:
      ConfigParserData::Icon m_icon;
      ConfigParserData::ServiceAppInfo& m_serviceAppInfo;

      static DPL::OptionalInt ParseSizeAttributeValue(const DPL::String& value)
      {
          DPL::OptionalString normalizedValue = value;
          NormalizeString(normalizedValue);
          if (!(*normalizedValue).empty()) {
              char* reterr = NULL;
              errno = 0;
              long int valueInt = strtol(DPL::ToUTF8String(value).c_str(), &reterr, 10);
              if (errno != 0 || std::string(reterr) == DPL::ToUTF8String(value) || valueInt <= 0) {
                  return DPL::OptionalInt();
              } else {
                  return valueInt;
              }
          }
          return DPL::OptionalInt();
      }
    };

    struct ServiceDescriptionParser : public ElementParser
    {
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/, const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create; //ignore unknown according to w3c
        }

        virtual void Accept(const XmlAttribute& attribute)
        {
            m_lang = attribute.lang;
        }

        virtual void Accept(const Element& element)
        {}

        virtual void Accept(const Text& text)
        {
            m_description = text.value;
        }

        virtual void Verify()
        {
            ConfigParserData::LocalizedData& data = m_serviceAppInfo.m_localizedDataSet[m_lang];
            if (!data.description) {
                data.description = m_description;
            }
        }

        ServiceDescriptionParser(ConfigParserData::ServiceAppInfo& data) :
            ElementParser(),
            m_serviceAppInfo(data)
        {}

    private:
        DPL::String m_lang;
        DPL::OptionalString m_description;
        ConfigParserData::ServiceAppInfo& m_serviceAppInfo;
    };

    struct ServiceMetadataParser : public ElementParser
    {
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/, const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create; //ignore unknown according to w3c
        }

        virtual void Accept(const XmlAttribute& attribute)
        {
            if (attribute.name == L"key") {
                m_key = attribute.value;
            } else if (attribute.name == L"value") {
                m_value = attribute.value;
            }
        }

        virtual void Accept(const Element& element)
        {}

        virtual void Accept(const Text& text)
        {
            ThrowMsg(Exception::ParseError, "param element must be empty");
        }

        virtual void Verify()
        {
            if (!m_key) {
            _W("metadata element must have key attribute");
            return;
            }
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
            NormalizeString(m_key, MAX_NAME_KEY_LENGTH);
            NormalizeString(m_value, MAX_NAME_KEY_VALUE_LENGTH);
#else
            NormalizeString(m_key);
            NormalizeString(m_value);
#endif
            ConfigParserData::Metadata metaData(m_key, m_value);
            FOREACH(it, m_serviceAppInfo.m_metadataList) {
                if (!DPL::StringCompare(*it->key, *m_key)) {
                    _E("Key isn't unique");
                    return;
                }
            }
            m_serviceAppInfo.m_metadataList.push_back(metaData);
        }

        ServiceMetadataParser(ConfigParserData::ServiceAppInfo& data) :
            ElementParser(),
            m_serviceAppInfo(data)
        {}

      private:
        DPL::OptionalString m_key;
        DPL::OptionalString m_value;
        ConfigParserData::ServiceAppInfo& m_serviceAppInfo;
    };

    struct ServiceCategoryParser : public ElementParser
    {
        virtual ActionFunc GetElementParser(const DPL::String& /*ns*/, const DPL::String& /*name*/)
        {
            return &IgnoringParser::Create; //ignore unknown according to w3c
        }

        virtual void Accept(const XmlAttribute& attribute)
        {
            if (attribute.name == L"name") {
                if (attribute.value.size() > 0) {
                    m_name = attribute.value;
                }
            }
        }

        virtual void Accept(const Element& element)
        {}

        virtual void Accept(const Text& text)
        {
            ThrowMsg(Exception::ParseError, "param element must be empty");
        }

        virtual void Verify()
        {
            if (!m_name) {
                _W("name attribute of category element is mandatory - ignoring");
                return;
            }

            if (m_serviceAppInfo.m_categoryList.find(*m_name) == m_serviceAppInfo.m_categoryList.end()) {
                m_serviceAppInfo.m_categoryList.insert(*m_name);
            }
        }

        ServiceCategoryParser(ConfigParserData::ServiceAppInfo& data) :
            ElementParser(),
            m_serviceAppInfo(data)
        {}

      private:
        DPL::OptionalString m_name;
        ConfigParserData::ServiceAppInfo& m_serviceAppInfo;
    };

    virtual ActionFunc GetElementParser(const DPL::String& /*ns*/,
                                        const DPL::String& name)
    {
        if (name == L"content") {
            return std::bind(&ServiceAppParser::OnServiceContentElement, this);
        } else if (name == L"name") {
            return std::bind(&ServiceAppParser::OnServiceNameElement, this);
        } else if (name == L"icon") {
            return std::bind(&ServiceAppParser::OnServiceIconElement, this);
        } else if (name == L"description") {
            return std::bind(&ServiceAppParser::OnServiceDescriptionElement, this);
        } else if (name == L"metadata") {
            return std::bind(&ServiceAppParser::OnServiceMetadataElement, this);
        } else if (name == L"category") {
            return std::bind(&ServiceAppParser::OnServiceCategoryElement, this);
        } else {
            return &IgnoringParser::Create;
        }
    }

    virtual void Accept(const Element& element)
    {}

    virtual void Accept(const XmlAttribute& attribute)
    {
        if (attribute.name == L"id") {
            if (attribute.value.size() > 0) {
                m_serviceId = attribute.value;
            }
        } else if (attribute.name == L"auto-restart") {
            if (attribute.value == L"true") {
                m_autoRestart = true;
            } else if (attribute.value == L"false") {
                m_autoRestart = false;
            } else {
                ThrowMsg(Exception::ParseError, "Wrong boolean value");
            }
        } else if (attribute.name == L"on-boot") {
            if (attribute.value == L"true") {
                m_onBoot = true;
            } else if (attribute.value == L"false") {
                m_onBoot = false;
            } else {
                ThrowMsg(Exception::ParseError, "Wrong boolean value");
            }
        }
    }

    virtual void Accept(const Text& /*text*/)
    {
        ThrowMsg(Exception::ParseError, "param element must be empty");
    }

    virtual void Verify()
    {
        FOREACH(it, m_data.settingsList) {
            if (it->m_name == L"encryption" && it->m_value == L"enable") {
                ThrowMsg(Exception::ParseError, "Service application does not support application encryption");
            }
        }

        if (m_serviceAppInfo.serviceContent.empty()) {
            ThrowMsg(Exception::ParseError, "service element must have content element");
        }

        if (m_serviceId.empty()) {
            ThrowMsg(Exception::ParseError, "service attribute must have id attribute");
        }

        m_serviceAppInfo.serviceId = m_serviceId;

        m_serviceAppInfo.autoRestart = m_autoRestart;

        m_serviceAppInfo.onBoot = m_onBoot;

        m_data.serviceAppInfoList.push_back(m_serviceAppInfo);
    }

    ServiceAppParser(ConfigParserData& data) :
        ElementParser(),
        m_data(data),
        m_serviceAppInfo(),
        m_onBoot(false),
        m_autoRestart(false)
    {}

    ElementParserPtr OnServiceContentElement()
    {
        return ElementParserPtr(new ServiceContentParser(m_serviceAppInfo));
    }

    ElementParserPtr OnServiceNameElement()
    {
        return ElementParserPtr(new ServiceNameParser(m_serviceAppInfo));
    }

    ElementParserPtr OnServiceIconElement()
    {
        return ElementParserPtr(new ServiceIconParser(m_serviceAppInfo));
    }

    ElementParserPtr OnServiceDescriptionElement()
    {
        return ElementParserPtr(new ServiceDescriptionParser(m_serviceAppInfo));
    }

    ElementParserPtr OnServiceMetadataElement()
    {
        return ElementParserPtr(new ServiceMetadataParser(m_serviceAppInfo));
    }

    ElementParserPtr OnServiceCategoryElement()
    {
        return ElementParserPtr(new ServiceCategoryParser(m_serviceAppInfo));
    }

  private:
    bool m_autoRestart;
    bool m_onBoot;
    DPL::String m_serviceId;
    ConfigParserData& m_data;
    ConfigParserData::ServiceAppInfo m_serviceAppInfo;
};
#endif

ElementParser::ActionFunc WidgetParser::GetElementParser(
    const DPL::String& /*ns*/,
    const DPL::String& name)
{
    FuncMap::const_iterator it = m_map.find(name);
    if (it != m_map.end()) {
        return it->second;
    } else {
        return &IgnoringParser::Create; //ignore unknown according to w3c
    }
}

WidgetParser::WidgetParser(ConfigParserData& data) :
    m_data(data),
    m_textDirection(Unicode::EMPTY)
{
    m_map[L"name"] = std::bind(&WidgetParser::OnNameElement, this);
    m_map[L"access"] = std::bind(&WidgetParser::OnAccessElement, this);
    m_map[L"description"] = std::bind(&WidgetParser::OnDescriptionElement, this);
    m_map[L"author"] = std::bind(&WidgetParser::OnAuthorElement, this);
    m_map[L"license"] = std::bind(&WidgetParser::OnLicenseElement, this);
    m_map[L"icon"] = std::bind(&WidgetParser::OnIconElement, this);
    m_map[L"small-icon"] = std::bind(&WidgetParser::OnSmallIconElement, this);
    m_map[L"content"] = std::bind(&WidgetParser::OnContentElement, this);
    m_map[L"preference"] = std::bind(&WidgetParser::OnPreferenceElement, this);
    m_map[L"setting"] = std::bind(&WidgetParser::OnSettingElement, this);
    m_map[L"application"] = std::bind(&WidgetParser::OnApplicationElement, this);
#ifdef IME_ENABLED
    m_map[L"ime"] = std::bind(&WidgetParser::OnImeElement, this);
#endif
#ifdef SERVICE_ENABLED
    m_map[L"service"] = std::bind(&WidgetParser::OnServiceAppElement, this);
#endif
    m_map[L"splash"] = std::bind(&WidgetParser::OnSplashElement, this);
    m_map[L"background"] = std::bind(&WidgetParser::OnBackgroundElement, this);
    m_map[L"privilege"] = std::bind(&WidgetParser::OnPrivilegeElement, this);
    m_map[L"app-control"] = std::bind(&WidgetParser::OnAppControlElement, this);
    m_map[L"category"] = std::bind(&WidgetParser::OnCategoryElement, this);
#ifdef DBOX_ENABLED
    m_map[L"app-widget"] = std::bind(&WidgetParser::OnAppWidgetElement, this);
#endif
#if ENABLE(CONTENT_SECURITY_POLICY)
    m_map[L"content-security-policy"] = std::bind(&WidgetParser::OnCspElement, this);
    m_map[L"content-security-policy-report-only"] = std::bind(&WidgetParser::OnCspReportOnlyElement, this);
#endif
#if ENABLE(ALLOW_NAVIGATION)
    m_map[L"allow-navigation"] = std::bind(&WidgetParser::OnAllowNavigationElement, this);
#endif
    m_map[L"account"] = std::bind(&WidgetParser::OnAccountElement, this);
    m_map[L"metadata"] = std::bind(&WidgetParser::OnMetadataElement, this);
}

ElementParserPtr WidgetParser::OnNameElement()
{
    return ElementParserPtr(new NameParser(m_textDirection, m_data));
}

ElementParserPtr WidgetParser::OnAccessElement()
{
    return ElementParserPtr(new AccessParser(m_data));
}

ElementParserPtr WidgetParser::OnDescriptionElement()
{
    return ElementParserPtr(new DescriptionParser(m_textDirection, m_data));
}

ElementParserPtr WidgetParser::OnAuthorElement()
{
    return ElementParserPtr(new AuthorParser(m_textDirection, m_data));
}

ElementParserPtr WidgetParser::OnLicenseElement()
{
    return ElementParserPtr(new LicenseParser(m_textDirection, m_data));
}

ElementParserPtr WidgetParser::OnIconElement()
{
    return ElementParserPtr(new IconParser(m_data));
}

ElementParserPtr WidgetParser::OnSmallIconElement()
{
    return ElementParserPtr(new IconParser(m_data, true));
}

ElementParserPtr WidgetParser::OnContentElement()
{
    return ElementParserPtr(new ContentParser(m_data));
}

ElementParserPtr WidgetParser::OnPreferenceElement()
{
    return ElementParserPtr(new PreferenceParser(m_data));
}

ElementParserPtr WidgetParser::OnSettingElement()
{
    return ElementParserPtr(new SettingParser(m_data));
}

ElementParserPtr WidgetParser::OnApplicationElement()
{
    return ElementParserPtr(new ApplicationParser(m_data));
}

ElementParserPtr WidgetParser::OnSplashElement()
{
    return ElementParserPtr(new SplashParser(m_data));
}

ElementParserPtr WidgetParser::OnBackgroundElement()
{
    return ElementParserPtr(new BackgroundParser(m_data));
}

ElementParserPtr WidgetParser::OnPrivilegeElement()
{
    return ElementParserPtr(new PrivilegeParser(m_data));
}

ElementParserPtr WidgetParser::OnAppControlElement()
{
    return ElementParserPtr(new AppControlParser(m_data));
}

ElementParserPtr WidgetParser::OnCategoryElement()
{
    return ElementParserPtr(new CategoryParser(m_data));
}

#ifdef DBOX_ENABLED
ElementParserPtr WidgetParser::OnAppWidgetElement()
{
    return ElementParserPtr(new AppWidgetParser(m_data));
}
#endif

ElementParserPtr WidgetParser::OnCspElement()
{
    return ElementParserPtr(new CspParser(m_data));
}

ElementParserPtr WidgetParser::OnCspReportOnlyElement()
{
    return ElementParserPtr(new CspReportOnlyParser(m_data));
}

ElementParserPtr WidgetParser::OnAllowNavigationElement()
{
    return ElementParserPtr(new AllowNavigationParser(m_data));
}

ElementParserPtr WidgetParser::OnAccountElement()
{
    return ElementParserPtr(new AccountParser(m_data));
}

ElementParserPtr WidgetParser::OnMetadataElement()
{
    return ElementParserPtr(new MetadataParser(m_data));
}

#ifdef IME_ENABLED
ElementParserPtr WidgetParser::OnImeElement()
{
    return ElementParserPtr(new ImeParser(m_data));
}
#endif

#ifdef SERVICE_ENABLED
ElementParserPtr WidgetParser::OnServiceAppElement()
{
    return ElementParserPtr(new ServiceAppParser(m_data));
}
#endif

void WidgetParser::Accept(const Element& element)
{
    if (element.ns != ConfigurationNamespace::W3CWidgetNamespaceName &&
        element.ns != ConfigurationNamespace::TizenWebAppNamespaceName)
    {
        ThrowMsg(Exception::ParseError,
                 "Wrong xml namespace for widget element");
    }
}

void WidgetParser::Accept(const Text& /*text*/)
{
    ThrowMsg(Exception::ParseError, "widged element must be empty");
}

void WidgetParser::Accept(const XmlAttribute& attribute)
{
    if (attribute.name == L"id") {
        LibIri::Wrapper iri(DPL::ToUTF8String(attribute.value).c_str());
        //If may important tests starts to fail this test we will have
        //to consider commenting this test out again.
        if (iri.Validate()) {
            m_widgetId = attribute.value;
        } else {
            _W("Widget id validation failed: %ls", attribute.value.c_str());
        }
    } else if (attribute.name == L"version") {
        m_version = attribute.value;
    } else if (attribute.name == L"min-version") {
        _D("min-version attribute was found. Value: %ls", attribute.value.c_str());
        m_minVersion = attribute.value;
        m_data.minVersionRequired = m_minVersion;
    } else if (attribute.name == L"height") {
        DPL::OptionalString value = attribute.value;
        if (!(*value).empty()) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
            NormalizeString(value, MAX_ATTR_ELEMENT_LENGTH, true);
#else
            NormalizeString(value);
#endif
            std::string v = DPL::ToUTF8String(*value);
            unsigned char c = v.c_str()[0];
            if (isdigit(c)) {
                int val = 0;
                for (size_t i = 0; i < v.size(); ++i) {
                    c = v.c_str()[i];
                    if (isdigit(c)) {
                        val *= 10;
                        val += (c - '0');
                    } else {
                        break;
                    }
                }
                m_data.height = val;
            }
        }
    } else if (attribute.name == L"width") {
        DPL::OptionalString value = attribute.value;
        if (!(*value).empty()) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
            NormalizeString(value, MAX_ATTR_ELEMENT_LENGTH, true);
#else
            NormalizeString(value);
#endif
            std::string v = DPL::ToUTF8String(*value);
            unsigned char c = v.c_str()[0];
            if (c >= '0' && c <= '9') {
                int val = 0;
                for (size_t i = 0; i < v.size(); ++i) {
                    c = v.c_str()[i];
                    if (c >= '0' && c <= '9') {
                        val *= 10;
                        val += (c - '0');
                    } else {
                        break;
                    }
                }
                m_data.width = val;
            }
        }
    } else if (attribute.name == L"viewmodes") {
        DPL::Tokenize(attribute.value,
                      L" ",
                      std::inserter(m_windowModes,
                                    m_windowModes.end()),
                      true);
    } else if (attribute.name == L"dir") {
        m_textDirection = Unicode::ParseDirAttribute(attribute);
    } else if (L"defaultlocale" == attribute.name) {
        if (!m_defaultlocale) {
            m_defaultlocale = attribute.value;
        } else {
            _W("Ignoring subsequent default locale");
        }
        //Any other value consider as a namespace definition
    } else if (attribute.name == L"xmlns" || attribute.prefix == L"xmlns") {
        _D("Namespace domain: %ls", attribute.name.c_str());
        _D("Namespace value: %ls", attribute.value.c_str());
        m_nameSpaces[attribute.name] = attribute.value;
    } else {
        _E("Unknown attirbute: namespace=%ls, name=%ls, value=%ls",
            attribute.ns.c_str(), attribute.name.c_str(), attribute.value.c_str());
    }
}

void WidgetParser::Verify()
{
    FOREACH(mode, m_windowModes) {
        if (L"windowed" == *mode || L"floating" == *mode ||
            L"fullscreen" == *mode || L"maximized" == *mode ||
            L"minimized" == *mode)
        {
            m_data.windowModes.insert(*mode);
        }
    }
    if (!!m_widgetId) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
        NormalizeString(m_widgetId, MAX_ATTR_ELEMENT_LENGTH);
#else
        NormalizeString(m_widgetId);
#endif
        m_data.widget_id = m_widgetId;
    }
    if (!!m_version) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
        NormalizeString(m_version, MAX_ATTR_ELEMENT_LENGTH, true);
#else
        NormalizeString(m_version);
#endif
        Unicode::UpdateTextWithDirectionMark(m_textDirection, &*m_version);
        m_data.version = m_version;
    }
    if (!!m_minVersion) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
        NormalizeString(m_minVersion, MAX_ATTR_ELEMENT_LENGTH);
#else
        NormalizeString(m_minVersion);
#endif
        m_data.minVersionRequired = m_minVersion;
    }
    if (!!m_defaultlocale) {
#if ENABLE(ELEMENT_ATTR_MAX_LENGTH)
        NormalizeString(m_defaultlocale, MAX_ATTR_ELEMENT_LENGTH);
#else
        NormalizeString(m_defaultlocale);
#endif
        std::string dl = DPL::ToUTF8String(*m_defaultlocale);

        if (!LanguageSubtagRstTreeSingleton::Instance().
                ValidateLanguageTag(dl)) {
            _W("Language tag: %s is not valid", dl.c_str());
            m_defaultlocale = DPL::OptionalString();
        } else {
            _D("Default locale found %s", dl.c_str());
        }
        m_data.defaultlocale = m_defaultlocale;
    }
    FOREACH(ns, m_nameSpaces) {
        m_data.nameSpaces.insert(ns->second);
    }
}
