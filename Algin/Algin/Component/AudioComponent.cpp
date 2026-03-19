#include "pch.h"
#include "AudioComponent.h"

namespace AG {
	namespace Component {
		std::vector<const char*> AudioComponent::GetEventItems() {
			std::vector<const char*> cptrs;

			const auto& events = AUDIOSYSTEM.GetAllEvents();
			cptrs.reserve(events.size());
			for (auto& [path, _] : events) {
				cptrs.push_back(path.c_str());
			}

			return cptrs;
		}

		void AudioComponent::Awake()
		{
			for (auto& eventPath : allEventsPath) {
				eventPathToInstance.emplace(std::string{ eventPath }, nullptr);
			}

			for (auto& event : eventPathToInstance) {
				FMOD::Studio::EventDescription* eventDesciption = AUDIOSYSTEM.getEvents(event.first);
				FMOD::Studio::EventInstance* eventInstance;
				FMOD_RESULT result = eventDesciption->createInstance(&eventInstance);
				if (result != FMOD_OK)
				{
					std::string temp{ FMOD_ErrorString(result) };
					AG_CORE_ERROR("[AudioComponent] Failed to create event instance ({}): {}", (int)result, temp);
				}
				event.second = eventInstance;
			}
		}

		void AudioComponent::Start()
		{
		}

		void AudioComponent::Update()
		{
			FMOD_3D_ATTRIBUTES attributes = GetAttributes();
			
			// update 3D attributes and settings for each event instance
			for (auto& e : eventPathToInstance) {
				FMOD::Studio::EventInstance* instance = e.second;

				if (instance) {
					instance->set3DAttributes(&attributes);
				}
			}
		}

		void AudioComponent::LateUpdate()
		{
		}

		void AudioComponent::Free()
		{
			for (auto& e : eventPathToInstance) {
				if (e.second) {
					StopEvent(e.first);
					e.second->release();
					e.second = nullptr;
				}
			}
		}

		/* ////////////////////////////////////////////////////////////
		Pulls all loaded FMOD events from the audio banks (AudioSystem).
		Displays them in a combo box in the Inspector.
		When you select one, it�s added to the component via AddEvent().
		Play/Stop buttons
		//////////////////////////////////////////////////////////// */
		void AudioComponent::Inspector()
		{
			// --- Get event items from the reflection provider ---
			auto items = Self::GetEventItems();
			if (items.empty()) {
				ImGui::Text("No audio events loaded from banks");
				AG_CORE_WARN("[AudioComponent] No audio events loaded from AudioSystem!");
				return;
			}

			// Calculate preview based on current selection index
			const char* previewValue = (selectedEventIndex >= 0 && selectedEventIndex < items.size())
				? items[selectedEventIndex]
				: items[0]; // Fallback if index invalid

			static ImGuiTextFilter filter;

			// Choosing event here (Searchable)
			if (ImGui::BeginCombo("##AudioEventCombo", previewValue))
			{
				// Search Bar
				// -1.0f width means it spans the full available width
				filter.Draw("##AudioSearch", -1.0f);
				ImGui::Separator();

				for (int i = 0; i < items.size(); i++)
				{
					// Apply the filter
					if (filter.PassFilter(items[i]))
					{
						bool isSelected = (i == selectedEventIndex);
						if (ImGui::Selectable(items[i], isSelected)) {
							selectedEventIndex = i;
						}

						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}
					}
				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();

			// Add Event button
			bool eventAdded = false;
			if (ImGui::Button("Add Event")) {
				for (auto& event : allEventsPath) {
					if (event == items[selectedEventIndex]) {
						eventAdded = true;
						break;
					}
				}
				if (!eventAdded) {
					allEventsPath.emplace_back(items[selectedEventIndex]);
				}
				//eventPathToInstance.emplace(items[selectedEventIndex], nullptr);
			}

			ImGui::Separator();

			ImGui::Text("Added Event");
			// List of current Event
			if (allEventsPath.empty())
			{
				ImGui::TextDisabled("No events assigned");
			}
			else
			{
				for (auto it = allEventsPath.begin(); it != allEventsPath.end();)
				{
					std::string eventName = *it;

					ImGui::Selectable(eventName.c_str());

					// Right-click to remove
					if (ImGui::BeginPopupContextItem(eventName.c_str()))
					{
						if (ImGui::MenuItem("Remove"))
						{
							it = allEventsPath.erase(it);
							ImGui::EndPopup();
							continue; // skip iterator increment
						}
						ImGui::EndPopup();
					}
					++it;
				}
			}
		}
		
		//To be Test!!!!
		void AudioComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp) {
			ASSIGNFROM_FN_BODY(
				allEventsPath.clear();
				allEventsPath.reserve(fromCmpPtr->allEventsPath.size());
				for (auto& event : fromCmpPtr->allEventsPath) {
					allEventsPath.emplace_back(event);
				}
			)

		}

		std::string AudioComponent::getEvent(std::string event)
		{
			for (auto& eventPath : eventPathToInstance) {
				if (eventPath.first.find(event) != std::string::npos) {
					return eventPath.first;
				}
			}
			return "";
		}

