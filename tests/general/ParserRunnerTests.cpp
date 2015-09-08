/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file    ParserRunnerTests.cpp
 * @author  Adrian Szafranek (a.szafranek@samsung.com)
 * @version 1.0
 * @brief   Perform ParserRunner test's.
 */

#include <parser_runner.h>
#include <dpl/scope_guard.h>
#include <dpl/test/test_runner.h>
#include <dpl/utils/path.h>

#include <installer_log.h>

#include <string>
#include <fstream>
#include <cstdio>
#include <memory>

RUNNER_TEST_GROUP_INIT(ParserRunner)

using namespace DPL::Utils;

namespace {
const std::string rootTest = "/tmp/";
const std::string schemaFile = "/usr/etc/wrt-installer/widgets.xsd";
const std::string configFile = "/usr/lib/wrt-plugins/tizen-content/config.xml";
const std::string installConfigFile = "/opt/share/widget/tests/installer/configs/InstallConfig.xml";
}

class StreamRedirector
{
    public:
    StreamRedirector(FILE* src, std::string dest)
        : source(src), file_dest(dest)
    {
        fd = dup(fileno(source));
        if ((fd < 0) || (freopen(file_dest.c_str(), "w", source) == NULL))
        {
            _D("Error catching stream.");
        }
    }
    ~StreamRedirector()
    {
        if (TEMP_FAILURE_RETRY(fflush(source)) == 0)
        {
            if (dup2(fd, fileno(source)) >= 0)
            {
                close(fd);
                clearerr(source);
            }
            else
            {
                _D("Error returning stream.");
            }
        }
        else
        {
            _D("Error returning stream.");
        }
    }

    private:
    FILE* source;
    std::string file_dest;
    int fd;
};


/*
Name: parserRunner01
Description: Tests validation and parsing functionality.
Test performed for proper files existing in system, wrong files or empty parameters.
*/
RUNNER_TEST(parserRunner01)
{
    ParserRunner parser;

    std::unique_ptr<StreamRedirector> sd(new StreamRedirector(stderr, "/dev/null"));


    if (Path(installConfigFile).Exists() && Path(schemaFile).Exists()) {
        RUNNER_ASSERT(parser.Validate(installConfigFile, schemaFile) == true);
    }

    if (Path(configFile).Exists() && Path(schemaFile).Exists()) {
        RUNNER_ASSERT(parser.Validate(configFile, schemaFile) == false);
    }

    RUNNER_ASSERT(parser.Validate("", "") == false);
}

/*
Name: parserRunner02
Description: Tests validation and parsing functionality.
Test performed for wrong 'xml', empty 'xsd' file and wrong 'xml', non empty 'xsd' file.
*/
RUNNER_TEST(parserRunner02)
{
    ParserRunner parser;

    std::unique_ptr<StreamRedirector> sd(new StreamRedirector(stderr, "/dev/null"));


    Path pathF1 = Path(rootTest + "testFile1.xml");
    if (!pathF1.Exists()) {
        MakeEmptyFile(pathF1);

        DPL_SCOPE_EXIT(&pathF1) { TryRemove(pathF1); };

        std::ofstream ofs1;
        ofs1.open(pathF1.Fullpath(), std::ofstream::out);
        if (!ofs1) {
            RUNNER_ASSERT_MSG(false, "Error creating file");
        }
        ofs1 << "wrongContent";
        ofs1.close();

        RUNNER_ASSERT(parser.Validate(pathF1.Fullpath(), "") == false);
        if (Path(schemaFile).Exists()) {
            RUNNER_ASSERT(parser.Validate(pathF1.Fullpath(), schemaFile) == false);
        }
    }
}

/*
Name: parserRunner03
Description: Tests validation and parsing functionality.
Test performed for empty 'xml', wrong 'xsd' file and non empty 'xml', wrong 'xsd' file.
*/
RUNNER_TEST(parserRunner03)
{
    ParserRunner parser;

    std::unique_ptr<StreamRedirector> sd(new StreamRedirector(stderr, "/dev/null"));


    Path pathF2 = Path(rootTest + "testFile2.xsd");
    if (!pathF2.Exists()) {
        MakeEmptyFile(pathF2);

        DPL_SCOPE_EXIT(&pathF2) { TryRemove(pathF2); };

        std::ofstream ofs2;
        ofs2.open(pathF2.Fullpath(), std::ofstream::out);
        if (!ofs2) {
            RUNNER_ASSERT_MSG(false, "Error creating file");
        }
        ofs2 << "<element name=\"Test\" type=\"ds:TransformType\"/>";
        ofs2.close();

        RUNNER_ASSERT(parser.Validate("", pathF2.Fullpath()) == false);
        if (Path(configFile).Exists()) {
            RUNNER_ASSERT(parser.Validate(configFile, pathF2.Fullpath()) == false);
        }
    }
}
