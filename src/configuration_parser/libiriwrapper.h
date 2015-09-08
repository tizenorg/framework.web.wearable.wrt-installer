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

#ifndef _WRT_ENGINE_SRC_INSTALLERCORE_CONFIGURATION_PARSER_LIBIRIWRAPPER_H_
#define _WRT_ENGINE_SRC_INSTALLERCORE_CONFIGURATION_PARSER_LIBIRIWRAPPER_H_

#include <iostream>
#include <iri.h>

//TODO: Design and implement new IRI manager class
//
namespace LibIri {
struct Wrapper
{
    Wrapper(const char* aIri);
    ~Wrapper();
    iri_t *m_Iri;
    //! \brief Returns true if iri is valid
    bool Validate();
};

std::ostream & operator<<(std::ostream& a_stream,
                          const Wrapper& a_wrapper);
} //namespace LibIri

#endif // _WRT_ENGINE_SRC_INSTALLERCORE_CONFIGURATION_PARSER_LIBIRIWRAPPER_H_

