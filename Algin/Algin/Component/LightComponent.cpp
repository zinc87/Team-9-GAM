#include "pch.h"
#include "LightComponent.h"

using namespace AG;

void AG::Component::LightComponent::Awake()
{
    //light = std::make_shared<Light>();
    //LightManager::GetInstance().addLight(light);

    //SyncToLight();
}

void AG::Component::LightComponent::Start() {}

void AG::Component::LightComponent::Update()
{
    light->disabled = !IsEnable();

    auto trf_weak = GetObj().lock()->GetComponent<Component::TransformComponent>();
    auto trf_shared = trf_weak.lock();
    if (!trf_shared)
    {
        AG_CORE_WARN("Light does not have Transform Component.");
        return;
    }

    // Transform drives position
    light->position = trf_shared->GetTransformation().position;
    r_position = light->position;

    // Keep mirrors in sync with any runtime changes to the light
    //SyncFromLight();
}

void AG::Component::LightComponent::LateUpdate()
{
    SyncToLight();
    auto state = RenderPipeline::GetInstance().GetPipeline();
    ComputeCos();

    auto trf = GetObj().lock()->GetComponent<Component::TransformComponent>().lock();
    if (!trf) return;

    if (state == RenderPipeline::OBJPICK) {
        Data::GUID objID = this->GetObjID();
        DebugRenderer::GetInstance().DrawCubeColor(trf->getM2W(), objID);
        return;
    }

    if (CAMERAMANAGER.getCurrentCamera().lock() == CAMERAMANAGER.getGameCamera().lock())
        return;

    if (state == RenderPipeline::LIGHT) {
        DebugRenderer::GetInstance().DrawCube(trf->getM2W());
    }
}

void AG::Component::LightComponent::Free() 
{
    if (light)
    {
        light.reset();
    }
}

void AG::Component::LightComponent::Inspector()
{
    SyncToLight();
    // Draw UI from mirrors
    AG::RenderFieldsInspector(this, Self::GetFields());

    SyncToLight();
}

//To be Tested!!!!
void AG::Component::LightComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp) {
    ASSIGNFROM_FN_BODY(
        this->r_position = fromCmpPtr->r_position;
        this->r_direction = fromCmpPtr->r_direction;
        this->r_range = fromCmpPtr->r_range;
        this->r_intensity = fromCmpPtr->r_intensity;
        this->r_fov = fromCmpPtr->r_fov;
        this->r_innerCos = fromCmpPtr->r_innerCos;
        this->r_outerCos = fromCmpPtr->r_outerCos;
        this->r_type_index = fromCmpPtr->r_type_index;
    )
}

// ---------- Serialization ----------

bool AG::Component::LightComponent::Serialize(std::ostream& out) const
{
    // Use reflection to write all mirrored fields in a single block
    std::ostringstream temp(std::ios::binary);
    if (!AG::SerializeFields(temp, const_cast<LightComponent*>(this), Self::GetFields()))
        return false;

    const std::string buf = temp.str();
    return IO::WriteUInt32(out, LIGHT_DATA_HASH)
        && IO::WriteUInt32(out, static_cast<uint32_t>(buf.size()))
        && IO::WriteBytes(out, buf.data(), buf.size());
}

bool AG::Component::LightComponent::Deserialize(std::istream& in)
{
    while (in.good() && in.peek() != EOF)
    {
        uint32_t hash = 0, size = 0;
        if (!IO::ReadUInt32(in, hash)) break;
        if (!IO::ReadUInt32(in, size)) break;

        if (hash == LIGHT_DATA_HASH)
        {
            std::string buf(size, '\0');
            if (size && !IO::ReadBytes(in, buf.data(), size)) return false;

            std::istringstream tmp(buf, std::ios::binary);
            if (!AG::DeserializeFields(tmp, this, Self::GetFields()))
                return false;

            SyncToLight();
        }
        else
        {
            // Skip unknown future blocks
            in.seekg(size, std::ios::cur);
        }
    }
    return in.good() || in.eof();
}

// ---------- Sync helpers ----------

void AG::Component::LightComponent::SyncToLight()
{
    if (!light) return;
    light->position = r_position;
    light->direction = r_direction;
    light->range = r_range;
    light->intensity = r_intensity;
    light->fovFloat = r_fov;
    light->innerCutoffCos = r_innerCos;
    light->outerCutoffCos = r_outerCos;
    light->type = static_cast<LightType>(r_type_index);
}

void AG::Component::LightComponent::SyncFromLight()
{
    if (!light) return;
    r_direction = light->direction;
    r_range = light->range;
    r_intensity = light->intensity;
    r_fov = light->fovFloat;
    r_innerCos = light->innerCutoffCos;
    r_outerCos = light->outerCutoffCos;
    r_type_index = static_cast<int>(light->type);
}

REGISTER_REFLECTED_TYPE(AG::Component::LightComponent)
