/*
 * Copyright 2002-2016 the original author or authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "voice.h"

using namespace Speech;
using namespace std;

Voice::Voice()
{
}

Voice::Voice(wstring name, wstring vendor, wstring language)
{
	this->name = name;
	this->vendor = vendor;
	this->language = language;
}

Voice::~Voice()
{
}

wstring Voice::getName() const
{
	return name;
}

wstring Voice::getVendor() const
{
	return vendor;
}

wstring Voice::getLanguage() const
{
	return language;
}



