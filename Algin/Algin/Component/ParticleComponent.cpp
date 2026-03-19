#include "pch.h"
#include "ParticleComponent.h"

using namespace AG;

void AG::Component::ParticleComponent::Awake(){}


void AG::Component::ParticleComponent::Start(){}

void AG::Component::ParticleComponent::Update() 
{
    auto trf_weak = GetObj().lock()->GetComponent<Component::TransformComponent>();
    auto trf_shared = trf_weak.lock();
    if (!trf_shared)
    {
        AG_CORE_WARN("Light does not have Transform Component.");
        return;
    }

    m_particle->emitterPosition = trf_shared->GetTransformation().position;
    r_position = m_particle->emitterPosition;

    if (play) {
        m_particle->updateTest(float(BENCHMARKER.GetDeltaTime()));
    }

    SyncToParticle();
}

void AG::Component::ParticleComponent::LateUpdate()
{
    m_particle->drawParticles();
}

void AG::Component::ParticleComponent::Free(){}

void AG::Component::ParticleComponent::Inspector() 
{   

    AG::RenderFieldsInspector(this, Self::GetFields());
    if (ImGui::Button("Start")) {
        play = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        play = false;
    }

    SyncToParticle();
}

void AG::Component::ParticleComponent::AssignFrom([[maybe_unused]] const std::shared_ptr<IComponent>& fromCmp){}

void AG::Component::ParticleComponent::SyncToParticle()
{
    if (!m_particle) return;
    m_particle->emitterPosition = r_position;
    m_particle->speedControl = r_speed;
    m_particle->gravControl = r_grav;
    m_particle->upVelControl = r_upVel;
}

REGISTER_REFLECTED_TYPE(AG::Component::ParticleComponent)
