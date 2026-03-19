#include "pch.h"
#include "IComponent.h"

namespace AG {
	namespace Component {
        bool IComponent::Serialize(std::ostream& out) const {
            auto ti = AG::TypeRegistry::Instance().GetType(GetTypeName());
            if (!ti) {
                std::cerr << "[Serialize] No TypeInfo for " << GetTypeName() << "\n";
                return false;
            }
            auto fields = ti->getFieldsFunc();

            // For each reflected field write: <fieldHash:uint32><payloadSize:uint32><payload bytes>
            for (auto& f : fields) {
                void* fieldPtr = f.getPtr((void*)this);
                if (!fieldPtr) continue;

                auto ts = System::SerializationSystem::GetInstance().getRegistry().Get(f.type);
                if (!ts) {
                    std::cerr << "[Serialize] Unsupported field type for field '" << f.name << "'\n";
                    return false;
                }

                // write field hash
                // compute hash (use FNV1a)
                uint32_t h = 2166136261u;
                for (char c : f.name) { h ^= static_cast<uint8_t>(c); h *= 16777619u; }
                if (!AG::IO::WriteUInt32(out, h)) return false;

                // temporarily write payload to memory to know its size (allows complex types)
                std::ostringstream tmp(std::ios::binary);
                if (!ts->write(tmp, fieldPtr)) return false;

                std::string payload = tmp.str();
                uint32_t payloadSize = static_cast<uint32_t>(payload.size());
                if (!AG::IO::WriteUInt32(out, payloadSize)) return false;
                if (payloadSize > 0) out.write(payload.data(), payloadSize);
                if (!out.good()) return false;
            }
            return true;
        }

        bool IComponent::Deserialize(std::istream& in)
        {
            auto ti = AG::TypeRegistry::Instance().GetType(GetTypeName());
            if (!ti) {
                std::cerr << "[Deserialize] No TypeInfo for " << GetTypeName() << "\n";
                return false;
            }
            auto fields = ti->getFieldsFunc();

            // Build a map hash->FieldInfo for quick lookup
            std::unordered_map<uint32_t, FieldInfo> fmap;
            for (auto& f : fields) {
                uint32_t h = 2166136261u;
                for (char c : f.name) { h ^= static_cast<uint8_t>(c); h *= 16777619u; }
                fmap[h] = f;
            }

            // We'll repeatedly read <fieldHash><payloadSize><payload>
            // STOP CRITERIA: We rely on caller to know how many fields exist for the component.
            // In our scene format we will write exactly the fields in same order; here we just loop until we've consumed all expected fields.
            size_t fieldsRead = 0;
            while (fieldsRead < fields.size() && in.good()) {
                uint32_t fieldHash = 0;
                if (!AG::IO::ReadUInt32(in, fieldHash)) return false;
                uint32_t payloadSize = 0;
                if (!AG::IO::ReadUInt32(in, payloadSize)) return false;

                auto it = fmap.find(fieldHash);
                if (it == fmap.end()) {
                    // unknown field: skip payload
                    in.seekg(payloadSize, std::ios::cur);
                    if (!in.good()) return false;
                }
                else {
                    FieldInfo f = it->second;
                    void* fieldPtr = f.getPtr((void*)this);
                    if (!fieldPtr) {
                        // skip if pointer invalid
                        in.seekg(payloadSize, std::ios::cur);
                        if (!in.good()) return false;
                    }
                    else {
                        auto ts = System::SerializationSystem::GetInstance().getRegistry().Get(f.type);
                        if (!ts) {
                            // unsupported type: skip
                            in.seekg(payloadSize, std::ios::cur);
                            if (!in.good()) return false;
                        }
                        else {
                            // read payload into buffer then feed to ts->read via stringstream
                            std::string buf(payloadSize, '\0');
                            if (payloadSize > 0) in.read(&buf[0], payloadSize);
                            if (!in.good()) return false;
                            std::istringstream iss(buf, std::ios::binary);
                            if (!ts->read(iss, fieldPtr)) return false;
                        }
                    }
                }
                ++fieldsRead;
            }
            return true;
        }
	}
}