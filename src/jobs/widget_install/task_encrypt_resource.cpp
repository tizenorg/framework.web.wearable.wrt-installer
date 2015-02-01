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
/**
 * @file    task_ecnrypt_resource.cpp
 * @author  Soyoung Kim (sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Implementation file for installer task encrypt resource
 */
#include "task_encrypt_resource.h"

#undef __USE_FILE_OFFSET64

#include <unistd.h>
#include <sys/stat.h>
#include <fts.h>
#include <string.h>
#include <errno.h>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <algorithm>

#include <memory>

#include <dpl/errno_string.h>
#include <dpl/foreach.h>
#include <dpl/scoped_fclose.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/string.h>
#include <ss_manager.h>

#include <widget_install/job_widget_install.h>
#include <widget_install/widget_install_context.h>
#include <widget_install/widget_install_errors.h>

#include <installer_log.h>

using namespace WrtDB;

namespace {
const std::size_t ENCRYPTION_CHUNK_MAX_SIZE = 8192; // bytes
const std::size_t ENCRYPTION_DEC_CHUNK_SIZE = 4; // bytes

std::set<std::string>& getSupportedForEncryption()
{
    static std::set<std::string> encryptSet;
    if (encryptSet.empty()) {
        encryptSet.insert(".html");
        encryptSet.insert(".htm");
        encryptSet.insert(".css");
        encryptSet.insert(".js");
    }
    return encryptSet;
}

bool isSupportedForEncryption(const std::string &file)
{
    size_t foundKey = file.rfind(".");
    if (std::string::npos != foundKey) {
        std::string mimeType = file.substr(foundKey);
        std::transform(mimeType.begin(), mimeType.end(), mimeType.begin(),
                       ::tolower);

        return getSupportedForEncryption().count(mimeType) > 0;
    }
    return false;
}

/**
 * Opens a file.
 *
 * @param path Path to a file.
 * @param mode Mode.
 * @return Stream handle.
 * @throw ExtractFileFailed If error (other than EINTR) occurs.
 */
FILE* openFile(const std::string& path, const std::string& mode)
{
    FILE* result = NULL;

    do
    {
        result = fopen(path.c_str(), mode.c_str());
    } while ((NULL == result) && (EINTR == errno));

    if (NULL == result)
    {
        ThrowMsg(Jobs::WidgetInstall::Exceptions::EncryptionFailed,
                 "Could not open file " << path);
    }

    return result;
}

/**
 * Reads bytes from a stream.
 *
 * @param buffer Buffer to read the bytes into.
 * @param count Number of bytes to read.
 * @param stream Stream to read from.
 * @return Number of bytes read
 * @throw ExtractFileFailed If error (other than EINTR) occurs.
 */
std::size_t readBytes(unsigned char* buffer, std::size_t count, FILE* stream)
{
    std::size_t result = std::fread(buffer,
                                    sizeof(unsigned char),
                                    count,
                                    stream);

    if (result != count)
    {
        int error = errno;
        if (0 != std::ferror(stream))
        {
            if (EINTR != error)
            {
                ThrowMsg(Jobs::WidgetInstall::Exceptions::ErrorExternalInstallingFailure,
                         "Error while reading data" <<
                         " [" << DPL::GetErrnoString(error) << "]");
            }
        }
    }

    return result;
}

/**
 * Writes bytes to a stream.
 *
 * @param buffer Data to write.
 * @param count Number of bytes.
 * @param stream Stream to write to.
 * @throw ExtractFileFailed If error (other than EINTR) occurs.
 */
void writeBytes(unsigned char* buffer, std::size_t count, FILE* stream)
{
    std::size_t bytesWritten = 0;
    std::size_t bytesToWrite = 0;
    do
    {
        bytesToWrite = count - bytesWritten;
        bytesWritten = std::fwrite(buffer + bytesWritten,
                                   sizeof(unsigned char),
                                   count - bytesWritten,
                                   stream);
        if ((bytesWritten != bytesToWrite) && (EINTR != errno))
        {
            int error = errno;
            ThrowMsg(Jobs::WidgetInstall::Exceptions::EncryptionFailed,
                     "Error while writing data" <<
                     " [" << DPL::GetErrnoString(error) << "]");
        }
    } while ((bytesWritten != bytesToWrite) && (EINTR == errno));
}

int ssmEncrypt(InstallMode::InstallTime time, std::string pkgId, const char*
        inChunk, int inBytes, char** outChunk, int *outBytes)
{
    int isPreload = 0;

    if (time == InstallMode::InstallTime::CSC ||
        time == InstallMode::InstallTime::PRELOAD ||
        time == InstallMode::InstallTime::FOTA) {
        isPreload = 1;
    }

    *outBytes = ssa_encrypt_web_application(pkgId.c_str(),pkgId.size(), inChunk, inBytes, outChunk, isPreload);
    if( *outBytes > 0 )
        return 0;
    else
        return *outBytes;
}

}

