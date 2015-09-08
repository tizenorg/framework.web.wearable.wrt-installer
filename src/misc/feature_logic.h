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

#ifndef SRC_INSTALLER_MISC_FEATURE_LOGIC
#define SRC_INSTALLER_MISC_FEATURE_LOGIC

#include <list>
#include <string>

#include <dpl/assert.h>
#include <dpl/noncopyable.h>
#include <memory>

#include <dpl/wrt-dao-ro/feature_dao_read_only.h>
#include <wrt_common_types.h>

namespace Jobs {
namespace WidgetInstall {
class FeatureLogic : DPL::Noncopyable
{
  public:

    FeatureLogic(const WrtDB::TizenAppId & tzAppid);

    bool isDone() const;

    bool next();

    void setAceResponse(bool allowed);

    DPL::String getDevice() const;

    bool isRejected(void) const
    {
        return m_rejected;
    }

    struct Feature : public WidgetFeature {
        WrtDB::DeviceCapabilitySet devCapSet;
        WrtDB::DeviceCapabilitySet::const_iterator currentCap;

        Feature(const WidgetFeature &wf,
                const WrtDB::DeviceCapabilitySet &set) :
            WidgetFeature(wf)
            , devCapSet(set)
        {
            currentCap = devCapSet.begin();
        }

        explicit Feature(const Feature &second) : WidgetFeature(second)
        {
            devCapSet = second.devCapSet;
            currentCap = devCapSet.find(*second.currentCap);
            rejected = second.rejected;
        }

      private:
        void operator=(const Feature &second)
        {
            name = second.name;
            devCapSet = second.devCapSet;
            rejected = second.rejected;
            pluginId = second.pluginId;
            currentCap = devCapSet.find(*second.currentCap);
        }
    };

    typedef std::list<Feature> FeatureList;
    typedef FeatureList::const_iterator FeatureIterator;

    FeatureIterator resultBegin()
    {
        return m_featureList.begin();
    }
    FeatureIterator resultEnd()
    {
        return m_featureList.end();
    }

  private:
    bool isProcessable() const;

    FeatureList m_featureList;
    FeatureList::iterator m_currentFeature;
    bool m_rejected;
};

typedef std::shared_ptr<FeatureLogic> FeatureLogicPtr;
} // namespace WidgetInstall
} // namespace Jobs

#endif // SRC_INSTALLER_MISC_FEATURE_LOGIC
