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
 * @file    manifest.h
 * @author  Mariusz Domanski (m.domanski@samsung.com)
 */

#ifndef INSTALLER_JOBS_MANIFEST_H
#define INSTALLER_JOBS_MANIFEST_H

#include <list>

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <dpl/platform.h>
#include <dpl/string.h>
#include <dpl/optional_typedefs.h>
#include <dpl/foreach.h>
#include <dpl/wrt-dao-ro/config_parser_data.h>

namespace Jobs {
namespace WidgetInstall {
/**
 * @brief string with optional language attribute
 */
class StringWithLang
{
  public:
    StringWithLang() { }
    StringWithLang(DPL::String s) : string(s) { }
    StringWithLang(DPL::String s, DPL::String l) : string(s), lang(l) { }
    DPL::String getString()
    {
        return this->string;
    }
    DPL::String getLang()
    {
        return this->lang;
    }
    bool hasLang()
    {
        return !this->lang.empty();
    }
    int operator==(const StringWithLang &other)
    {
        return (DPL::ToUTF8String(other.string) == DPL::ToUTF8String(string)) &&
               (DPL::ToUTF8String(other.lang) == DPL::ToUTF8String(lang));
    }

  private:
    DPL::String string;
    DPL::String lang;
};

class IconType : public StringWithLang
{
    public:
      IconType() { }
      IconType(DPL::String s) : StringWithLang(s), small(false) { }
      IconType(DPL::String s, DPL::String l) : StringWithLang(s, l), small(false) { }
      IconType(DPL::String s, bool m) : StringWithLang(s), small(m) { }
      IconType(DPL::String s, DPL::String l, bool m) : StringWithLang(s, l), small(m) { }

      bool isSmall()
      {
          return small;
      }

      int operator==(const IconType &other)
      {
          return (StringWithLang::operator==(other) && (other.small == small));
      }

    private:
        bool small;
};

typedef StringWithLang LabelType, DescriptionType;

/**
 * These types are basicaly strings but they should allow usage of different
 * range of characters or words (details in XML spec.).
 * For simplicity DPL::Strings are used, although this can lead to XML
 * validation
 * errors (related to usage of not allowed characters in given places).
 */
typedef DPL::String NcnameType, NmtokenType, AnySimpleType, LangType;
typedef DPL::String OperationType, MimeType, UriType, TypeType, PackageType;
typedef DPL::OptionalString InstallLocationType, CategoriesType;
typedef DPL::String AppCategoryType;
typedef DPL::OptionalString KeyType, ValueType;
typedef DPL::String AmbientType;

#ifdef IME_ENABLED
typedef DPL::String UuidType, LanguageType, IseTypeType, OptionType;
#endif

#ifdef SERVICE_ENABLED
typedef DPL::String ComponentType;
#endif

/**
 * xmllib2 wrappers
 */
void writeElement(xmlTextWriterPtr writer, const char * name, DPL::String body);
void writeText(xmlTextWriterPtr writer, DPL::String text);
void writeElement(xmlTextWriterPtr writer, const char * name, const char * body);
void writeElementWithOneAttribute(xmlTextWriterPtr writer,
                                  const char * name,
                                  const char * body,
                                  const char * nameAttr,
                                  DPL::String bodyAttr,
                                  bool condition = true);
void startElement(xmlTextWriterPtr writer, const char * name);
void endElement(xmlTextWriterPtr writer);
void writeAttribute(xmlTextWriterPtr writer, const char * name,
                    DPL::String body, bool condition);
void writeAttribute(xmlTextWriterPtr writer, const char * name,
                    const char * body, bool condition);

/**
 * @brief author element
 */
class Author
{
  public:
    Author() {}
    Author(AnySimpleType e,
           NcnameType h,
           LangType l,
           DPL::String b) :
        email(e), href(h), lang(l), body(b) {}
    void serialize(xmlTextWriterPtr writer);

