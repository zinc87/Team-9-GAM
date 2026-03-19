#include "pch.h"
#include "ButtonComponent.h"

void AG::Component::ButtonComponent::Awake()
{
}

void AG::Component::ButtonComponent::Start()
{
}

void AG::Component::ButtonComponent::Update()
{

	auto rect_trf = GetObj().lock()->GetComponent<Component::RectTransformComponent>().lock();
	if (rect_trf)
	{
		// button position and size
		glm::vec2	btn_pos = rect_trf->GetTransform().position;
		glm::vec2	btn_size = rect_trf->GetTransform().scale;
		float		btn_rot = rect_trf->GetTransform().rotation;

		std::pair<float, float> mouse_pos_pair = Input::GetMousePosition();
		glm::vec2 mouse_pos = glm::vec2(mouse_pos_pair.first, mouse_pos_pair.second);

		glm::vec2 translated_mouse = mouse_pos - btn_pos;

		float cos_theta = glm::cos(-btn_rot);
		float sin_theta = glm::sin(-btn_rot);

		glm::vec2 rotated_mouse;
		rotated_mouse.x = translated_mouse.x * cos_theta - translated_mouse.y * sin_theta;
		rotated_mouse.y = translated_mouse.x * sin_theta + translated_mouse.y * cos_theta;


		float half_width = btn_size.x / 2.0f;
		float half_height = btn_size.y / 2.0f;

		bool is_mouse_in_button =
			(rotated_mouse.x >= -half_width && rotated_mouse.x <= half_width) &&
			(rotated_mouse.y >= -half_height && rotated_mouse.y <= half_height);


		std::array<std::array<bool, 4>*, 3> allButtonStates = { &L_buttonStates, &R_buttonStates, &M_buttonStates };
		std::array<int, 3> buttonCodes = { MOUSE_BUTTON_LEFT, MOUSE_BUTTON_RIGHT, MOUSE_BUTTON_MIDDLE };

		for (int i = 0; i < 3; ++i)
		{
			std::array<bool, 4>& states = *allButtonStates[i];
			int buttonCode = buttonCodes[i];

			states[STATE_PRESSED] = false;
			states[STATE_RELEASED] = false;

			states[STATE_HOVER] = is_mouse_in_button;

			if (is_mouse_in_button)
			{
				if (Input::IsMouseButtonTriggered(buttonCode))
				{
					states[STATE_PRESSED] = true;
					states[STATE_HELD] = true; 
				}
			}

			if (states[STATE_HELD])
			{
				if (Input::IsMouseButtonReleased(buttonCode))
				{
					states[STATE_RELEASED] = true;
					states[STATE_HELD] = false; 
				}
				else
				{
					states[STATE_HELD] = Input::IsMouseButtonPressed(buttonCode) && is_mouse_in_button;
				}
			}

		}
	}

}

void AG::Component::ButtonComponent::LateUpdate()
{
}

void AG::Component::ButtonComponent::Free()
{
}

void AG::Component::ButtonComponent::Inspector()
{
}

void AG::Component::ButtonComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp)
{
	(void)fromCmp;
}

REGISTER_REFLECTED_TYPE(AG::Component::ButtonComponent)