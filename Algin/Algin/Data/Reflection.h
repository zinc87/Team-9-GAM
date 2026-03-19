#pragma once

#include "pch.h"
#include "../Data/SerializationSystem.h"

namespace AG {
    namespace System {
        class IObject;
    }
    struct FieldInfo {
        std::string name;                                             // label shown in inspector
        std::type_index type;                                         // typeid of the field
        std::function<void* (void*)> getPtr;                           // returns pointer to field for given object
        std::string category;                                         // grouping category (can be empty)
        std::function<std::vector<const char*>()> getComboItems;      // provider for combo strings (if isCombo)

        // UI metadata
        bool hasRange = false;  // if true, use min/max with DragFloat
        float min = 0.0f;
        float max = 0.0f;
        float step = 0.0f;

        bool isCombo = false;   // treat as indexed combo (int index) when true

        bool hiddenInInspector = false;

        // Default constructor
        FieldInfo()
            : name(""),
            type(typeid(void)),
            getPtr(nullptr),
            category(""),
            getComboItems(nullptr),
            hasRange(false),
            min(0.f), max(0.f), step(0.f),
            isCombo(false) {}

        // Full constructor for macros
        FieldInfo(const std::string& n,
            std::type_index t,
            std::function<void* (void*)> g,
            const std::string& cat,
            std::function<std::vector<const char*>()> combo,
            bool hr, float mn, float mx, float st, bool ic, bool hidden = false)
            : name(n), type(t), getPtr(g), category(cat), getComboItems(combo),
            hasRange(hr), min(mn), max(mx), step(st), isCombo(ic), hiddenInInspector(hidden) {}
    };

    // ------------------------
    // Type Registry
    // ------------------------
    struct TypeInfo {
        std::string name;
        std::function<void* ()> createFunc;
        std::function<std::vector<FieldInfo>()> getFieldsFunc;
    };

    class TypeRegistry {
    public:
        static TypeRegistry& Instance() {
            static TypeRegistry instance;
            return instance;
        }

        void RegisterType(const std::string& name,
            std::function<void* ()> createFunc,
            std::function<std::vector<FieldInfo>()> fieldsFunc) {
            types[name] = { name, createFunc, fieldsFunc };
        }

        TypeInfo* GetType(const std::string& name) {
            auto it = types.find(name);
            if (it != types.end()) return &it->second;
            return nullptr;
        }

        std::vector<std::string> GetRegisteredTypeNames() const {
            std::vector<std::string> names;
            for (const auto& kv : types) names.push_back(kv.first);
            return names;
        }

    private:
        std::unordered_map<std::string, TypeInfo> types;
    };


    void RenderFieldsInspector(void* componentThis, const std::vector<FieldInfo>& fields);

    inline bool SerializeFields(std::ostream& out, void* object,
        const std::vector<AG::FieldInfo>& fields) {
        if (!object) return false;
        auto& reg = System::SerializationSystem::GetInstance().getRegistry();   // now resolves
        for (auto const& f : fields) {
            void* ptr = f.getPtr(object);
            if (!ptr) return false;
            if (auto* ts = reg.Get(f.type)) {         // ts is a pointer
                if (!ts->write(out, ptr)) return false;
            }
            else {
                std::cerr << "[SerializeFields] No serializer for type: "
                    << f.name << "\n";
                return false;
            }
        }
        return out.good();
    }

    inline bool DeserializeFields(std::istream& in, void* object,
        const std::vector<AG::FieldInfo>& fields) {
        if (!object) return false;
        auto& reg = System::SerializationSystem::GetInstance().getRegistry();
        for (auto const& f : fields) {
            void* ptr = f.getPtr(object);
            if (!ptr) return false;
            if (auto* ts = reg.Get(f.type)) {
                if (!ts->read(in, ptr)) return false;
            }
            else {
                std::cerr << "[DeserializeFields] No serializer for type: "
                    << f.name << "\n";
                return false;
            }
        }
        return in.good();
    }

} // namespace AG

