/*!*****************************************************************************
\file IComponent.h
\author Zulfami Ashrafi Bin Wakif
\date 17/2/2025 (MM/DD/YYYY)

\brief Interface class for component
*******************************************************************************/
#pragma once
#include "../Header/pch.h"
#include "../Object Base/ObjectManager.h"

//Macro for the assignFrom function
//Use fromCompPtr to get the derived from component pointer
#define ASSIGNFROM_FN_BODY(AssignmentBody) auto fromCmpPtr = std::dynamic_pointer_cast<Self>(fromCmp);\
                        if (fromCmpPtr) { \
                            AssignmentBody \
                        }\
                        else {\
                            AG_CORE_WARN("AssignFrom: incompatible component type");\
                        }\

#define BASIC_ASSIGN(param) this->param = fromCmpPtr->param

namespace AG {
	namespace System {
		class IObject;
	}

	namespace Component {
		class IComponent {
		public:
			using Type = Data::ComponentTypes;
			using ID = Data::GUID;

		public:
			// === Constructor === //
			IComponent() 
                : m_type{ Type::None },
                  m_id{ Data::GUIDZero() },
                  m_objId{ Data::GUIDZero() },
                  m_hasAwake{ false },
                  m_hasStart{ false },
                  m_isEnable{ true } { }

			IComponent(Type type) 
                : m_type{ type }, 
                  m_id{ Data::GUIDZero() },
                  m_objId{ Data::GUIDZero() },
                  m_hasAwake{ false }, 
                  m_hasStart{ false }, 
                  m_isEnable{ true } { }

			IComponent(Type type, ID id) 
                : m_type{ type }, 
                  m_id{ id }, 
                  m_objId{ Data::GUIDZero() },
                  m_hasAwake{ false }, 
                  m_hasStart{ false }, 
                  m_isEnable{ true } { }

            // Mainly in use
			IComponent(Type type, ID id, ID objId) 
                : m_type{ type }, 
                  m_id{ id }, 
                  m_objId{ objId }, 
                  m_hasAwake{ false }, 
                  m_hasStart{ false }, 
                  m_isEnable{ true } { }

			// === Component Main Function ===//
			virtual void Awake()		= 0;
			virtual void Start()		= 0;
			virtual void Update()		= 0;
			virtual void LateUpdate()	= 0;
			virtual void Free()			= 0;

			virtual void Inspector()	= 0;

            //virtual std::shared_ptr<IComponent> Clone() = 0;
            
            // Funtion that DEEP COPY data from other to this
            virtual void AssignFrom(const std::shared_ptr<IComponent>& other) = 0;

            virtual std::string GetTypeName() const = 0;

			// Serialization
            virtual bool Serialize(std::ostream& out) const;
                //{
                //    auto ti = AG::TypeRegistry::Instance().GetType(GetTypeName());
                //    if (!ti) {
                //        std::cerr << "[Serialize] No TypeInfo for " << GetTypeName() << "\n";
                //        return false;
                //    }
                //    auto fields = ti->getFieldsFunc();

                //    // For each reflected field write: <fieldHash:uint32><payloadSize:uint32><payload bytes>
                //    for (auto& f : fields) {
                //        void* fieldPtr = f.getPtr((void*)this);
                //        if (!fieldPtr) continue;

                //        auto ts = System::SerializationSystem::GetInstance().getRegistry().Get(f.type);
                //        if (!ts) {
                //            std::cerr << "[Serialize] Unsupported field type for field '" << f.name << "'\n";
                //            return false;
                //        }

                //        // write field hash
                //        // compute hash (use FNV1a)
                //        uint32_t h = 2166136261u;
                //        for (char c : f.name) { h ^= static_cast<uint8_t>(c); h *= 16777619u; }
                //        if (!AG::IO::WriteUInt32(out, h)) return false;

                //        // temporarily write payload to memory to know its size (allows complex types)
                //        std::ostringstream tmp(std::ios::binary);
                //        if (!ts->write(tmp, fieldPtr)) return false;

