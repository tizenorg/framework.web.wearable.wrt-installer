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
 * @brief   Implementation file for installer widget unzip
 */
#include <widget_install/widget_unzip.h>
#include <widget_install/widget_install_errors.h>
#include <widget_install/widget_install_context.h>
#include <widget_install/job_widget_install.h>
#include <dpl/copy.h>
#include <dpl/utils/path.h>
#include <dpl/file_output.h>
#include <dpl/abstract_waitable_input_adapter.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <task_commons.h>
#include <sys/stat.h>
#include <storage.h>
#include <dlfcn.h>
#include <installer_log.h>

using namespace WrtDB;

namespace {
const char *const DRM_LIB_PATH = "/usr/lib/libdrm-service-core-tizen.so";
const size_t SPACE_SIZE = 1024 * 1024;
const char *const WEB_APP_CONFIG_XML= "config.xml";
const char *const HYBRID_CONFIG_XML = "res/wgt/config.xml";

struct PathAndFilePair
{
    std::string path;
    std::string file;

    PathAndFilePair(const std::string &p,
                    const std::string &f) :
        path(p),
        file(f)
    {}
};

PathAndFilePair SplitFileAndPath(const std::string &filePath)
{
    std::string::size_type position = filePath.rfind('/');

    // Is this only a file without a path ?
    if (position == std::string::npos) {
        return PathAndFilePair(std::string(), filePath);
    }

    // This is full file-path pair
    return PathAndFilePair(filePath.substr(0,
                                           position),
                           filePath.substr(position + 1));
}
}