		void AudioComponent::PlayEvent(std::string event)
		{
			auto it = eventPathToInstance.find(event);
			if (it == eventPathToInstance.end() || !it->second) {
				return;
			}

			FMOD::Studio::EventInstance* instance = it->second;

			bool paused = false;
			instance->getPaused(&paused);
			if (paused) {
				instance->setPaused(false);
			}

			FMOD_STUDIO_PLAYBACK_STATE state;
			if (instance->getPlaybackState(&state) == FMOD_OK)
			{
				if (state == FMOD_STUDIO_PLAYBACK_PLAYING)
					return;
			}

			instance->start();
		}

		void AudioComponent::pauseEvent(std::string event)
		{
			auto it = eventPathToInstance.find(event);
			if (it == eventPathToInstance.end() || !it->second) {
				return;
			}

			FMOD::Studio::EventInstance* instance = it->second;

			FMOD_STUDIO_PLAYBACK_STATE state;
			if (instance->getPlaybackState(&state) == FMOD_OK)
			{
				if (state == FMOD_STUDIO_PLAYBACK_PLAYING) {
					instance->setPaused(true);
				}
			}
		}

		void AudioComponent::StopEvent(std::string event)
		{
			auto it = eventPathToInstance.find(event);
			if (it == eventPathToInstance.end() || !it->second) {
				return;
			}

			FMOD::Studio::EventInstance* instance = it->second;

			instance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
			
		}

		void AudioComponent::setVolume(std::string event, float volume)
		{
			auto it = eventPathToInstance.find(event);
			if (it == eventPathToInstance.end() || !it->second) return;

			FMOD::Studio::EventInstance* instance = it->second;

			instance->setVolume(volume);
		}

		float AudioComponent::getVolume(std::string event)
		{
			auto it = eventPathToInstance.find(event);
			if (it == eventPathToInstance.end() || !it->second) {
				return -1.0f;
			}

			FMOD::Studio::EventInstance* instance = it->second;

			float volume = 0.0f;

			FMOD_RESULT result = instance->getVolume(&volume, nullptr);
			if (result != FMOD_OK) {
				std::string temp{};
				AG_CORE_ERROR("[AudioComponent] Failed to get volume ({}): {}", (int)result, temp);
				return -1.0f;
			}
				

			return volume;
		}

		void AudioComponent::setParameter(std::string event, std::string paramName, float value)
		{
			auto it = eventPathToInstance.find(event);
			if (it == eventPathToInstance.end() || !it->second) {
				return;
			}
			FMOD::Studio::EventInstance* instance = it->second;

			// Set parameter by name (FMOD Studio)
			FMOD_RESULT result = instance->setParameterByName(paramName.c_str(), value);
			if (result != FMOD_OK)
			{
				AG_CORE_ERROR("[AudioComponent] Failed to get volume ({}): {}", (int)result, FMOD_ErrorString(result));
			}
		}

		float AudioComponent::getParameter(std::string event, std::string paramName)
		{
			float value = -1.0f;
			auto it = eventPathToInstance.find(event);
			if (it == eventPathToInstance.end() || !it->second) return value;

			FMOD::Studio::EventInstance* instance = it->second;

			

			FMOD_RESULT result = instance->getParameterByName(paramName.c_str(), &value);
			(void)result;
			return value;
		}

		void AudioComponent::AddEvent(std::string newEventPath)
		{
			if (eventPathToInstance.find(newEventPath) != eventPathToInstance.end()) return;

			//Register into the map
			eventPathToInstance[newEventPath] = nullptr;
		}

		void AudioComponent::RemoveEvent(std::string eventPath)
		{
			if (eventPathToInstance.find(eventPath) == eventPathToInstance.end()) return;
			eventPathToInstance.erase(eventPath);
		}

		void AudioComponent::PrintData()
		{
		}

		// hook events to entity
		FMOD_3D_ATTRIBUTES AudioComponent::GetAttributes()
		{
			//TBC Need entity postition
			//return FMOD_3D_ATTRIBUTES();

			FMOD_3D_ATTRIBUTES attributes = {};
			// entity transform
			auto trf_weak = GetObj().lock()->GetComponent<Component::TransformComponent>();
			auto trf_shared = trf_weak.lock();
			if (trf_shared)
			{
				auto pos = trf_shared->GetTransformation().position;
				auto rot = trf_shared->GetTransformation().rotation; // for forward, up

				// position
				attributes.position = { -pos.x, pos.y, pos.z };

				// orientation
				attributes.forward = { 0.0f, 0.0f, 1.0f };
				attributes.up = { 0.0f, 1.0f, 0.0f };
			}
			else {
				attributes.position = { 0.0f, 0.0f, 0.0f };
				attributes.forward = { 0.0f, 0.0f, 1.0f };
				attributes.up = { 0.0f, 1.0f, 0.0f };
			}
			attributes.velocity = { 0.0f, 0.0f, 0.0f };
		
			return attributes;
		}
	}
}

REGISTER_REFLECTED_TYPE(AG::Component::AudioComponent)
