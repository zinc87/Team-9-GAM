#pragma once
#include "pch.h"

constexpr int MAX_CHANNEL = 512;

namespace AG {
	class AudioSystem : public Pattern::ISingleton<AudioSystem> {
	public:
		AudioSystem() { Init(); }
		~AudioSystem() {}
		void Init();
		void Start();
		void Update();
		void End();

		void loadBankFile(std::filesystem::path filePath, FMOD::Studio::Bank* bankptr);
		void unloadBank(FMOD::Studio::Bank* bankptr);
		void getAllEvents(FMOD::Studio::Bank* bankptr);

		void setBusVolume(std::string busName, float volume);
		float getBusVolume(std::string busName);

		
		FMOD::Studio::EventDescription* getEvents(const std::string& eventPath) const;

		void printBankEvents();

		//getter
		const std::unordered_map<std::string, FMOD::Studio::EventDescription*>& GetAllEvents() const {
			return nameToEvent;
		}

		FMOD::System* GetCoreSystem() 
		{ 
			FMOD::System* core = nullptr;
			if (audioSystem)
				audioSystem->getCoreSystem(&core);
			return core;
		}

	private:
		void updateAttributes();
		void registerAllBus(FMOD::Studio::Bank* bankptr);
	private:
		FMOD::Studio::System* audioSystem = nullptr;
		std::unordered_map <FMOD::Studio::Bank*, std::vector<std::string>> bankToEventName;
		std::unordered_map<std::string, FMOD::Studio::EventDescription*> nameToEvent;
		std::unordered_map<std::string, FMOD::Studio::Bus*> nameToBus;

		glm::vec3 lastPos; //since this is the first person game, camera always be the listener
	};
}
#define AUDIOSYSTEM AG::AudioSystem::GetInstance()


