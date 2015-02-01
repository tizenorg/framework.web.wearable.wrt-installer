/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file    LanguageSubtagRstTreeTests.cpp
 * @author  Zbigniew Kostrzewa (z.kostrzewa@samsung.com)
 * @version 1.0
 * @brief   Language tags tests
 */

#include <dpl/test/test_runner.h>
#include <language_subtag_rst_tree.h>

namespace {
const char LANGUAGE_TAG_VALID[] = "en-us";
const char LANGUAGE_TAG_INVALID[] = "invalid0";
}

////////////////////////////////////////////////////////////////////////////////

RUNNER_TEST_GROUP_INIT(LanguageSubtagRstTree)

/*
Name: ValidateLanguageTag_Valid
Description: tests result returned for valid language tag
Expected: value true should be returned
*/
RUNNER_TEST(ValidateLanguageTag_Valid)
{
  RUNNER_ASSERT(LanguageSubtagRstTreeSingleton::Instance().
          ValidateLanguageTag(LANGUAGE_TAG_VALID));
}

/*
Name: ValidateLanguageTag_Invalid
Description: tests result returned for invalid language tag
Expected: value false should be returned
*/
RUNNER_TEST(ValidateLanguageTag_Invalid)
{
  RUNNER_ASSERT(!LanguageSubtagRstTreeSingleton::Instance().
          ValidateLanguageTag(LANGUAGE_TAG_INVALID));
}
