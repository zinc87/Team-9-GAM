#pragma once
#include "pch.h"

namespace AG
{
	namespace Component {

		class Image2DComponent : public IComponent {
		public:
			using Self = Image2DComponent;
			static Type GetStaticType() { return Data::ComponentTypes::Image2DComponent; }
			Image2DComponent() :
				IComponent(Data::ComponentTypes::Image2DComponent)
			{
			}

			Image2DComponent(ID id, ID objId) :
				IComponent(Data::ComponentTypes::Image2DComponent, id, objId)
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

			// Inherited via IComponent
			void Awake() override;
			void Start() override;
			void Update() override;
			void LateUpdate() override;
			void Free() override;
			void Inspector() override;

			//virtual std::shared_ptr<IComponent> Clone() override;
			void AssignFrom(const std::shared_ptr<IComponent>& fromCmp) override;


			std::string GetTypeName() const override { return "AG::Component::Image2DComponent"; }

			REFLECT() {
				return {
					REFLECT_FIELD(m_image_hashed, size_t, "Image Hashed"),
					REFLECT_FIELD(m_tc, TextureCoordinate, "TexCoord"),
					REFLECT_FIELD(m_color, Color, "Color")
				};
			}

			REFLECT_SERIALIZABLE(Image2DComponent)

			size_t m_image_hashed{};
			TextureCoordinate m_tc;
			Color m_color;

		private:
			

		};
	}
}