namespace Jobs {
namespace WidgetInstall {

WidgetUnzip::WidgetUnzip(const std::string &source)
{
    Try {
        m_requestFile = getDecryptedPackage(source);
        m_zip.reset(new DPL::ZipInput(m_requestFile));
    }
    Catch(DPL::ZipInput::Exception::OpenFailed)
    {
        ReThrowMsg(Exceptions::OpenZipFailed, source);
    }
    Catch(Exceptions::DrmDecryptFailed)
    {
        ReThrowMsg(Exceptions::ExtractFileFailed, source);
    }
}

void WidgetUnzip::ExtractFile(DPL::ZipInput::File *input,
                            const std::string &destFileName)
{
    Try
    {
        DPL::AbstractWaitableInputAdapter inputAdapter(input);
        DPL::FileOutput output(destFileName);

        DPL::Copy(&inputAdapter, &output);
    }
    Catch(DPL::FileOutput::Exception::OpenFailed)
    {
        ReThrowMsg(Exceptions::ExtractFileFailed, destFileName);
    }
    Catch(DPL::CopyFailed)
    {
        ReThrowMsg(Exceptions::ExtractFileFailed, destFileName);
    }
}

void WidgetUnzip::unzipProgress(const std::string &destination)
{
    // Show file info
    _D("Unzipping: '%s', Comment: '%s', Compressed size: %lld, Uncompressed size: %lld",
            m_zipIterator->name.c_str(), m_zipIterator->comment.c_str(), m_zipIterator->compressedSize, m_zipIterator->uncompressedSize);

    // Normalize file paths
    // FIXME: Implement checking for invalid characters

    // Extract file or path
    std::string fileName = m_zipIterator->name;

    if (fileName[fileName.size() - 1] == '/') {
        // This is path
        std::string newPath = destination + "/" +
            fileName.substr(0, fileName.size() - 1);
        _D("Path to extract: %s", newPath.c_str());

        // Create path in case of it is empty
        createTempPath(newPath);
    } else {
        // This is regular file
        std::string fileExtractPath = destination + "/" + fileName;

        _D("File to extract: %s", fileExtractPath.c_str());

        // Split into pat & file pair
        PathAndFilePair pathAndFile = SplitFileAndPath(fileExtractPath);

        _D("Path and file: %s : %s", pathAndFile.path.c_str(), pathAndFile.file.c_str());

        // First, ensure that path exists
        createTempPath(pathAndFile.path);

        Try
        {
            // Open file
            std::unique_ptr<DPL::ZipInput::File> file(
                m_zip->OpenFile(fileName));

            // Extract single file
            ExtractFile(file.get(), fileExtractPath);
        }
        Catch(DPL::ZipInput::Exception::OpenFileFailed)
        {
            ThrowMsg(Exceptions::ExtractFileFailed, fileName);
        }
    }

    // Check whether there are more files to extract
    if (++m_zipIterator == m_zip->end()) {
        _D("Unzip progress finished successfuly");
    } else {
        unzipProgress(destination);
    }
}

bool WidgetUnzip::isDRMPackage(const std::string &source)
{
    _D("Enter : isDRMPackage()");
    int ret = 0;
    void* pHandle = NULL;
    char* pErrorMsg = NULL;
    int (*drm_oem_sapps_is_drm_file)(const char* pDcfPath, int dcfPathLen);

    //TODO: quickfix for platform issues...
    if(!DPL::Utils::Path(DRM_LIB_PATH).Exists())
    {
        _E("Cannot open %s!", DRM_LIB_PATH);
        return false;
    }

    pHandle = dlopen(DRM_LIB_PATH, RTLD_LAZY | RTLD_GLOBAL);
    if (!pHandle) {
        _E("dlopen failed : %s [%s]", source.c_str(), dlerror());
        return false;
    }

    // clear existing error
    dlerror();

    drm_oem_sapps_is_drm_file = reinterpret_cast <int (*)(const char*, int)>
        (dlsym(pHandle, "drm_oem_sapps_is_drm_file"));

    if ((pErrorMsg = dlerror()) != NULL) {
        _E("dlsym failed : %s [%s]", source.c_str(), pErrorMsg);
        dlclose(pHandle);
        return false;
    }

    if (drm_oem_sapps_is_drm_file == NULL) {
        _E("drm_oem_sapps_is_drm_file is NULL : %s", source.c_str());
        dlclose(pHandle);
        return false;
    }

    ret = drm_oem_sapps_is_drm_file(source.c_str(), source.length());
    dlclose(pHandle);
    if (1 == ret) {
        _D("%s is DRM file", source.c_str());
        return true;
    }
    _D("%s isn't DRM file", source.c_str());
    return false;
}

bool WidgetUnzip::decryptDRMPackage(const std::string &source, const std::string
        &decryptedSource)
{
    _D("Enter : decryptDRMPackage()");
    int ret = 0;
    void* pHandle = NULL;
    char* pErrorMsg = NULL;
    int (*drm_oem_sapps_decrypt_package)(const char* pDcfPath, int dcfPathLen,
            const char* pDecryptedFile, int decryptedFileLen);

    pHandle = dlopen(DRM_LIB_PATH, RTLD_LAZY | RTLD_GLOBAL);
    if (!pHandle) {
        _E("dlopen failed : %s [%s]", source.c_str(), dlerror());
        return false;
    }

    // clear existing error
    dlerror();

    drm_oem_sapps_decrypt_package = reinterpret_cast <int (*)(const char*, int,
            const char*, int)>
        (dlsym(pHandle, "drm_oem_sapps_decrypt_package"));

    if ((pErrorMsg = dlerror()) != NULL) {
        _E("dlsym failed : %s [%s]", source.c_str(), pErrorMsg);
        dlclose(pHandle);
        return false;
    }

    if (drm_oem_sapps_decrypt_package == NULL) {
        _E("drm_oem_sapps_decrypt_package is NULL : %s", source.c_str());
        dlclose(pHandle);
        return false;
    }

    ret = drm_oem_sapps_decrypt_package(source.c_str(), source.length(),
            decryptedSource.c_str(), decryptedSource.length());
    dlclose(pHandle);
    if (1 == ret) {
        _D("%s is decrypted : %s", source.c_str(), decryptedSource.c_str());
        return true;
    }
    return false;
}

std::string WidgetUnzip::getDecryptedPackage(const std::string &source)
{
    _D("Check DRM...");
    if (isDRMPackage(source)) {
        std::string decryptedFile;
        size_t found = source.find_last_of(".wgt");
        if (found == std::string::npos) {
            decryptedFile += source + "_tmp.wgt";
        } else {
            decryptedFile += source.substr(0, source.find_last_not_of(".wgt") +
                    1) + "_tmp.wgt";
        }

        _D("decrypted file name : %s", decryptedFile.c_str());
        if (!decryptDRMPackage(source, decryptedFile)) {
            _E("Failed decrypt drm file");
            ThrowMsg(Exceptions::DrmDecryptFailed, source);
        }
        return decryptedFile;
    }
    return source;
}

void WidgetUnzip::unzipWgtFile(const std::string &destination)
{
    _D("Prepare to unzip...");
    Try
    {
        _D("wgtFile : %s", m_requestFile.c_str());
        _D("Widget package comment: %s", m_zip->GetGlobalComment().c_str());

        // Widget package must not be empty
        if (m_zip->empty()) {
            ThrowMsg(Exceptions::ZipEmpty, m_requestFile);
        }

        // Set iterator to first file
        m_zipIterator = m_zip->begin();

        unzipProgress(destination);

        // Unzip finished, close internal structures
        m_zip.reset();

        // Done
        _D("Unzip finished");
    }
    Catch(DPL::ZipInput::Exception::OpenFailed)
    {
        ReThrowMsg(Exceptions::OpenZipFailed, m_requestFile);
    }
    Catch(DPL::ZipInput::Exception::SeekFileFailed)
    {
        ThrowMsg(Exceptions::ExtractFileFailed, m_requestFile);
    }
    Catch(Exceptions::DrmDecryptFailed)
    {
        ReThrowMsg(Exceptions::ExtractFileFailed, m_requestFile);
    }
}

bool WidgetUnzip::checkAvailableSpace(const std::string &/*destination*/)
{
    _D("checkAvailableSpace ... ");

    double unCompressedSize = m_zip->GetTotalUncompressedSize();
    _D("unCompressedSize : %f", unCompressedSize);

    struct statvfs vfs;
    if (0 > storage_get_internal_memory_size(&vfs)) {
        _E("There is no space for installation");
        return false;
    }

    double freeSize = (double)vfs.f_bsize * vfs.f_bavail;
    _D("Space Size : %f", freeSize);

    if (unCompressedSize + SPACE_SIZE >= freeSize) {
        _E("There is no space for installation");
        return false;
    }
    return true;
}

void WidgetUnzip::unzipConfiguration(const std::string &destination,
        WrtDB::PackagingType* type)
{
    _D("unzipConfiguration");

    Try {
        _D("wgtFile : %s", m_requestFile.c_str());

        std::unique_ptr<DPL::ZipInput::File> configFile;

        Try {
            configFile.reset(m_zip->OpenFile(HYBRID_CONFIG_XML));
            *type = PKG_TYPE_HYBRID_WEB_APP;
        } Catch(DPL::ZipInput::Exception::OpenFileFailed) {
            configFile.reset(m_zip->OpenFile(WEB_APP_CONFIG_XML));
            *type = PKG_TYPE_NOMAL_WEB_APP;
        }

        std::string extractPath = destination + "/" + WEB_APP_CONFIG_XML;
        ExtractFile(configFile.get(), extractPath);
    }
    Catch(DPL::ZipInput::Exception::OpenFailed)
    {
        ReThrowMsg(Exceptions::OpenZipFailed, m_requestFile);
    }
    Catch(DPL::ZipInput::Exception::OpenFileFailed)
    {
        ThrowMsg(Exceptions::ExtractFileFailed, "config.xml");
    }
    Catch(Exceptions::DrmDecryptFailed)
    {
        ReThrowMsg(Exceptions::ExtractFileFailed, m_requestFile);
    }
}

} //namespace WidgetInstall
} //namespace Jobs
