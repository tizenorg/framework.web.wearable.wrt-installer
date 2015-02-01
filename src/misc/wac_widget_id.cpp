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
/*
 * @file
 * @author      Bartlomiej Grzelewski (b.grzelewski@samsung.com)
 * @version     1.0
 * @brief
 */
#include "wac_widget_id.h"

#include <memory>
#include <string>

#include <dpl/string.h>

#include <iri.h>
#include <installer_log.h>

namespace {
const char *SCHEME_HTTP = "http";
const char *SCHEME_HTTPS = "https";
}

WacWidgetId::WacWidgetId(const DPL::OptionalString &widgetId) :
    m_schemaMatch(false)
{
    if (!!widgetId) {
        std::string wid = DPL::ToUTF8String(*widgetId);
        parse(wid.c_str());
    }
}

bool WacWidgetId::matchHost(const DPL::String &second) const
{
    _D("m_schemaMatch is: %d", m_schemaMatch);
    if (!m_schemaMatch) {
        return false;
    }

    _D("Matching DNS identity: %s %ls", m_host.c_str(), second.c_str());

    return m_host == DPL::ToUTF8String(second);
}

void WacWidgetId::parse(const char *url)
{
    _D("Widget id to parse: %s", url);

    std::unique_ptr<iri_struct, std::function<void(iri_struct*)> >
    iri(iri_parse(url), iri_destroy);

    if (!iri.get()) {
        _E("Error in parsing widget id.");
        return; // m_schemaMatch == false;
    }

    std::string scheme;

    if (iri.get()->scheme) {
        scheme = iri.get()->scheme;
    } else {
        _W("Error. No scheme in widget id.");
        return; // m_schemaMatch == false;
    }

    // should we support HTTP and HTTPS? wac says nothing
    // std::transform(m_scheme.begin(), m_scheme.end(), m_scheme.begin(),
    // tolower);

    // We only match "http" and "https" schemas
    if ((scheme != SCHEME_HTTP) && (scheme != SCHEME_HTTPS)) {
        _W("Unknown scheme in widget id. %s", scheme.c_str());
        return; // m_schemaMatch == false;
    } else {
        m_schemaMatch = true;
    }

    if (iri.get()->host) {
        m_host = iri.get()->host;
        _D("Host has been set to: %s", m_host.c_str());
    }

    // What to do when host is empty? No info in wac documentation.

    // Any post processing algorithm? No info in wac documentation.
}
