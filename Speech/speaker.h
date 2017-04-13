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

#pragma once

#include "global.h"
#include "voice.h"
#include "state.h"

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>

namespace Speech
{
	class DLL_MAGIC Speaker
	{
	public:	
		Speaker();
		virtual ~Speaker();

		void speak(std::wstring text);
		void pause();
		void resume(); 	
		void stop();	
		void setVoiceBy(std::wstring name);
		void setOutputToDefault();
		void setOutputToAudioFile(std::wstring name);
		void closeOutput();
		void waitFor(State someState);
		std::vector<Voice> getAvailableVoices() const;
		State getState() const;

		void setVoice(Voice voice);
		Voice getVoice() const;

		void setVolume(unsigned int volume);
		unsigned int getVolume() const;

		void setRate(int rate);
		unsigned int getRate() const;

		void setDelayTime(unsigned int delayTime);
		unsigned int getDelayTime() const;

	private:
		Voice voice;
		std::atomic<State> state;
		unsigned int volume;
		int rate;
		std::atomic<unsigned int> delayTime;
		std::mutex voiceGuardian;

		class PrivateSpeaker;
		std::unique_ptr<PrivateSpeaker> d;
		friend class PrivateSpeaker;
	};
	
}
