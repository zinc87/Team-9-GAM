#pragma once
#include "pch.h"

namespace AG {
	namespace Component {
		class AudioComponent : public IComponent {
		public:
			using Self = AudioComponent;

			AudioComponent() :
				IComponent(Data::ComponentTypes::Audio) {
			}

			AudioComponent(ID id, ID objId) :
				IComponent(Data::ComponentTypes::Audio, id, objId) {
			}
			// --- ADD THIS STATIC FUNCTION ---
			static Type GetStaticType() { return Data::ComponentTypes::Audio; }


			// Inherited via IComponent
			void Awake() override;

			void Start() override;

			void Update() override;

			void LateUpdate() override;

			void Free() override;

			void Inspector() override;

			//std::shared_ptr<IComponent> Clone() override;
			void AssignFrom(const std::shared_ptr<IComponent>& other) override;

			std::string getEvent(std::string event);
			void PlayEvent(std::string event);
			void pauseEvent(std::string event);
			void StopEvent(std::string event);
			void setVolume(std::string event, float volume);
			float getVolume(std::string event);
			void setParameter(std::string event, std::string paramName, float value);
			float getParameter(std::string event, std::string paramName);

			void AddEvent(std::string newEventPath);
			void RemoveEvent(std::string eventPath);

			// === Debug === //
			void PrintData();

			std::string GetTypeName() const override { return "AG::Component::AudioComponent"; }

			static std::vector<const char*> GetEventItems();

			// Reflection definition
			REFLECT() {
				return {
					REFLECT_FIELD(allEventsPath, std::vector<std::string>, "Audio Events"),
				};
			}

			REFLECT_SERIALIZABLE(AG::Component::AudioComponent);

		private:
			FMOD_3D_ATTRIBUTES GetAttributes();
		private:
			std::unordered_map<std::string, FMOD::Studio::EventInstance*> eventPathToInstance;
			//std::unordered_map<std::string, EventSettings> eventPathToSettings;
			std::vector<std::string> allEventsPath;
			int selectedEventIndex = 0;
		};
	}
}
