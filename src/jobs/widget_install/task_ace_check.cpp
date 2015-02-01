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
 * @file    task_ace_check.cpp
 * @author  Pawel Sikorski (p.sikorski@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task ace check
 */

#include <utility>
#include <vector>
#include <string>

#include <widget_install/task_ace_check.h>
#include <dpl/assert.h>
#include <dpl/foreach.h>

#include <widget_install/widget_install_context.h>
#include <widget_install/widget_install_errors.h>
#include <widget_install/job_widget_install.h>

#include <dpl/wrt-dao-rw/widget_dao.h>
#include <ace_api_install.h>

#include <installer_log.h>

namespace Jobs {
namespace WidgetInstall {
TaskAceCheck::TaskAceCheck(InstallerContext& context) :
    DPL::TaskDecl<TaskAceCheck>(this),
    m_context(context)
{
    AddStep(&TaskAceCheck::StartStep);
    AddStep(&TaskAceCheck::StepPrepareForAce);
    AddStep(&TaskAceCheck::StepAceCheck);
    AddStep(&TaskAceCheck::StepProcessAceResponse);
    AddStep(&TaskAceCheck::StepCheckAceResponse);
    AddStep(&TaskAceCheck::EndStep);
}

void TaskAceCheck::StepPrepareForAce()
{
    m_context.featureLogic =
        FeatureLogicPtr(new FeatureLogic(m_context.widgetConfig.tzAppid));
    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_ACE_PREPARE,
        "Widget Access Control Check Prepared");
}

void TaskAceCheck::StepAceCheck()
{
    WrtDB::WidgetDAO dao(m_context.widgetConfig.tzAppid);
    _D("StepAceCheck!");
    // This widget does not use any device cap
    if (m_context.featureLogic->isDone()) {
        return;
    }

    _D("StepAceCheck!");
    DPL::String deviceCap = m_context.featureLogic->getDevice();

    _D("StepAceCheck!");
    _D("DevCap is : %ls", deviceCap.c_str());

    std::string devCapStr = DPL::ToUTF8String(deviceCap);
    ace_policy_result_t policyResult = ACE_DENY;

    if (m_context.mode.installTime == InstallMode::InstallTime::PRELOAD ||
        m_context.mode.installTime == InstallMode::InstallTime::FOTA) {
        _D("This widget is preloaded. So ace check will be skiped");
        policyResult = ACE_PERMIT;
    } else {
        ace_return_t ret = ace_get_policy_result(
                const_cast<const ace_resource_t>(devCapStr.c_str()),
                dao.getHandle(),
                &policyResult);
        if (ACE_OK != ret) {
            ThrowMsg(Exceptions::AceCheckFailed, "Instalation failure. "
                    "ACE check failure");
        }
    }

    _D("PolicyResult is : %d", static_cast<int>(policyResult));
    m_context.staticPermittedDevCaps.insert(std::make_pair(deviceCap,
                                                           policyResult ==
                                                           ACE_PERMIT));

    m_context.featureLogic->setAceResponse(policyResult != ACE_DENY);
}

void TaskAceCheck::StepProcessAceResponse()
{
    if (m_context.widgetConfig.packagingType ==
        WrtDB::PKG_TYPE_HOSTED_WEB_APP)
    {
        return;
    }

    _D("StepProcessAceResponse");
    m_context.featureLogic->next();

    // No device caps left to process
    if (m_context.featureLogic->isDone()) {
        WrtDB::WidgetDAO dao(m_context.widgetConfig.tzAppid);
#ifdef SERVICE_ENABLED
        std::list<WrtDB::DbWidgetHandle> serviceList;
        FOREACH( it , m_context.widgetConfig.configInfo.serviceAppInfoList ){
             WrtDB::WidgetDAO serviceDao(it->serviceId);
             serviceList.push_back(serviceDao.getHandle());
        }
#endif
        _D("All responses has been received from ACE.");
        // Data to convert to C API
        std::vector<std::string> devCaps;
        std::vector<bool> devCapsSmack;
        // Saving static dev cap permissions
        FOREACH(cap, m_context.staticPermittedDevCaps) {
            _D("staticPermittedDevCaps : %ls smack: %d", cap->first.c_str(), cap->second);
            std::string devCapStr = DPL::ToUTF8String(cap->first);
            devCaps.push_back(devCapStr);
            devCapsSmack.push_back(cap->second);
        }
        ace_requested_dev_cap_list_t list;
        list.count = devCaps.size();
        list.items = new ace_requested_dev_cap_t[list.count];

        for (unsigned int i = 0; i < devCaps.size(); ++i) {
            list.items[i].device_capability =
                const_cast<const ace_resource_t>(devCaps[i].c_str());
            list.items[i].smack_granted =
                devCapsSmack[i] ? ACE_TRUE : ACE_FALSE;
        }
        //TODO: remove dao.getHandle()
        int ret = ace_set_requested_dev_caps(dao.getHandle(),&list);
#ifdef SERVICE_ENABLED
        FOREACH( it, serviceList){
            ret |= ace_set_requested_dev_caps(*it,&list);
        }
#endif
        delete[] list.items;

        if (ACE_OK != static_cast<ace_return_t>(ret)) {
            ThrowMsg(Exceptions::AceCheckFailed, "Instalation failure. "
                                             "ACE failure");
        }

        std::set<std::string> acceptedFeature;
        auto it = m_context.featureLogic->resultBegin();
        for (; it != m_context.featureLogic->resultEnd(); ++it) {
            if (!(it->rejected)) {
                acceptedFeature.insert(DPL::ToUTF8String(it->name));
            }
        }
        ace_feature_list_t featureList;
        featureList.count = acceptedFeature.size();
        featureList.items = new ace_string_t[featureList.count];

        size_t i = 0;
        for (std::set<std::string>::const_iterator iter = acceptedFeature.begin();
             iter != acceptedFeature.end(); ++iter)
        {
            _D("Accepted feature item: %s", iter->c_str());
            featureList.items[i] = const_cast<char *>(iter->c_str());
            i++;
        }

        //TODO: remove dao.getHandle()
        ret = ace_set_accepted_feature(dao.getHandle(), &featureList);
#ifdef SERVICE_ENABLED
        FOREACH( it, serviceList){
            ret |= ace_set_accepted_feature(*it, &featureList);
        }
#endif
        delete[] featureList.items;

        if (ACE_OK != static_cast<ace_return_t>(ret)) {
            _E("Error in ace_set_feature");
            ThrowMsg(Exceptions::AceCheckFailed, "Instalation failure. "
                                             "ace_set_feature failure.");
        }
        return;
    }

    _D("Next device cap.");
    // Process next device cap
    SwitchToStep(&TaskAceCheck::StepAceCheck);
}

void TaskAceCheck::StepCheckAceResponse()
{
    _D("Checking ACE response");
    if (m_context.featureLogic->isRejected()) {
        _E("Installation failure. Some devCap was not accepted by ACE.");
        ThrowMsg(
            Exceptions::PrivilegeLevelViolation,
            "Instalation failure. "
            "Some deviceCap was not accepted by ACE.");
    }
    _D("Updating \"feature reject status\" in database!");
    auto it = m_context.featureLogic->resultBegin();
    auto end = m_context.featureLogic->resultEnd();
    for (; it != end; ++it) {
        _D("  |-  Feature: %ls has reject status: %d", it->name.c_str(), it->rejected);
        if (it->rejected) {
            WrtDB::WidgetDAO dao(m_context.widgetConfig.tzAppid);
            dao.updateFeatureRejectStatus(*it);

#ifdef SERVICE_ENABLED
            FOREACH( svcApp , m_context.widgetConfig.configInfo.serviceAppInfoList){
                WrtDB::WidgetDAO dao(svcApp->serviceId);
                dao.updateFeatureRejectStatus(*it);
            }
#endif
        }
    }
    _D("Installation continues...");
}

void TaskAceCheck::StartStep()
{
    LOGI("--------- <TaskAceCheck> : START ----------");
}

void TaskAceCheck::EndStep()
{
    m_context.job->UpdateProgress(
        InstallerContext::INSTALL_ACE_CHECK,
        "Widget Access Control Check Finished");

    LOGI("--------- <TaskAceCheck> : END ----------");
}
} //namespace WidgetInstall
} //namespace Jobs
