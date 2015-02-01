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
 * @file    widget_security.h
 * @author  Krzysztof Jackiewicz(k.jackiewicz@samsung.com)
 * @version 1.0
 * @brief
 */

#ifndef WACSECURITY_H_
#define WACSECURITY_H_

#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <vcore/Certificate.h>
#include <vcore/CertificateCollection.h>

namespace Jobs {
namespace WidgetInstall {
class WidgetSecurity : public WrtDB::IWidgetSecurity
{
  public:
    WidgetSecurity() :
        mRecognized(false),
        mDistributorSigned(false)
    {}

    // from IWidgetSecurity
    virtual const WrtDB::WidgetCertificateDataList& getCertificateList() const
    {
        return mCertificateList;
    }

    virtual bool isRecognized() const
    {
        return mRecognized;
    }

    virtual bool isDistributorSigned() const
    {
        return mDistributorSigned;
    }

    virtual void getCertificateChainList(
        WrtDB::CertificateChainList& list,
        WrtDB::CertificateSource source) const;

    void setRecognized(bool recognized)
    {
        mRecognized = recognized;
    }
    void setDistributorSigned(bool distributorSigned)
    {
        mDistributorSigned = distributorSigned;
    }
    void setAuthorCertificatePtr(ValidationCore::CertificatePtr certPtr)
    {
        mAuthorCertificate = certPtr;
    }

    ValidationCore::CertificatePtr getAuthorCertificatePtr() const
    {
        return mAuthorCertificate;
    }

    ValidationCore::CertificateCollectionList& getCertificateChainListRef()
    {
        return mCertificateChainList;
    }

    ValidationCore::CertificateCollectionList& getCertificateChainList2Ref()
    {
        return mCertificateChainList2;
    }

    ValidationCore::CertificateCollectionList&
    getAuthorsCertificateChainListRef()
    {
        return mAuthorsCertificateChainList;
    }

    WrtDB::WidgetCertificateDataList& getCertificateListRef()
    {
        return mCertificateList;
    }

  private:
    // This data are used to evaluate policy
    WrtDB::WidgetCertificateDataList mCertificateList;

    // author signature verified
    bool mRecognized;
    // known distribuor
    bool mDistributorSigned;
    // Author end entity certificate.
    // Information from this certificate are shown to user
    // during installation process.
    ValidationCore::CertificatePtr mAuthorCertificate;
    // This certificates are used by OCSP/CRL
    ValidationCore::CertificateCollectionList mCertificateChainList;
    // This certificates are for distributor2
    ValidationCore::CertificateCollectionList mCertificateChainList2;
    // This authors certificates are used by tizen
    ValidationCore::CertificateCollectionList mAuthorsCertificateChainList;
};
} // namespace WidgetInstall
} // namespace Jobs

#endif /* WACSECURITY_H_ */
