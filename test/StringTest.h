/****************************************************************************
* Copyright 2014-2015 Trefilov Dmitrij                                      *
*                                                                           *
* Licensed under the Apache License, Version 2.0 (the "License");           *
* you may not use this file except in compliance with the License.          *
* You may obtain a copy of the License at                                   *
*                                                                           *
*    http://www.apache.org/licenses/LICENSE-2.0                             *
*                                                                           *
* Unless required by applicable law or agreed to in writing, software       *
* distributed under the License is distributed on an "AS IS" BASIS,         *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
* See the License for the specific language governing permissions and       *
* limitations under the License.                                            *
****************************************************************************/
#ifndef STRINGTEST_H
#define STRINGTEST_H
#include "String.h"
#include <gtest/gtest.h>

class StringTest : public testing::Test
{
public:
	void testNull();
	void testRefCount();
	void testDetach();
	void testStl();
	void testAppend(const char *a, const char *b, const char *result);
	void testArrayOfStrings();
	void testFindSubstr();
	void testSubStr2();
	void testStringBuilder();
    void testAssign(const char *v);
    void testStreams(const char *str);
    void testJoin();
    void testMap();
};
#endif // STRINGTEST_H