namespace Jobs {
namespace WidgetInstall {
TaskEncryptResource::TaskEncryptResource(InstallerContext& context) :
    DPL::TaskDecl<TaskEncryptResource>(this),
    m_context(context)
{
    AddStep(&TaskEncryptResource::StartStep);
    AddStep(&TaskEncryptResource::StepEncryptResource);
    AddStep(&TaskEncryptResource::EndStep);
}

void TaskEncryptResource::StepEncryptResource()
{
    _D("Step Encrypt resource");

    EncryptDirectory(m_context.locations->getSourceDir());
}

void TaskEncryptResource::EncryptDirectory(std::string path)
{
    FTS *fts;
    FTSENT *ftsent;
    char * const paths[] = { const_cast<char * const>(path.c_str()), NULL };

    if ((fts = fts_open(paths, FTS_PHYSICAL | FTS_NOCHDIR, NULL)) == NULL) {
        //ERROR
        int error = errno;
        _W("%s: fts_open failed with error: %s", __PRETTY_FUNCTION__, strerror(error));
        ThrowMsg(Exceptions::EncryptionFailed, "Error reading directory: "
                 << path);
    }

    while ((ftsent = fts_read(fts)) != NULL) {
        switch (ftsent->fts_info) {
        case FTS_DP:
        case FTS_DC:
        case FTS_D:
        case FTS_DEFAULT:
        case FTS_SLNONE:
            //directories, non-regular files, dangling symbolic links
            break;
        case FTS_F:
        case FTS_NSOK:
        case FTS_SL:
            //regular files and other objects that can be counted
            if (isSupportedForEncryption(ftsent->fts_path)) {
                EncryptFile(ftsent->fts_path);
            }
            break;
        case FTS_NS:
        case FTS_DOT:
        case FTS_DNR:
        case FTS_ERR:
        default:
            _W("%s: traversal failed on file: %s with error: %s", __PRETTY_FUNCTION__, ftsent->fts_path, strerror(ftsent->fts_errno));
            ThrowMsg(Exceptions::EncryptionFailed, "Error reading file");
            break;
        }
    }

    if (fts_close(fts) == -1) {
        int error = errno;
        _W("%s: fts_close failed with error: %s", __PRETTY_FUNCTION__, strerror(error));
    }
}

void TaskEncryptResource::EncryptFile(const std::string &fileName)
{
    _D("Encrypt file: %s", fileName.c_str());
    std::string encFile = fileName + ".enc";

    struct stat info;
    memset(&info, 0, sizeof(info));
    if (stat(fileName.c_str(), &info) != 0)
    {
        int error = errno;
        ThrowMsg(Exceptions::EncryptionFailed,
                "Could not access file " << fileName <<
                "[" << DPL::GetErrnoString(error) << "]");
    }
    const std::size_t fileSize = info.st_size;
    if (0 == fileSize) {
        _D("%s size is 0, so encryption is skiped", fileName.c_str());
        return;
    }

    // If update installed preload web, should skip encryption.
    if (!(m_context.mode.rootPath == InstallMode::RootPath::RO &&
                (m_context.mode.installTime == InstallMode::InstallTime::PRELOAD
                 || m_context.mode.installTime == InstallMode::InstallTime::FOTA)
                && m_context.mode.extension == InstallMode::ExtensionType::DIR)) {

        DPL::ScopedFClose inFile(openFile(fileName, "r"));
        DPL::ScopedFClose outFile(openFile(encFile, "w"));

        const std::size_t chunkSize = (fileSize > ENCRYPTION_CHUNK_MAX_SIZE
                ? ENCRYPTION_CHUNK_MAX_SIZE : fileSize);

        std::unique_ptr<unsigned char[]> inChunk(new unsigned char[chunkSize]);
        std::size_t bytesRead = 0;
        std::string pkgId = DPL::ToUTF8String(m_context.widgetConfig.tzPkgid);

        do
        {
            bytesRead = readBytes(inChunk.get(), chunkSize, inFile.Get());
            if (0 != bytesRead) {
                int outDecSize = 0;
                char *outChunk = NULL;
                if (0 != ssmEncrypt(m_context.mode.installTime, pkgId,
                            (char*)inChunk.get(), (int)bytesRead,
                            &outChunk, &outDecSize)) {
                    ThrowMsg(Exceptions::EncryptionFailed,
                            "Encryption Failed using TrustZone");
                }

                if (outDecSize <= 0 ) {
                    ThrowMsg(Exceptions::EncryptionFailed,
                            "Encryption Failed using TrustZone");
                }

                std::stringstream toString;
                toString << outDecSize;

                writeBytes((unsigned char*)toString.str().c_str(),
                        sizeof(int), outFile.Get());
                writeBytes((unsigned char*)outChunk, outDecSize, outFile.Get());
                delete outChunk;
            }
            inChunk.reset(new unsigned char[chunkSize]);

        } while (0 == std::feof(inFile.Get()));

        outFile.Reset();
        inFile.Reset();

        _D("File encrypted successfully");
        _D("Remove plain-text file: %s", fileName.c_str());
        if (0 != unlink(fileName.c_str()))
        {
            Throw(Exceptions::EncryptionFailed);
        }

        _D("Rename encrypted file");
        if (0 != std::rename(encFile.c_str(), fileName.c_str()))
        {
            Throw(Exceptions::EncryptionFailed);
        }
    }

    WrtDB::EncryptedFileInfo fileInfo;
    fileInfo.fileName = DPL::FromUTF8String(fileName);
    fileInfo.fileSize = fileSize;

    m_context.widgetConfig.encryptedFiles.insert(fileInfo);
}

void TaskEncryptResource::StartStep()
{
    LOGI("--------- <TaskEncryptResource> : START ----------");
}

void TaskEncryptResource::EndStep()
{
    m_context.job->UpdateProgress(
            InstallerContext::INSTALL_ECRYPTION_FILES,
            "Ecrypt resource files");

    LOGI("--------- <TaskEncryptResource> : END ----------");
}
} //namespace WidgetInstall
} //namespace Jobs