  private:
    AnySimpleType email;
    NcnameType href;
    LangType lang;
    DPL::String body;
};

typedef Author AuthorType;

/**
 * @brief application-service element
 */
class AppControl
{
  public:
    AppControl() {}
    void addOperation(const OperationType &x)
    {
        this->operation.push_back(x);
    }
    void addUri(const UriType &x)
    {
        this->uri.push_back(x);
    }
    void addMime(const MimeType &x)
    {
        this->mime.push_back(x);
    }
    void serialize(xmlTextWriterPtr writer);

  private:
    std::list<OperationType> operation; //attr name AnySimpleType
    std::list<UriType> uri; //attr name AnySimpleType
    std::list<MimeType> mime; //attr name AnySimpleType
};

typedef AppControl AppControlType;

/**
 * @brief account element
 */
typedef std::list<std::pair<DPL::String, DPL::String>> IconListType;
typedef std::list<LabelType> DisplayNameListType;
typedef std::list<DPL::String> AccountCapabilityType;

struct AccountProvider
{
    NcnameType appid;
    NcnameType multiAccount;
    IconListType icon;
    DisplayNameListType name;
    AccountCapabilityType capability;
};

typedef AccountProvider AccountProviderType;

class Account
{
  public:
    Account() {}
    void addAccountProvider(const AccountProvider &x)
    {
        this->provider = x;
    }
    void serialize(xmlTextWriterPtr writer);

  private:
    AccountProviderType provider;
};

class Privilege
{
  public:
    Privilege() {}
    void addPrivilegeName(const DPL::String &x)
    {
        this->name.push_back(x);
    }
    bool isEmpty()
    {
        return this->name.empty();
    }

    void serialize(xmlTextWriterPtr writer);

  private:
    std::list<DPL::String> name;
};

typedef Privilege PrivilegeType;

class Metadata
{
  public:
    Metadata(KeyType k, ValueType v) :
        key(k),
        value(v)
    {}
    void serialize(xmlTextWriterPtr writer);

  private:
    KeyType key;
    ValueType value;
};

typedef Metadata MetadataType;

#ifdef IME_ENABLED
/**
 * @brief ime-application element
 */
class ImeApplication
{
  public:
    ImeApplication() {}
    void setAppid(const NcnameType &x)
    {
        this->appid = x;
    }
    void addLabel(const LabelType &x)
    {
        this->label.push_back(x);
    }
    void addUuid(const UuidType &x)
    {
        this->uuid = x;
    }
    void addLanguage(const LanguageType &x)
    {
        this->language.push_back(x);
    }
    void addIseType(const IseTypeType &x)
    {
        this->iseType = x;
    }
    void addOption(const OptionType &x)
    {
        this->option.push_back(x);
    }
    void serialize(xmlTextWriterPtr writer);

  private:
    NcnameType appid;
    std::list<LabelType> label;
    DPL::String uuid;
    std::list<LanguageType> language;
    DPL::String iseType;
    std::list<OptionType> option;
};

typedef ImeApplication ImeApplicationType;
#endif

#ifdef SERVICE_ENABLED
/**
 * @brief service-application element
 */

class ServiceApplication
{
  public:
    ServiceApplication() {}
    void setAppid(const NcnameType &x)
    {
        this->appid = x;
    }
    void setComponent(const ComponentType &x)
    {
        this->component = x;
    }
    void setAutoRestart(bool x)
    {
        this->autoRestart = x;
    }
    void setOnBoot(bool x)
    {
        this->onBoot = x;
    }
    void setExec(const AnySimpleType &x)
    {
        this->exec = x;
    }
    void setExtraid(const NcnameType &x)
    {
        this->extraid = x;
    }
    void setNodisplay(bool x)
    {
        this->nodisplay = x;
    }
    void setMultiple(bool x)
    {
        this->multiple = x;
    }
    void setType(const TypeType &x)
    {
        this->type = x;
    }
    void setTaskmanage(bool x)
    {
        this->taskmanage = x;
    }
    void setCategories(const NcnameType &x)
    {
        this->categories = x;
    }
    void addLabel(const LabelType &x)
    {
        this->label.push_back(x);
    }
    void addIcon(const IconType &x)
    {
        this->icon.push_back(x);
    }
    void addAppCategory(const AppCategoryType &x)
    {
        this->appCategory.push_back(x);
    }
    void addMetadata(const MetadataType &x)
    {
        this->metadata.push_back(x);
    }
    void serialize(xmlTextWriterPtr writer);