// ------------------------
// Macros for reflection
// ------------------------
// Steps to use:
// 1) In your class, define a type alias `using Self = YourClassName
// 
// 2) Use REFLECT() to declare the GetFields() static method
// Example:
// REFLECT() {
//     return {
//         REFLECT_FIELD(myInt, int, "Some Int"),
//         REFLECT_FIELD_CATEGORIZED(myVec, glm::vec3, "Position", "Transform"),
//         REFLECT_FIELD_LIST(selectedIndex, int, "Mesh", "", Self::GetMeshItems) // dropdown/list
//     };
// }
// 
// 3) Use REFLECT_FIELD(...) macros inside GetFields() to list fields to reflect (replace with REFLECT_FIELD_CATEGORIZED or REFLECT_FIELD_LIST as needed)
// Notes:
// - REFLECT_FIELD: simple field (int, float, bool, glm::vec2/3, std::string).
// - REFLECT_FIELD_CATEGORIZED: same, but groups under a category (collapsing header in inspector).
// - REFLECT_FIELD_LIST: for dropdowns/lists. Field stores an int index into the provider’s list.
// - Provider function: static std::vector<const char*> Func(); returns the list of items for ImGui::Combo.
//   Example:
//     static std::vector<const char*> GetMeshItems() {
//         static std::vector<std::string> names;
//         static std::vector<const char*> cptrs;
//         names = {"NO MESH", "Cube", "Sphere"};
//         cptrs.clear(); for (auto &s : names) cptrs.push_back(s.c_str());
//         return cptrs;
//     }
// - Category: empty string "" = default group, otherwise used as a collapsing header.
// 
// 4) Use REGISTER_REFLECTED_TYPE(YourClassName) in a cpp file to register the type
// 
// This will then allow you to use AG::RenderFieldsInspector(this, Self::GetFields()) in your ImGui inspector method and have the fields rendered automatically.
#define REFLECT() static std::vector<AG::FieldInfo> GetFields()

#define REFLECT_FIELD(expr, FieldType, displayName) \
    AG::FieldInfo{ displayName, std::type_index(typeid(FieldType)), \
        [](void* obj)->void* { return &(static_cast<Self*>(obj)->expr); }, \
        std::string(), nullptr, false, 0.f, 0.f, 0.f, false }

#define REFLECT_FIELD_HIDDEN(expr, FieldType, displayName) \
    AG::FieldInfo{ displayName, std::type_index(typeid(FieldType)), \
        [](void* obj)->void* { return &(static_cast<Self*>(obj)->expr); }, \
        std::string(), nullptr, false, 0.f, 0.f, 0.f, false, true }


#define REFLECT_FIELD_RANGE(expr, FieldType, displayName, categoryName, minVal, maxVal, stepVal) \
    AG::FieldInfo{ displayName, std::type_index(typeid(FieldType)), \
        [](void* obj)->void* { return &(static_cast<Self*>(obj)->expr); }, \
        std::string(categoryName), nullptr, true, minVal, maxVal, stepVal, false }

#define REFLECT_FIELD_CATEGORIZED(expr, FieldType, displayName, categoryName) \
    AG::FieldInfo{ displayName, std::type_index(typeid(FieldType)), \
        [](void* obj)->void* { return &(static_cast<Self*>(obj)->expr); }, \
        std::string(categoryName), nullptr, true, 0.0f, 1.0f, 0.05f, false }

#define REFLECT_FIELD_LIST(expr, FieldType, displayName, categoryName, providerFunc) \
    AG::FieldInfo{ displayName, std::type_index(typeid(FieldType)), \
        [](void* obj)->void* { return &(static_cast<Self*>(obj)->expr); }, \
        std::string(categoryName), providerFunc, false, 0.f,0.f,0.f, true }

#define REGISTER_REFLECTED_TYPE(Type) \
    namespace { \
        struct Registration_##__COUNTER__ { \
            Registration_##__COUNTER__() { \
                AG::TypeRegistry::Instance().RegisterType( \
                    #Type, \
                    []() -> void* { return new Type(); }, \
                    []() -> std::vector<AG::FieldInfo> { return Type::GetFields(); }); \
            } \
        }; \
        static Registration_##__COUNTER__ global_registration_##__COUNTER__; \
    }

#define REFLECT_SERIALIZABLE(Type) \
    bool Serialize(std::ostream& out) const override { \
        return AG::SerializeFields(out, const_cast<Type*>(this), Type::GetFields()); \
    } \
    bool Deserialize(std::istream& in) override { \
        return AG::DeserializeFields(in, this, Type::GetFields()); \
    }

#define REFLECT_PTR_FIELD(expr, PointeeType, displayName) \
    AG::FieldInfo{ displayName, std::type_index(typeid(std::shared_ptr<PointeeType>)), \
        [](void* obj)->void* { return &(static_cast<Self*>(obj)->expr); }, \
        std::string(), nullptr, false, 0.f, 0.f, 0.f, false }
