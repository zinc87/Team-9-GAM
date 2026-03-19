#include "pch.h"
#include "Audio.h"

namespace AG {
	void AudioSystem::Init()
	{
		FMOD_RESULT result;

		result = FMOD::Studio::System::create(&audioSystem); 
		if (result != FMOD_OK)
		{
			std::cerr << "FMOD error! (" << result << ") " << FMOD_ErrorString(result);
			//Some Error Handling
		}

		// Initialize the Studio system, which also initializes the Core system
		result = audioSystem->initialize(MAX_CHANNEL, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0);
		if (result != FMOD_OK)
		{
			std::cerr << "FMOD error! (" << result << ") " << FMOD_ErrorString(result);
			//Some Error Handling
		}
	}
	void AudioSystem::Start()
	{

		FMOD::Studio::Bank* masterBank = nullptr;
		FMOD_RESULT result = audioSystem->getBank("bank:/Master", &masterBank);
		if (result == FMOD_OK) {
			registerAllBus(masterBank);
		}

		

		printBankEvents();

		// Debugging the Master Bank contents
		int count = 0;
		masterBank->getEventCount(&count);
		std::cout << "[AUDIO DEBUG] Master Bank contains " << count << " events." << std::endl;

		// List the names found
		FMOD::Studio::EventDescription* eventList[512];
		int actualCount = 0;
		masterBank->getEventList(eventList, 512, &actualCount);

		for (int i = 0; i < actualCount; i++) {
			char path[256];
			eventList[i]->getPath(path, 256, nullptr);
			std::cout << "[AUDIO DEBUG] Event Found: " << path << std::endl;
		}
	}
	void AudioSystem::Update()
	{
		updateAttributes();

		audioSystem->update();
	}
	void AudioSystem::End()
	{
		if (audioSystem) {
			audioSystem->release();
			audioSystem = nullptr;
		}
	}
	void AudioSystem::loadBankFile(std::filesystem::path filePath, FMOD::Studio::Bank* bankptr)
	{
		FMOD_RESULT result = audioSystem->loadBankFile(filePath.string().c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bankptr);
		
		if (result != FMOD_OK)
		{
			AG_CORE_WARN("FMOD error! ({}), {}", (int)result, FMOD_ErrorString(result));
		}
		
		bankptr->loadSampleData();
		getAllEvents(bankptr);
	}
	void AudioSystem::unloadBank(FMOD::Studio::Bank* bankptr)
	{
		if (!bankptr) return;

		auto it = bankToEventName.find(bankptr);
		if (it != bankToEventName.end())
		{
			for (auto& path : it->second)
			{
				nameToEvent.erase(path);
			}
			bankToEventName.erase(it);
		}

		bankptr->unload();
	}
	void AudioSystem::getAllEvents(FMOD::Studio::Bank* bankptr)
	{
		if (!bankptr) return;

		int eventCount = 0;
		bankptr->getEventCount(&eventCount);

		if (eventCount == 0) return;

		std::vector<FMOD::Studio::EventDescription*> eventDescriptions(eventCount);
		FMOD_RESULT result = bankptr->getEventList(eventDescriptions.data(), eventCount, nullptr);
		if (result != FMOD_OK) {
			//Some Error Handling
			std::cerr << "FMOD error! (" << result << ") " << FMOD_ErrorString(result);
		}

		std::vector<std::string> eventData;
		eventData.reserve(eventCount);

		for (auto& e : eventDescriptions)
		{
			char pathBuffer[512];
			int retrieved = 0;

			result = e->getPath(pathBuffer, sizeof(pathBuffer), &retrieved);
			if (result == FMOD_OK)
			{
				eventData.emplace_back(pathBuffer);
				nameToEvent[std::string{ pathBuffer }] = e;
			}
		}

		bankToEventName[bankptr] = std::move(eventData);
	}

	void AudioSystem::setBusVolume(std::string busName, float volume)
	{
		//bus Name full name (bus:/SFX...)
		if (nameToBus.find(busName) == nameToBus.end()) {
			AG_CORE_WARN("Bus not found: {}", busName);
			return;
		}
		nameToBus[busName]->setVolume(volume);
	}

	float AudioSystem::getBusVolume(std::string busName)
	{
		float volume{ -1.f };
		if (nameToBus.find(busName) == nameToBus.end()) {
			AG_CORE_WARN("Bus not found: {}", busName);
			return volume;
		}
		nameToBus[busName]->getVolume(&volume);
		return volume;
	}

	// hook listener to game camera
	void AudioSystem::updateAttributes()
	{
		FMOD_3D_ATTRIBUTES attributes{ };
		std::weak_ptr<Camera> activeCamera = CAMERAMANAGER.getGameCamera().expired() ? CAMERAMANAGER.getSceneCamera() : CAMERAMANAGER.getGameCamera();
		
		// pos
		auto pos = activeCamera.lock()->getCameraPosition();
		attributes.position = { pos.x, pos.y, pos.z };

		// forward orientation 
		auto forward = activeCamera.lock()->getCameraDirection();
		attributes.forward = { forward.x, forward.y, forward.z };
		
		// up
		auto up = activeCamera.lock()->getCameraUp();
		attributes.up = { up.x, up.y, up.z };

		// velocity for track camera movement
		auto velocity = (pos - lastPos) / (float)BENCHMARKER.GetDeltaTime();
		lastPos = pos;
		attributes.velocity = { velocity.x, velocity.y, velocity.z };

		audioSystem->setListenerAttributes(0, &attributes);

		return;
	}

	void AudioSystem::registerAllBus(FMOD::Studio::Bank* bankptr)
	{
		int busCount = 0;
		// First, get number of buses
		FMOD_RESULT result = bankptr->getBusCount(&busCount);
		if (result != FMOD_OK || busCount == 0) {
			return;
		}
		std::vector<FMOD::Studio::Bus*> buses(busCount);
		bankptr->getBusList(buses.data(), busCount, &busCount);

		for (auto* bus : buses)
		{
			char path[256] = {};
			result = bus->getPath(path, sizeof(path), nullptr);
			if (result == FMOD_OK)
			{
				std::string busName(path); // bus:/SFX, bus:/Music, etc.
				nameToBus[busName] = bus;
			}
		}
		
	}


	FMOD::Studio::EventDescription* AudioSystem::getEvents(const std::string& eventPath) const
	{
		auto it = nameToEvent.find(eventPath);
		if (it != nameToEvent.end()) {
			return it->second;
		}

		return nullptr;

	}
	void AudioSystem::printBankEvents()
	{
		if (bankToEventName.empty()) {
			AG_DEBUG_INFO("[AudioSystem] No loaded banks or events.");
			return;
		}

		AG_DEBUG_INFO("=== Loaded Audio Banks and Events ===");

		for (auto& bankPair : bankToEventName)
		{
			FMOD::Studio::Bank* bank = bankPair.first;
			std::vector<std::string>& events = bankPair.second;

			// --- Get the bank path ---
			char bankPathBuffer[512];
			int retrieved = 0;
			FMOD_RESULT result = bank->getPath(bankPathBuffer, sizeof(bankPathBuffer), &retrieved);

			if (result != FMOD_OK) {
				AG_CORE_WARN("[AudioSystem] Failed to retrieve bank path.FMOD Error : {}", FMOD_ErrorString(result));
				continue;
			}

			AG_DEBUG_INFO("Bank: {} (Events: {})", bankPathBuffer, events.size());

			// --- Print each event in the bank ---
			for (size_t i = 0; i < events.size(); ++i)
			{
				// Print event index and path
				AG_CORE_INFO("  [{}] {}", i, events[i]);
			}
		}
	}
}