/*
 * Copyright 2016 the original author or authors.
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

#include "speaker.h"

#include <algorithm>
#include <windows.h>
#include <sapi.h>
#include <sphelper.h>

using namespace Speech;
using namespace std;

class Speaker::PrivateSpeaker
{
public:
	CComPtr<ISpVoice> comSpeaker;
	CComPtr<IEnumSpObjectTokens> comVoices;
	CComPtr<ISpStream> comStream;
};

//TODO: CoMemTaskFree() - check in documentation
Speaker::Speaker(): d(new PrivateSpeaker)
{
	::CoInitialize(NULL);
	CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&(d->comSpeaker));
	SpEnumTokens(SPCAT_VOICES, L"", L"", &(d->comVoices));
	(d->comSpeaker)->SetOutput(NULL, TRUE);

	state = Ready;
	delayTime = 0;
}

Speaker::~Speaker()
{
	::CoUninitialize();
}

void Speaker::setVoice(Voice voice)
{
	CComPtr<ISpObjectToken> comVoice = NULL;

	wstring attributes = L"Name=" + voice.getName() + L";";
	attributes += L"Vendor=" + voice.getVendor() + L";";
	attributes += L"Language=" + voice.getLanguage() + L";";
	wstring optionalAttributes = L"";

	HRESULT result = SpFindBestToken(SPCAT_VOICES, attributes.c_str(), optionalAttributes.c_str(), &comVoice);
	if (SUCCEEDED(result))
	{
		voiceGuardian.lock();
		(d->comSpeaker)->SetVoice(comVoice);
		voiceGuardian.unlock();
	}
}

Voice Speaker::getVoice() const
{
	HRESULT result = S_OK;

	ULONG j = 0;
	CComPtr<ISpObjectToken> comVoice = NULL;
	CComPtr<ISpDataKey> comKey = NULL;
	LPWSTR keyName;
	LPWSTR name, vendor, language;

	(d->comSpeaker)->GetVoice(&comVoice);

	do
	{
		result = comVoice->EnumKeys(j, &keyName);
		if (!SUCCEEDED(result)) break;
		++j;
	} while (wcscmp(keyName, L"Attributes") != 0);
	j = 0;

	comVoice->OpenKey(keyName, &comKey);

	comKey->GetStringValue(L"Name", &name);
	comKey->GetStringValue(L"Vendor", &vendor);
	comKey->GetStringValue(L"Vendor", &language);

	return Voice(name, vendor, language);
}

void Speaker::setVoiceBy(wstring name)
{
	CComPtr<ISpObjectToken> comVoice = NULL;

	wstring attributes = L"Name=" + name + L";";
	wstring optionalAttributes = L"";

	HRESULT result = SpFindBestToken(SPCAT_VOICES, attributes.c_str(), optionalAttributes.c_str(), &comVoice);

	if (SUCCEEDED(result))
	{
		(d->comSpeaker)->SetVoice(comVoice);
	}
}

vector<Voice> Speaker::getAvailableVoices() const
{
	HRESULT result = S_OK;

	vector<Voice> voices;
	ULONG voiceCount;
	ULONG i = 0, j = 0;
	(d->comVoices)->GetCount(&voiceCount);

	CComPtr<ISpObjectToken> comVoice = NULL;
	CComPtr<ISpDataKey> comKey = NULL;
	LPWSTR keyName;
	LPWSTR name, vendor, language;

	while (i < voiceCount)
	{
		(d->comVoices)->Item(i, &comVoice);
		++i;

		do
		{
			result = comVoice->EnumKeys(j, &keyName);
			if (!SUCCEEDED(result)) break;
			++j;
		} while (wcscmp(keyName, L"Attributes") != 0);
		j = 0;

		comVoice->OpenKey(keyName, &comKey);

		comKey->GetStringValue(L"Name", &name);
		comKey->GetStringValue(L"Vendor", &vendor);
		comKey->GetStringValue(L"Language", &language);
		voices.push_back(Voice(name, vendor, language));

		comVoice.Release();
		comKey.Release();
	}

	return voices;
}

void Speaker::setOutputToAudioFile(wstring name)
{
	CSpStreamFormat audioFormat;
	audioFormat.AssignFormat(SPSF_22kHz16BitMono);
	size_t wavBeginIndex = name.length() - 5;

	if (name.find(L".wav", wavBeginIndex) == wstring::npos)
		name.append(L".wav");

	SPBindToFile(name.c_str(), SPFM_CREATE_ALWAYS, &(d->comStream), &audioFormat.FormatId(), audioFormat.WaveFormatExPtr());
	(d->comSpeaker)->SetOutput((d->comStream), TRUE);
}

void Speaker::setOutputToDefault()
{
	(d->comSpeaker)->SetOutput(NULL, NULL);
}

void Speaker::closeOutput()
{
	if ((d->comStream) != NULL)
		(d->comStream)->Close();
}

State Speaker::getState() const
{
	return state;
}

void Speaker::setVolume(unsigned int volume)
{
	if (volume > 100)
		volume = 100;

	(d->comSpeaker)->SetVolume(volume);

}

void Speaker::setRate(int rate)
{
	if (rate > 10)
		rate = 10;
	else if (rate < -10)
		rate = -10;

	(d->comSpeaker)->SetRate(rate);
}

void Speaker::setDelayTime(unsigned int time)
{
	delayTime = time;
}

unsigned int Speaker::getVolume() const
{
	USHORT volume;
	(d->comSpeaker)->GetVolume(&volume);

	return volume;
}

unsigned int Speaker::getRate() const
{
	long rate;
	(d->comSpeaker)->GetRate(&rate);

	return static_cast<int>(rate);
}

unsigned int Speaker::getDelayTime() const
{
	return delayTime;
}

void Speaker::pause()
{
	if (state == Speaking)
	{
		(d->comSpeaker)->Pause();
		state = Paused;
	}
}

void Speaker::resume()
{
	if (state == Paused)
	{
		(d->comSpeaker)->Resume();
		state = Speaking;
	}
}

void Speaker::stop()
{

	if (state == Speaking)
	{
		(d->comSpeaker)->Skip(L"SENTENCE", LONG_MAX, NULL);
		state = Ready;
	}
	else if (state == Paused)
	{
		resume();
		stop();
	}
}

void Speaker::waitFor(State someState)
{
	while (1)
	{
		if (state == someState)
			break;
	}
}

void Speaker::speak(std::wstring text)
{
	if (state == Ready)
	{
		state = Speaking;
		text += L"<silence msec = '" + to_wstring(delayTime * 1000) + L"' />";

		voiceGuardian.lock();
		(d->comSpeaker)->Speak(text.c_str(), SPF_ASYNC | SPF_IS_XML, NULL);

		(d->comSpeaker)->WaitUntilDone(INFINITE);
		voiceGuardian.unlock();
		state = Ready;
	}
}
