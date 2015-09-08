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
 * @file        libiriwrapper.cpp
 * @author      Piotr Marcinkiewicz (p.marcinkiew@samsung.com
 * @version     0.1
 * @brief       Libiri parser wrapper
 */
#include <stdlib.h>
#include <iri.h>
#include "libiriwrapper.h"

//TODO: Design and implement new IRI manager class

namespace LibIri {
Wrapper::Wrapper(const char* aIri) : m_Iri(iri_parse(aIri))
{}
Wrapper::~Wrapper()
{
    iri_destroy(m_Iri);
}
//! \brief Returns true if iri is valid
bool Wrapper::Validate()
{
    return
        m_Iri != NULL &&
        m_Iri->scheme != NULL && (
            m_Iri->display != NULL ||
            m_Iri->user != NULL ||
            m_Iri->auth != NULL ||
            m_Iri->password != NULL ||
            m_Iri->host != NULL ||
            m_Iri->path != NULL ||
            m_Iri->query != NULL ||
            m_Iri->anchor != NULL ||
            m_Iri->qparams != NULL ||
            m_Iri->schemelist != NULL);
}

std::ostream & operator<<(std::ostream& a_stream,
                          const Wrapper& a_wrapper)
{
    iri_t& iri = *a_wrapper.m_Iri;
#define PRINT_FIELD(field) "] " #field " [" << (iri.field ? iri.field : "null")
    a_stream <<
    " display [" << (iri.display ? iri.display : "null") <<
    PRINT_FIELD(scheme) <<
    PRINT_FIELD(user) <<
    PRINT_FIELD(auth) <<
    PRINT_FIELD(password) <<
    PRINT_FIELD(host) <<
    "] port [" << (iri.port ? iri.port : -1) <<
    PRINT_FIELD(path) <<
    PRINT_FIELD(query) <<
    PRINT_FIELD(anchor) <<
    "]";
    return a_stream;
#undef PRINT_FIELD
}
} //namespace LibIri
