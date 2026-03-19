#include "pch.h"
#include "Light.h"

bool LightManager::addLight(std::shared_ptr<Light> light)
{
	if (light.get())
	{
		lightList.emplace_back(light);
		return true;
	}
	return false;
}

void LightManager::removeLight(std::shared_ptr<Light> /*light*/)
{
	//lightList.erase(
	//	std::remove_if(lightList.begin(), lightList.end(),
	//		[&](const std::weak_ptr<Light>& w) {
	//			return w.lock() == light;
	//		}),
	//	lightList.end()
	//);
}

std::vector<Light> LightManager::getLightList()
{
	std::vector<Light> list;
	list.reserve(lightList.size());

	for (auto it = lightList.begin(); it != lightList.end(); )
	{
		if (auto light = it->lock()) // still alive 
		{
			if (!light->disabled)    // only add if not disabled
			{
				list.push_back(*light);
			}
			++it;
		}
		else // expired 
		{
			it = lightList.erase(it);
		}
	}

	return list;
}

bool Light::operator==(const Light& other) const
{
	return this->type == other.type &&
		this->position == other.position &&
		this->direction == other.direction &&
		this->range == other.range &&
		this->intensity == other.intensity;
}
