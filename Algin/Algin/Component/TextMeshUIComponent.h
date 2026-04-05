#pragma once
#include "pch.h"
namespace AG {
	namespace Component {
		class TextMeshUIComponent : public IComponent
		{
		public:
			using Self = TextMeshUIComponent;
			TextMeshUIComponent() :
				IComponent(Data::ComponentTypes::TextMeshUI){}

			TextMeshUIComponent(ID id, ID objId) :
				IComponent(Data::ComponentTypes::TextMeshUI, id, objId)
			{
				auto obj = GetObj().lock();
				if (obj)
				{
					if (obj->hasComponent<Component::RectTransformComponent>())
					{
						COMPONENTMANAGER.CreateComponent(obj.get(), Data::ComponentTypes::RectTransform);
					}
				}
			}

			static Type GetStaticType() { return Data::ComponentTypes::TextMeshUI; }

			std::string GetTypeName() const override { return "AG::Component::TextMeshUIComponent"; }

			void Awake() override;

			void Start() override;

			void Update() override;

			void LateUpdate() override;

			void Free() override;

			void Inspector() override;

			//virtual std::shared_ptr<IComponent> Clone() override;
			void AssignFrom(const std::shared_ptr<IComponent>& fromCmp) override;

			// setters for c# 
			void SetFont(std::string font_str);

			void SetFontSize(float size);

			void SetLineSpacing(float spacing);

			void SetLetterSpacing(float spacing);

			void SetWrapLimit(float limit);

			void SetWrap(bool _wrap);

			void SetCentered(bool _centered);

			void SetColor(float r, float g, float b, float a);

			REFLECT() {
				return {
					REFLECT_FIELD(font_color, Color, "Font Color"),
					REFLECT_FIELD(font_hash, size_t, "Font Hash"),
					REFLECT_FIELD_RANGE(font_size, float, "Font Size", "", 0.05f, 128.0f, 0.05f),
					REFLECT_FIELD_RANGE(line_spacing, float, "Line Spacing", "", 0.0f, 10.0f, 0.01f),
					REFLECT_FIELD_RANGE(letter_spacing, float, "Word Spacing", "", 0.0f, 1.0f, 0.01f),
					REFLECT_FIELD_RANGE(wrap_limit, float, "Wrap Limit", "", 0.0f, 2.f, 0.01f),
					REFLECT_FIELD(wrap, bool, "Wrap Text"),
					REFLECT_FIELD(centered, bool, "Centered"),
					REFLECT_FIELD(right, bool, "RightAlign"),
					REFLECT_FIELD(left, bool, "LeftAlign"),

					REFLECT_FIELD(text_content, std::string, "Text Content")
				};
			}

			REFLECT_SERIALIZABLE(TextMeshUIComponent);
			Color font_color;
			size_t font_hash = 0;
			float font_size = 1.f;
			float line_spacing = 1.f;
			std::string text_content = "";

		private:
			/*Color font_color;
			size_t font_hash = 0;*/
			/*float font_size = 1.f;
			float line_spacing = 1.f;*/
			float letter_spacing = 0.f;
			float wrap_limit = 1.f;
			bool wrap = true;
			bool centered = true;
			bool right = false;
			bool left= false;
		};
	}
}