    DPL::OptionalBool isNoDisplay() {
        return this->nodisplay;
    }

  private:
    NcnameType appid;
    DPL::String component;
    DPL::OptionalBool autoRestart;
    DPL::OptionalBool onBoot;
    AnySimpleType exec;
    NcnameType extraid;
    DPL::OptionalBool nodisplay;
    DPL::OptionalBool multiple;
    TypeType type;
    DPL::OptionalBool taskmanage;
    CategoriesType categories;
    std::list<LabelType> label;
    std::list<IconType> icon;
    std::list<AppCategoryType> appCategory;
    std::list<MetadataType> metadata;
};

typedef ServiceApplication ServiceApplicationType;

#endif

/**
 * @brief ui-application element
 */
class UiApplication
{
  public:
    UiApplication() {}
    void setAppid(const NcnameType &x)
    {
        this->appid = x;
    }
    void setExtraid(const NcnameType &x)
    {
        this->extraid = x;
    }
    void setExec(const AnySimpleType &x)
    {
        this->exec = x;
    }
    void setMultiple(bool x)
    {
        this->multiple = x;
    }
    void setNodisplay(bool x)
    {
        this->nodisplay = x;
    }
    void setTaskmanage(bool x)
    {
        this->taskmanage = x;
    }
    void setType(const TypeType &x)
    {
        this->type = x;
    }
    void setCategories(const NcnameType &x)
    {
        this->categories = x;
    }
    void addLabel(const LabelType &x)
    {
        this->label.push_back(x);
    }
    void addIcon(const IconType &x)
    {
        this->icon.push_back(x);
    }
    void addAppControl(const AppControlType &x)
    {
        this->appControl.push_back(x);
    }
    void addAppCategory(const AppCategoryType &x)
    {
        this->appCategory.push_back(x);
    }
    void addMetadata(const MetadataType &m)
    {
        this->metadata.push_back(m);
    }
    void setAmbientSupport(const AmbientType &x)
    {
        this->ambient = x;
    }
    void serialize(xmlTextWriterPtr writer);

    DPL::OptionalBool isNoDisplay() {
        return this->nodisplay;
    }

  private:
    NcnameType appid;
    NcnameType extraid;
    AnySimpleType exec;
    DPL::OptionalBool multiple;
    DPL::OptionalBool nodisplay;
    DPL::OptionalBool taskmanage;
    TypeType type;
    CategoriesType categories;
    std::list<LabelType> label;
    std::list<IconType> icon;
    std::list<AppControlType> appControl;
    std::list<AppCategoryType> appCategory;
    std::list<MetadataType> metadata;
    DPL::String ambient;
};

typedef UiApplication UiApplicationType;

#if USE(WEB_PROVIDER)
/**
 * @brief LiveBox element
 */
typedef WrtDB::ConfigParserData::LiveboxInfo::BoxSizeList BoxSizeType;
typedef WrtDB::ConfigParserData::LiveboxInfo::BoxLabelList BoxLabelType;

struct BoxInfo
{
    NcnameType boxSrc;
    NcnameType boxMouseEvent;
    NcnameType boxTouchEffect;
    BoxSizeType boxSize;
    NcnameType pdSrc;
    NcnameType pdWidth;
    NcnameType pdHeight;
};
typedef BoxInfo BoxInfoType;

class LiveBox
{
  public:
    LiveBox() { }
    void setLiveboxId(const NcnameType &x)
    {
        this->liveboxId = x;
    }
    void setPrimary(const NcnameType &x)
    {
        this->primary = x;
    }
    void setAutoLaunch(const NcnameType &x)
    {
        this->autoLaunch = x;
    }
    void setUpdatePeriod(const NcnameType &x)
    {
        this->updatePeriod = x;
    }
    void setLabel(const BoxLabelType &x)
    {
        this->label = x;
    }
    void setIcon(const NcnameType &x)
    {
        this->icon = x;
    }
    void setBox(const BoxInfoType &x)
    {
        this->box = x;
    }

