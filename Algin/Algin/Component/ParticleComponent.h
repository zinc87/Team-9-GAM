#pragma once
#include "../Header/pch.h"
#include "../Graphics/Particles.h"


namespace AG {
	namespace Component {
		class ParticleComponent : public IComponent {
		public:
			static Type GetStaticType() { return Data::ComponentTypes::Particles; }

			using Self = ParticleComponent;
			
			ParticleComponent() : IComponent(Data::ComponentTypes::Particles) 
			{
				m_particle = std::make_shared<Particle>();

				m_particle->init();
			}

			ParticleComponent(ID id, ID objId) : IComponent(Data::ComponentTypes::Particles, id, objId)
			{
				m_particle = std::make_shared<Particle>();

				m_particle->init();
			}


			// Inherited via IComponent
			void Awake() override;

			void Start() override;

			void Update() override;

			void LateUpdate() override;

			void Free() override;

			void Inspector() override;

			//virtual std::shared_ptr<IComponent> Clone() override;
			void AssignFrom(const std::shared_ptr<IComponent>& fromCmp) override;

			std::string GetTypeName() const override { return "AG::Component::ParticleComponent"; }

			REFLECT() {
				return {
					REFLECT_FIELD(r_position, glm::vec3, "Emitter Position"),
					REFLECT_FIELD(r_speed, float, "Speed"),
					REFLECT_FIELD(r_upVel, float, "Up Velocity"),
					REFLECT_FIELD(r_grav, float, "Gravity"),
					REFLECT_FIELD(play, bool, "Play")
				};
			}

			REFLECT_SERIALIZABLE(ParticleComponent)

			glm::vec3 r_position{ 0.0f,  0.0f,  0.0f };
			float r_speed = 2.f;
			float r_upVel = 1.6f;
			float r_grav = 29.8f;
			bool play = true;

			void SyncToParticle();

		private:
			/*void SyncToParticle();*/

			std::shared_ptr<Particle> m_particle;

			/*glm::vec3 r_position{ 0.0f,  0.0f,  0.0f };
			float r_speed = 2.f;
			float r_upVel = 1.6f;
			float r_grav = 29.8f;
			bool play = true;*/
		};
	}
}