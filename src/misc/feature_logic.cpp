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

#include "feature_logic.h"

#include <list>

#include <dpl/assert.h>
#include <dpl/noncopyable.h>
#include <dpl/string.h>
#include <dpl/foreach.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/wrt-dao-ro/feature_dao_read_only.h>
#include <installer_log.h>

namespace Jobs {
namespace WidgetInstall {
namespace {
const DPL::String PRIVILEGE_TESTAUTOMATION =
    L"http://tizen.org/privilege/testautomation";
const DPL::String DEVICE_CAPABILITY_TESTAUTOMATION = L"testautomation";
}
FeatureLogic::FeatureLogic(const WrtDB::TizenAppId & tzAppid) :
    m_rejected(false)
{
    WrtDB::WidgetDAOReadOnly widgetDao(tzAppid);
    WidgetFeatureSet featureSet = widgetDao.getFeaturesList();
    FOREACH(it, featureSet) {
        _D("Feature name : %ls", it->name.c_str());
        WrtDB::DeviceCapabilitySet dcs;
        if (!DPL::StringCompare(it->name, PRIVILEGE_TESTAUTOMATION)) {
            // special privilege
            // This privilege doesn't have plugin in the target
            // only use to special behavior
            dcs.insert(DEVICE_CAPABILITY_TESTAUTOMATION);
        } else {
            // normal privilege
            dcs = WrtDB::FeatureDAOReadOnly::GetDeviceCapability(it->name);
        }
        FOREACH(devCap, dcs) {
            _D("--- dev cap  : %ls", (*devCap).c_str());
        }
        Feature feature(*it, dcs);
        m_featureList.push_back(feature);
    }
    m_currentFeature = m_featureList.begin();

    // ok we must set iterator on the first processable node
    if (!isProcessable()) {
        next();
    }
}

bool FeatureLogic::isDone() const
{
    return m_currentFeature == m_featureList.end();
}

bool FeatureLogic::next()
{
    while (!isDone()) {
        if (m_currentFeature->currentCap !=
            m_currentFeature->devCapSet.end())
        {
            m_currentFeature->currentCap++;
        } else {
            ++m_currentFeature;
        }
        // we moved pointer
        if (isProcessable()) {
            return true;
        }
    }
    return false;
}

void FeatureLogic::setAceResponse(bool allowed)
{
    AssertMsg(isProcessable(), "Wrong usage");
    if (!allowed) {
        m_currentFeature->rejected = true;
        m_rejected = true;
    }
}

DPL::String FeatureLogic::getDevice() const
{
    return *(m_currentFeature->currentCap);
}

bool FeatureLogic::isProcessable() const
{
    if (isDone()) {
        return false;
    }

    if (m_currentFeature->currentCap == m_currentFeature->devCapSet.end()) {
        return false;
    }

    return true;
}
} // namespace WidgetInstall
} // namespace Jobs

