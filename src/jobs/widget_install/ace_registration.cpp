/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file    ace_registration.cpp
 * @author  Andrzej Surdej (a.surdej@gmail.com)
 * @version 1.0
 * @brief   Translate structures to ace api - implementation file
 */

#include <ace_registration.h>
#include <dpl/foreach.h>
#include <dpl/optional_typedefs.h>
#include <ace_api_install.h>

#include <installer_log.h>

namespace {
char* toAceString(const DPL::OptionalString& os)
{
    if (!!os) {
        return strdup(DPL::ToUTF8String(*os).c_str());
    } else {
        return NULL;
    }
}

char* toAceString(const std::string& str)
{
    if (!str.empty()) {
        return strdup(str.c_str());
    } else {
        return NULL;
    }
}
} //anonymous namespace

namespace AceApi {
bool registerAceWidget(const WrtDB::DbWidgetHandle& widgetHandle,
                       const WrtDB::WidgetRegisterInfo& widgetConfig,
                       const WrtDB::WidgetCertificateDataList& certList)
{
    _D("Updating Ace database");
    struct widget_info wi;

    switch (widgetConfig.webAppType.appType) {
    case WrtDB::APP_TYPE_TIZENWEBAPP:
        wi.type = Tizen;
        break;
    default:
        _E("Unknown application type");
        return false;
    }

    wi.id = toAceString(widgetConfig.configInfo.widget_id);
    wi.version = toAceString(widgetConfig.configInfo.version);
    wi.author = toAceString(widgetConfig.configInfo.authorName);
    // TODO: Need to remove "shareHref" from "widget_info" in wrt-security
    wi.shareHerf = NULL;
    _D("Basic data converted. Certificates begin.");

    //one more element for NULL termination
    _D("Found: %d certificates", certList.size());
    ace_certificate_data** certData = new ace_certificate_data *
        [certList.size() + 1];
    certData[certList.size()] = NULL; // last element set to NULL

    int i = 0;
    FOREACH(it, certList)
    {
        certData[i] = new ace_certificate_data;
        switch (it->owner) {
        case WrtDB::WidgetCertificateData::AUTHOR:
            certData[i]->owner = AUTHOR;
            break;
        case WrtDB::WidgetCertificateData::DISTRIBUTOR:
            certData[i]->owner = DISTRIBUTOR;
            break;
        default:
            _D("Unknown owner type of cert");
            certData[i]->owner = UNKNOWN;
            break;
        }
        switch (it->type) {
        case WrtDB::WidgetCertificateData::ENDENTITY:
            certData[i]->type = ENDENTITY;
            break;
        case WrtDB::WidgetCertificateData::ROOT:
            certData[i]->type = ROOT;
            break;
        default:
            _E("Unknown type of cert");
            certData[i]->type = ENDENTITY;
            break;
        }
        certData[i]->chain_id = it->chainId;

        certData[i]->md5_fp = toAceString(it->strMD5Fingerprint);
        certData[i]->sha1_fp = toAceString(it->strSHA1Fingerprint);
        certData[i]->common_name =
            toAceString(DPL::ToUTF8String(it->strCommonName));
        ++i;
    }

    _D("Registerign widget in ace");
    ace_return_t retval = ace_register_widget(
            static_cast<ace_widget_handle_t>(widgetHandle), &wi, certData);

    //clean up - WidgetInfo
    free(wi.author);
    free(wi.id);
    free(wi.version);

    //free cert list
    i = 0;
    while (certData[i] != NULL) {
        free(certData[i]->common_name);
        free(certData[i]->md5_fp);
        free(certData[i]->sha1_fp);
        delete certData[i];
        ++i;
    }
    delete[] certData;
    return retval == ACE_OK;
}
bool registerAceWidgetFromDB(const WrtDB::DbWidgetHandle& widgetHandle)
{
    using namespace WrtDB;
    _D("Updating Ace database from Widget DB");
    struct widget_info wi;
    DPL::OptionalString os;
    WrtDB::WidgetCertificateDataList certList;

    wi.id = NULL;
    wi.version = NULL;
    wi.author = NULL;
    wi.shareHerf = NULL;

    Try {
        WidgetDAOReadOnly dao(widgetHandle);

        WidgetType type = dao.getWidgetType();
        if (type == WrtDB::APP_TYPE_TIZENWEBAPP) {
            wi.type = Tizen;
        } else {
            _E("Unknown application type");
            return false;
        }

        wi.id = toAceString(dao.getGUID());
        wi.version = toAceString(dao.getVersion());
        wi.author = toAceString(dao.getAuthorName());
        // TODO: Need to remove "shareHref" from "widget_info" in wrt-security
        wi.shareHerf = NULL;
        _D("Basic data converted. Certificates begin.");
        certList = dao.getCertificateDataList();
    }
    Catch(WidgetDAOReadOnly::Exception::WidgetNotExist) {
        _E("Widget does not exist");
        if(wi.id != NULL) {
            free(wi.id);
        }
        if(wi.version != NULL) {
            free(wi.version);
        }
        if(wi.author != NULL) {
            free(wi.author);
        }
        if(wi.shareHerf != NULL) {
            free(wi.shareHerf);
        }
        return false;
    }

    //one more element for NULL termination
    _D("Found: %d certificates", certList.size());
    ace_certificate_data** certData = new ace_certificate_data *
        [certList.size() + 1];
    certData[certList.size()] = NULL; // last element set to NULL

    int i = 0;
    FOREACH(it, certList)
    {
        certData[i] = new ace_certificate_data;
        switch (it->owner) {
        case WrtDB::WidgetCertificateData::AUTHOR:
            certData[i]->owner = AUTHOR;
            break;
        case WrtDB::WidgetCertificateData::DISTRIBUTOR:
            certData[i]->owner = DISTRIBUTOR;
            break;
        default:
            _D("Unknown owner type of cert");
            certData[i]->owner = UNKNOWN;
            break;
        }
        switch (it->type) {
        case WrtDB::WidgetCertificateData::ENDENTITY:
            certData[i]->type = ENDENTITY;
            break;
        case WrtDB::WidgetCertificateData::ROOT:
            certData[i]->type = ROOT;
            break;
        default:
            _E("Unknown type of cert");
            certData[i]->type = ENDENTITY;
            break;
        }
        certData[i]->chain_id = it->chainId;

        certData[i]->md5_fp = toAceString(it->strMD5Fingerprint);
        certData[i]->sha1_fp = toAceString(it->strSHA1Fingerprint);
        certData[i]->common_name =
            toAceString(DPL::ToUTF8String(it->strCommonName));
        ++i;
    }

    _D("Registerign widget in ace");
    ace_return_t retval = ace_register_widget(
            static_cast<ace_widget_handle_t>(widgetHandle), &wi, certData);

    //clean up - WidgetInfo
    free(wi.author);
    free(wi.id);
    free(wi.version);

    //free cert list
    i = 0;
    while (certData[i] != NULL) {
        free(certData[i]->common_name);
        free(certData[i]->md5_fp);
        free(certData[i]->sha1_fp);
        delete certData[i];
        ++i;
    }
    delete[] certData;
    return retval == ACE_OK;
}
}