                //        std::string payload = tmp.str();
                //        uint32_t payloadSize = static_cast<uint32_t>(payload.size());
                //        if (!AG::IO::WriteUInt32(out, payloadSize)) return false;
                //        if (payloadSize > 0) out.write(payload.data(), payloadSize);
                //        if (!out.good()) return false;
                //    }
                //    return true;
                //}

                virtual bool Deserialize(std::istream& in);
            //{
            //    auto ti = AG::TypeRegistry::Instance().GetType(GetTypeName());
            //    if (!ti) {
            //        std::cerr << "[Deserialize] No TypeInfo for " << GetTypeName() << "\n";
            //        return false;
            //    }
            //    auto fields = ti->getFieldsFunc();

            //    // Build a map hash->FieldInfo for quick lookup
            //    std::unordered_map<uint32_t, FieldInfo> fmap;
            //    for (auto& f : fields) {
            //        uint32_t h = 2166136261u;
            //        for (char c : f.name) { h ^= static_cast<uint8_t>(c); h *= 16777619u; }
            //        fmap[h] = f;
            //    }

            //    // We'll repeatedly read <fieldHash><payloadSize><payload>
            //    // STOP CRITERIA: We rely on caller to know how many fields exist for the component.
            //    // In our scene format we will write exactly the fields in same order; here we just loop until we've consumed all expected fields.
            //    size_t fieldsRead = 0;
            //    while (fieldsRead < fields.size() && in.good()) {
            //        uint32_t fieldHash = 0;
            //        if (!AG::IO::ReadUInt32(in, fieldHash)) return false;
            //        uint32_t payloadSize = 0;
            //        if (!AG::IO::ReadUInt32(in, payloadSize)) return false;

            //        auto it = fmap.find(fieldHash);
            //        if (it == fmap.end()) {
            //            // unknown field: skip payload
            //            in.seekg(payloadSize, std::ios::cur);
            //            if (!in.good()) return false;
            //        }
            //        else {
            //            FieldInfo f = it->second;
            //            void* fieldPtr = f.getPtr((void*)this);
            //            if (!fieldPtr) {
            //                // skip if pointer invalid
            //                in.seekg(payloadSize, std::ios::cur);
            //                if (!in.good()) return false;
            //            }
            //            else {
            //                auto ts = System::SerializationSystem::GetInstance().getRegistry().Get(f.type);
            //                if (!ts) {
            //                    // unsupported type: skip
            //                    in.seekg(payloadSize, std::ios::cur);
            //                    if (!in.good()) return false;
            //                }
            //                else {
            //                    // read payload into buffer then feed to ts->read via stringstream
            //                    std::string buf(payloadSize, '\0');
            //                    if (payloadSize > 0) in.read(&buf[0], payloadSize);
            //                    if (!in.good()) return false;
            //                    std::istringstream iss(buf, std::ios::binary);
            //                    if (!ts->read(iss, fieldPtr)) return false;
            //                }
            //            }
            //        }
            //        ++fieldsRead;
            //    }
            //    return true;
            //}

			Type GetType()		{ return m_type; }
			ID   GetID()		{ return m_id; }
			ID	 GetObjID()		{ return m_objId; }
			std::weak_ptr<System::IObject> GetObj() { return OBJECTMANAGER.GetObjectById(m_objId); }
			
            std::unordered_map<uint64_t, std::string> id64_to_guid;

			bool IsInit()		 { return m_hasAwake; }
			void IsInit(bool r)  { m_hasAwake = r; }
			bool IsStart()		 { return m_hasStart; }
			void IsStart(bool r) { m_hasStart = r; }
			bool& IsEnable() { 
				m_hasStart = (m_isEnable == false) ? false : m_hasStart;
				return m_isEnable; 
			}

            bool HasValidID() const { return !Data::IsZero(m_id); }

		protected:
			Type	m_type;
			ID		m_id;
			ID		m_objId;
			bool	m_hasAwake;
			bool	m_hasStart;
			bool	m_isEnable;
		};
	}
}
