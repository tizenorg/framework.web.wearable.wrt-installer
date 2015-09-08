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
 * @file        ManifestFile.cpp
 * @author      Tomasz Iwanek (t.iwanek@samsung.com)
 * @brief       Manifest file reading
 */

#include <ManifestFile.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <installer_log.h>

//TODO: This file reads manifest file. This functionality is familiar with writing
//      in wrt-installer but reading ws not necessary there.
//      Maybe it should be changed in some way.

ManifestFile::ManifestFile(const std::string & file) : filename(file)
{
    xmlXPathInit();
    parse();
}

ManifestFile::~ManifestFile()
{
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
}

void ManifestFile::parse()
{
    doc = xmlReadFile(filename.c_str(), NULL, 0);
    if (doc == NULL)
    {
        ThrowMsg(ManifestParseError,"File Problem");
    }
    else
    {
        //context
        xpathCtx = xmlXPathNewContext(doc);
        if(xpathCtx == NULL)
        {
            ThrowMsg(ManifestParseError,"Error: unable to create new XPath context\n");
        }
        xpathCtx->node = xmlDocGetRootElement(doc);

        if(xmlXPathRegisterNs(xpathCtx, BAD_CAST "p", BAD_CAST "http://tizen.org/ns/packages") != 0)
        {
            ThrowMsg(ManifestParseError,"Error: unable to register namespace\n");
        }
    }
}

std::string ManifestFile::getValueByXpath(const std::string & path) const
{
    std::string result;
    xmlXPathObjectPtr xpathObject;
    //get requested node's values
    xpathObject = xmlXPathEvalExpression(BAD_CAST path.c_str(), xpathCtx);
    if(xpathObject == NULL)
    {
        ThrowMsg(ManifestParseError,"XPath evaluation failure: " << path);
    }
    xmlNodeSetPtr nodes = xpathObject->nodesetval;
    int size = (nodes) ? nodes->nodeNr : 0;
    if(size != 1)
    {
        ThrowMsg(ManifestParseError,"Xpath does not point 1 element but " << size
                 << " for xpath: " << path);
    }
    else
    {
        if(nodes->nodeTab[0]->type == XML_ELEMENT_NODE)
        {
            xmlNodePtr cur = nodes->nodeTab[0];
            xmlChar * value = xmlNodeGetContent(cur);
            result = std::string(reinterpret_cast<char*>(value)); //this cast should be safe...
            xmlFree(value);
        }
        else if(nodes->nodeTab[0]->type == XML_ATTRIBUTE_NODE)
        {
            xmlNodePtr cur = nodes->nodeTab[0];
            xmlChar * value = xmlNodeGetContent(cur);
            result = std::string(reinterpret_cast<char*>(value));
            xmlFree(value);
        }
    }
    //Cleanup of XPath data
    xmlXPathFreeObject(xpathObject);
    return result;
}