    void serialize(xmlTextWriterPtr writer);

  private:
    NcnameType liveboxId;
    NcnameType primary;
    NcnameType autoLaunch;
    NcnameType updatePeriod;
    NcnameType timeout;
    BoxLabelType label;
    NcnameType icon;
    NcnameType lang;
    BoxInfoType box;
};

typedef LiveBox LiveBoxInfo;
#endif

/**
 * @brief manifest element
 *
 * Manifest xml file representation.
 */
class Manifest
{
  public:
    Manifest() {}
    void serialize(xmlTextWriterPtr writer);
    void generate(DPL::String filename);

    void addLabel(const LabelType &x)
    {
#ifdef MULTIPROCESS_SERVICE_SUPPORT
        auto pos = std::find(label.begin(), label.end(), x);
        if (pos == label.end()) {
            this->label.push_back(x);
        }
#else
        this->label.push_back(x);
#endif
    }
    void addIcon(const IconType &x)
    {
        this->icon.push_back(x);
    }
    void addAuthor(const AuthorType &x)
    {
        this->author.push_back(x);
    }
    void addDescription(const DescriptionType &x)
    {
        this->description.push_back(x);
    }
    //    void addCompatibility(const CompatibilityType &x)
    //    {
    //        this->compatibility.push_back(x);
    //    }
    //    void addDeviceProfile(const DeviceProfileType &x)
    //    {
    //        this->deviceProfile.push_back(x);
    //    }
    void addUiApplication(const UiApplicationType &x)
    {
        this->uiApplication.push_back(x);
    }
#ifdef IME_ENABLED
    void addImeApplication(const ImeApplicationType &x)
    {
        this->imeApplication.push_back(x);
    }
#endif
#ifdef SERVICE_ENABLED
    void addServiceApplication(const ServiceApplicationType &x)
    {
        this->serviceApplication.push_back(x);
    }
#endif
    //    void addFont(const FontType &x) { this->font.push_back(x); }

#if USE(WEB_PROVIDER)
    void addLivebox(const LiveBoxInfo &x)
    {
        this->livebox.push_back(x);
    }
#endif

    void addAccount(const Account &x)
    {
        this->account.push_back(x);
    }

    void addPrivileges(const PrivilegeType &x)
    {
        this->privileges = x;
    }

    void setInstallLocation(const InstallLocationType &x)
    {
        this->installLocation = x;
    }
    void setPackage(const NcnameType &x)
    {
        this->package = x;
    }
    void setType(const PackageType &x)
    {
        this->type = x;
    }
    void setVersion(const NmtokenType &x)
    {
        this->version = x;
    }
    void setStoreClientId(const NcnameType &x)
    {
        this->storeClientId= x;
    }
    void setCscPath(const NcnameType &x)
    {
        this->cscPath = x;
    }
    void setApiVersion(const NmtokenType &x)
    {
        this->apiVersion = x;
    }

  private:
    std::list<LabelType> label;
    std::list<IconType> icon;
    std::list<AuthorType> author;
    std::list<DescriptionType> description;
    //    std::list<CompatibilityType> compatibility;
    //    std::list<DeviceProfileType> deviceProfile;
#ifdef SERVICE_ENABLED
    std::list<ServiceApplicationType> serviceApplication;
#endif
    std::list<UiApplicationType> uiApplication;
#ifdef IME_ENABLED
    std::list<ImeApplicationType> imeApplication;
#endif
    //    std::list<FontType> font;
#if USE(WEB_PROVIDER)
    std::list<LiveBoxInfo> livebox;
#endif
    InstallLocationType installLocation;
    NcnameType package;
    PackageType type;
    NmtokenType version;
    std::list<Account> account;
    PrivilegeType privileges;
    NcnameType storeClientId;
    NcnameType cscPath;
    NcnameType apiVersion;

};
} //namespace Jobs
} //namespace WidgetInstall

#endif //INSTALLER_JOBS_MANIFEST_H
