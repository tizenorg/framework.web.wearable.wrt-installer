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
 * @file    manifest.cpp
 * @author  Mariusz Domanski (m.domanski@samsung.com)
 */

#include "manifest.h"
#include "libxml_utils.h"
#include <widget_install/task_manifest_file.h>
#include <dpl/foreach.h>
#include <installer_log.h>

namespace Jobs {
namespace WidgetInstall {
void writeElement(xmlTextWriterPtr writer, const char * name, DPL::String body)
{
    int state = xmlTextWriterWriteElement(writer, BAD_CAST name,
                                          BAD_CAST DPL::ToUTF8String(
                                              body).c_str());
    if (state < 0) {
        ThrowMsg(LibxmlUtils::Libxml2Error, "xmlTextWriterWriteElement failed");
    }
}

void writeText(xmlTextWriterPtr writer, DPL::String text)
{
    int state = xmlTextWriterWriteString(writer,
                                         BAD_CAST DPL::ToUTF8String(text).c_str());
    if (state < 0) {
        ThrowMsg(LibxmlUtils::Libxml2Error, "xmlTextWriterWriteText failed");
    }
}

void writeElement(xmlTextWriterPtr writer, const char * name, const char * body)
{
    int state = xmlTextWriterWriteElement(writer, BAD_CAST name, BAD_CAST body);
    if (state < 0) {
        ThrowMsg(LibxmlUtils::Libxml2Error, "xmlTextWriterWriteElement failed");
    }
}

void writeElementWithOneAttribute(xmlTextWriterPtr writer,
                                  const char * name,
                                  DPL::String body,
                                  const char * nameAttr,
                                  DPL::String bodyAttr,
                                  bool condition = true)
{
    startElement(writer, name);
    writeAttribute(writer, nameAttr, bodyAttr, condition);
    writeText(writer, body);
    endElement(writer);
}

void startElement(xmlTextWriterPtr writer, const char * name)
{
    int state = xmlTextWriterStartElement(writer, BAD_CAST name);
    if (state < 0) {
        ThrowMsg(LibxmlUtils::Libxml2Error, "xmlTextWriterStartElement failed");
    }
}

void endElement(xmlTextWriterPtr writer)
{
    int state = xmlTextWriterEndElement(writer);
    if (state < 0) {
        ThrowMsg(LibxmlUtils::Libxml2Error, "xmlTextWriterEndElement failed");
    }
}

void writeAttribute(xmlTextWriterPtr writer,
                    const char * name,
                    DPL::String body,
                    bool condition = true)
{
    if (!condition) {
        return;
    }
    int state = xmlTextWriterWriteAttribute(writer, BAD_CAST name,
                                            BAD_CAST DPL::ToUTF8String(
                                                body).c_str());
    if (state < 0) {
        ThrowMsg(LibxmlUtils::Libxml2Error,
                 "xmlTextWriterWriteAttribute failed");
    }
}

void writeAttribute(xmlTextWriterPtr writer,
                    const char * name,
                    const char * body,
                    bool condition = true)
{
    if (!condition) {
        return;
    }
    int state = xmlTextWriterWriteAttribute(writer,
                                            BAD_CAST name,
                                            BAD_CAST body);
    if (state < 0) {
        ThrowMsg(LibxmlUtils::Libxml2Error,
                 "xmlTextWriterWriteAttribute failed");
    }
}

void Manifest::generate(DPL::String filename)
{
    xmlTextWriterPtr writer;
    int state;

    //compression set to 0
    writer = xmlNewTextWriterFilename(DPL::ToUTF8String(filename).c_str(), 0);

    if (writer == NULL) {
        ThrowMsg(LibxmlUtils::Libxml2Error, "xmlNewTextWriterFilename failed");
    }
    state = xmlTextWriterSetIndent(writer, 1);
    if (state < 0) {
        ThrowMsg(LibxmlUtils::Libxml2Error, "xmlTextWriterSetIndent failed");
    }

    state = xmlTextWriterStartDocument(writer, NULL, "utf-8", NULL);
    if (state < 0) {
        ThrowMsg(LibxmlUtils::Libxml2Error, "xmlTextWriterStartDocument failed");
    }
    this->serialize(writer);
    state = xmlTextWriterEndDocument(writer);
    if (state < 0) {
        ThrowMsg(LibxmlUtils::Libxml2Error, "xmlTextWriterEndDocument failed");
    }
    if (writer != NULL) {
        xmlFreeTextWriter(writer);
        writer = NULL;
    }
}

void Manifest::serialize(xmlTextWriterPtr writer)
{
    startElement(writer, "manifest");
    {
        writeAttribute(writer, "xmlns", "http://tizen.org/ns/packages");
        writeAttribute(writer, "package", this->package);
        writeAttribute(writer, "type", this->type);
        writeAttribute(writer, "version", this->version);
        if (!!this->installLocation) {
            writeAttribute(writer, "install-location", (*this->installLocation));
        }
        writeAttribute(writer, "storeclient-id", this->storeClientId,
                !this->storeClientId.empty());
        writeAttribute(writer, "csc_path", this->cscPath,
                !this->cscPath.empty());

        FOREACH(l, this->label)
        {
            writeElementWithOneAttribute(writer, "label", l->getString(),
                                         "xml:lang", l->getLang(), l->hasLang());
        }
        FOREACH(i, this->icon)
        {
            writeElementWithOneAttribute(writer, "icon", i->getString(),
                                         "xml:lang", i->getLang(), i->hasLang());
        }
        FOREACH(a, this->author)
        {
            a->serialize(writer);
        }
        FOREACH(d, this->description)
        {
            writeElementWithOneAttribute(writer, "description", d->getString(),
                                         "xml:lang", d->getLang(), d->hasLang());
        }
        //FOREACH(c, this->compatibility) { c->serialize(writer); }
        //FOREACH(d, this->deviceProfile) { d->serialize(writer); }
#ifdef SERVICE_ENABLED
        FOREACH(s, this->serviceApplication) {
            s->serialize(writer);
        }
#endif
        FOREACH(u, this->uiApplication) {
            u->serialize(writer);
        }
#ifdef IME_ENABLED
        FOREACH(i, this->imeApplication) {
            i->serialize(writer);
        }
#endif
        //FOREACH(f, this->font) { f->serialize(writer); }
#ifdef DBOX_ENABLED
        FOREACH(l, this->livebox) {
            l->serialize(writer);
        }
#endif
        FOREACH(acc, this->account)
        {
            acc->serialize(writer);
        }

        if (!this->privileges.isEmpty()) {
            this->privileges.serialize(writer);
        }
    }
    endElement(writer);
}

void Author::serialize(xmlTextWriterPtr writer)
{
    startElement(writer, "author");
    writeAttribute(writer, "email", this->email, !this->email.empty());
    writeAttribute(writer, "href", this->href, !this->href.empty());
    writeAttribute(writer, "xml:lang", this->lang, !this->lang.empty());
    writeText(writer, body);
    endElement(writer);
}

void UiApplication::serialize(xmlTextWriterPtr writer)
{
    startElement(writer, "ui-application");
    writeAttribute(writer, "appid", this->appid);
    writeAttribute(writer, "exec", this->exec);
    if (!!this->multiple) {
        writeAttribute(writer, "multiple", (*this->multiple) ? "true" : "false");
    }
    if (!!this->nodisplay) {
        writeAttribute(writer,
                       "nodisplay",
                       (*this->nodisplay) ? "true" : "false");
    }
    if (!!this->taskmanage) {
        writeAttribute(writer,
                       "taskmanage",
                       (*this->taskmanage) ? "true" : "false");
    }
    writeAttribute(writer, "type", this->type);
    writeAttribute(writer, "extraid", this->extraid);
    if (!!this->categories) {
        writeAttribute(writer, "categories", (*this->categories));
    }
    FOREACH(l, this->label)
    {
        writeElementWithOneAttribute(writer, "label",
                                     l->getString(), "xml:lang",
                                     l->getLang(), l->hasLang());
    }
    FOREACH(i, this->icon)
    {
        startElement(writer, "icon");
        {
            writeAttribute(writer, "xml:lang", i->getLang(), i->hasLang());
            writeAttribute(writer, "section", "small", i->isSmall());
            writeText(writer, i->getString());
        }
        endElement(writer);
    }
    FOREACH(a, this->appControl)
    {
        a->serialize(writer);
    }
    FOREACH(c, this->appCategory)
    {
        startElement(writer, "category");
        writeAttribute(writer, "name", *c);
        endElement(writer);
    }
    FOREACH(m, this->metadata) {
        m->serialize(writer);
    }
    endElement(writer);
}

#ifdef IME_ENABLED
void ImeApplication::serialize(xmlTextWriterPtr writer)
{
    startElement(writer, "ime");
    writeAttribute(writer, "appid", this->appid);
    {
        startElement(writer, "uuid");
        writeText(writer, this->uuid);
        endElement(writer);
        FOREACH(l, this->label)
        {
            writeElementWithOneAttribute(writer, "label", l->getString(), "xml:lang", l->getLang(), l->hasLang());
        }
        startElement(writer, "languages");
        {
            FOREACH(g, this->language)
            {
                startElement(writer, "language");
                writeText(writer, *g);
                endElement(writer);
            }
        }
        endElement(writer);
        startElement(writer, "type");
        writeText(writer, this->iseType);
        endElement(writer);
        startElement(writer, "options");
        {
            FOREACH(o, this->option)
            {
                startElement(writer, "option");
                writeText(writer, *o);
                endElement(writer);
            }
        }
        endElement(writer);
    }
    endElement(writer);
}
#endif

#ifdef SERVICE_ENABLED
void ServiceApplication::serialize(xmlTextWriterPtr writer)
{
    startElement(writer, "ui-application");
    writeAttribute(writer, "component-type", this->component);
    writeAttribute(writer, "auto-restart", (!!this->autoRestart && (*this->autoRestart)) ? "true" : "false");
    writeAttribute(writer, "on-boot", (!!this->onBoot && (*this->onBoot)) ? "true" : "false");
    writeAttribute(writer, "appid", this->appid);
    writeAttribute(writer, "exec", this->exec);
    writeAttribute(writer, "extraid", this->extraid);
    if (!!this->nodisplay) {
        writeAttribute(writer, "nodisplay", (*this->nodisplay) ? "true" : "false");
    }
    if (!!this->multiple) {
        writeAttribute(writer, "multiple", (*this->multiple) ? "true" : "false");
    }
    writeAttribute(writer, "type", this->type);
    if (!!this->taskmanage) {
        writeAttribute(writer, "taskmanage", (*this->taskmanage) ? "true" : "false");
    }
    if (!!this->categories) {
        writeAttribute(writer, "categories", (*this->categories));
    }
    FOREACH(l, this->label)
    {
        writeElementWithOneAttribute(writer, "label",
                                     l->getString(), "xml:lang",
                                     l->getLang(), l->hasLang());
    }
    FOREACH(i, this->icon)
    {
        writeElementWithOneAttribute(writer, "icon", i->getString(), "xml:lang",
                                     i->getLang(), i->hasLang());
    }
    FOREACH(c, this->appCategory)
    {
        startElement(writer, "category");
        writeAttribute(writer, "name", *c);
        endElement(writer);
    }
    FOREACH(m, this->metadata) {
        m->serialize(writer);
    }
    endElement(writer);
}
#endif

void AppControl::serialize(xmlTextWriterPtr writer)
{
    startElement(writer, "app-control");
    FOREACH(o, this->operation)
    {
        startElement(writer, "operation");
        writeAttribute(writer, "name", *o);
        endElement(writer);
    }
    FOREACH(u, this->uri)
    {
        startElement(writer, "uri");
        writeAttribute(writer, "name", *u);
        endElement(writer);
    }
    FOREACH(m, this->mime)
    {
        startElement(writer, "mime");
        writeAttribute(writer, "name", *m);
        endElement(writer);
    }
    endElement(writer);
}

#ifdef DBOX_ENABLED
void LiveBox::serialize(xmlTextWriterPtr writer)
{
    startElement(writer, "livebox");
    if (!this->liveboxId.empty()) {
        writeAttribute(writer, "appid", this->liveboxId);
    }

    if (!this->primary.empty()) {
        writeAttribute(writer, "primary", this->primary);
    }

    if (!this->updatePeriod.empty()) {
        writeAttribute(writer, "period", this->updatePeriod);
    }

    writeAttribute(writer, "abi", "html");
    writeAttribute(writer, "network", "true");
    writeAttribute(writer, "nodisplay", "false");

    if (!this->label.empty()) {
        int defaultLabelChk = 0;
        FOREACH(m, this->label)
        {
            std::pair<DPL::String, DPL::String> boxLabel = *m;
            startElement(writer, "label");
            if (!boxLabel.first.empty()) {
                writeAttribute(writer, "xml:lang", boxLabel.first);
            } else {
                defaultLabelChk++;
            }
            writeText(writer, boxLabel.second);
            endElement(writer);
        }
        if(!defaultLabelChk) {
            startElement(writer, "label");
            writeText(writer, DPL::FromUTF8String("NO NAME"));
            endElement(writer);
        }
    }
    if (!this->icon.empty()) {
        startElement(writer, "icon");
        writeText(writer, this->icon);
        endElement(writer);
    }

    if (!this->autoLaunch.empty()) {
        startElement(writer, "launch");
        writeText(writer, this->autoLaunch);
        endElement(writer);
    }

    if (!this->box.boxSrc.empty() &&
        !this->box.boxMouseEvent.empty() &&
        !this->box.boxSize.empty())
    {
        startElement(writer, "box");
        writeAttribute(writer, "type", "buffer");
        writeAttribute(writer, "mouse_event", this->box.boxMouseEvent);
        writeAttribute(writer, "touch_effect", this->box.boxTouchEffect);

        FOREACH(it, this->box.boxSize)
        {
            startElement(writer, "size");
            if (!(*it).m_preview.empty()) {
                writeAttribute(writer, "preview", (*it).m_preview);
            }
            if (!(*it).m_useDecoration.empty()) {
                writeAttribute(writer, "need_frame", (*it).m_useDecoration);
            } else {
                // default value of use-decoration is "true"
                writeAttribute(writer, "need_frame", DPL::String(L"true"));
            }

            writeText(writer, (*it).m_size);
            endElement(writer);
        }

        startElement(writer, "script");
        writeAttribute(writer, "src", this->box.boxSrc);
        endElement(writer);

        endElement(writer);

        if (!this->box.pdSrc.empty() &&
            !this->box.pdWidth.empty() &&
            !this->box.pdHeight.empty())
        {
            startElement(writer, "pd");
            writeAttribute(writer, "type", "buffer");

            startElement(writer, "size");
            DPL::String pdSize = this->box.pdWidth + DPL::String(L"x") +
                this->box.pdHeight;
            writeText(writer, pdSize);
            endElement(writer);

            startElement(writer, "script");
            writeAttribute(writer, "src", this->box.pdSrc);
            endElement(writer);

            endElement(writer);
        }
    }

    endElement(writer);
}
#endif

void Account::serialize(xmlTextWriterPtr writer)
{
    startElement(writer, "account");
    {
        startElement(writer, "account-provider");
        writeAttribute(writer, "appid", this->provider.appid);
        writeAttribute(writer, "multiple-accounts-support",
                this->provider.multiAccount);

        FOREACH(i, this->provider.icon)
        {
            startElement(writer, "icon");
            writeAttribute(writer, "section", i->first);
            writeText(writer, i->second);
            endElement(writer);
        }

        bool setDefaultLang = false;
        FOREACH(n, this->provider.name)
        {
            if (!setDefaultLang && n->getLang() == L"en-gb") {
                writeElement(writer, "label", n->getString());
                setDefaultLang = true;
            }
            writeElementWithOneAttribute(writer, "label",
                    n->getString(), "xml:lang",
                    n->getLang(), n->hasLang());
        }
        if (!setDefaultLang) {
            writeElement(writer, "label", this->provider.name.begin()->getString());
        }

        FOREACH(c, this->provider.capability)
        {
            startElement(writer, "capability");
            writeText(writer, *c);
            endElement(writer);
        }
        endElement(writer);
    }
    endElement(writer);
}

void Privilege::serialize(xmlTextWriterPtr writer)
{
    startElement(writer, "privileges");
    {
        FOREACH(it, this->name)
        {
            startElement(writer, "privilege");
            writeText(writer, *it);
            endElement(writer);
        }
    }
    endElement(writer);
}

void Metadata::serialize(xmlTextWriterPtr writer)
{
    startElement(writer, "metadata");
    writeAttribute(writer, "key", *this->key);
    if (!!this->value) {
        writeAttribute(writer, "value", *this->value);
    }
    endElement(writer);
}
} //namespace Jobs
} //namespace WidgetInstall
