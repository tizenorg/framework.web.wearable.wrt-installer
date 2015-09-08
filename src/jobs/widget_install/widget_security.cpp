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
 * @file    widget_security.cpp
 * @author  Krzysztof Jackiewicz(k.jackiewicz@samsung.com)
 * @version 1.0
 * @brief
 */

#include "widget_security.h"
#include <dpl/foreach.h>
#include <installer_log.h>

namespace Jobs {
namespace WidgetInstall {
void WidgetSecurity::getCertificateChainList(
    WrtDB::CertificateChainList& list,
    WrtDB::CertificateSource source) const
{
    if (source == WrtDB::CertificateSource::SIGNATURE_AUTHOR) {
        FOREACH(certIter, mAuthorsCertificateChainList)
        list.push_back(certIter->toBase64String());
    } else if (source == WrtDB::CertificateSource::SIGNATURE_DISTRIBUTOR) {
        FOREACH(certIter, mCertificateChainList)
        list.push_back(certIter->toBase64String());
    } else if (source == WrtDB::CertificateSource::SIGNATURE_DISTRIBUTOR2) {
        FOREACH(certIter, mCertificateChainList2)
        list.push_back(certIter->toBase64String());
    } else {
        _E("UNKNOWN certificate");
    }
}
} // namespace WidgetInstall
} // namespace Jobs
