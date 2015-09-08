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
 * @file    widget_unzip.cpp
 * @author  Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task unzip
 */
#ifndef WIDGET_UNZIP_H
#define WIDGET_UNZIP_H

#include <string>

#include <dpl/zip_input.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>

namespace Jobs {
namespace WidgetInstall {
class WidgetUnzip 
{
  public:
      WidgetUnzip(const std::string &source);
      void unzipWgtFile(const std::string &destination);
      void unzipConfiguration(const std::string &destination, WrtDB::PackagingType *type);
      bool checkAvailableSpace(const std::string &destination);

  private:
    // Unzip state
    std::unique_ptr<DPL::ZipInput> m_zip;
    DPL::ZipInput::const_iterator m_zipIterator;
    std::string m_requestFile;

    void unzipProgress(const std::string &destination);
    void ExtractFile(DPL::ZipInput::File *input, const std::string
            &destFileName);
    bool isDRMPackage(const std::string &source);
    bool decryptDRMPackage(const std::string &source, const std::string
            &decryptedSource);
    std::string getDecryptedPackage(const std::string &source);
};

} //namespace WidgetInstall
} //namespace Jobs

#endif // WIDGET_UNZIP_H
