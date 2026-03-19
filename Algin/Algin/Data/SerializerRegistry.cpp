#include "pch.h"
#include "SerializerRegistry.h"
#include <iomanip>
#include <sstream>

using namespace AG;

void SerializerRegistry::RegisterDefaultSerializers()
{
    auto& reg = *this;

    // --- int ---
    reg.Register(typeid(int), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            out << *reinterpret_cast<int*>(ptr);
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            int v{}; in >> v;
            *reinterpret_cast<int*>(ptr) = v;
            return true;
        }
        });

    // --- uint32_t ---
    reg.Register(typeid(uint32_t), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            out << *reinterpret_cast<uint32_t*>(ptr);
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            uint32_t v{}; in >> v;
            *reinterpret_cast<uint32_t*>(ptr) = v;
            return true;
        }
        });

    // --- float ---
    reg.Register(typeid(float), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            out << std::fixed << std::setprecision(6)
                << *reinterpret_cast<float*>(ptr);
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            float v{}; in >> v;
            *reinterpret_cast<float*>(ptr) = v;
            return true;
        }
        });

    // --- bool ---
    reg.Register(typeid(bool), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            out << (*reinterpret_cast<bool*>(ptr) ? "true" : "false");
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            std::string val; in >> val;
            *reinterpret_cast<bool*>(ptr) = (val == "true" || val == "1");
            return true;
        }
        });

    // --- std::string ---
    reg.Register(typeid(std::string), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            auto& s = *reinterpret_cast<std::string*>(ptr);
            out << "\"" << s << "\"";
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            std::string value;
            in >> std::ws;
            char quote = static_cast<char>(in.peek());
            if (quote == '"') {
                in.get(); // eat opening quote
                std::getline(in, value, '"');
            }
             else {
              in >> value;
            }
            *reinterpret_cast<std::string*>(ptr) = value;
            return true;
            }
        });

    // --- glm::vec2 ---
    reg.Register(typeid(glm::vec2), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            glm::vec2* v = reinterpret_cast<glm::vec2*>(ptr);
            out << "[" << v->x << ", " << v->y << "]";
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            glm::vec2 v{};
            char ch;
            in >> ch >> v.x >> ch >> v.y >> ch; // [x, y]
            *reinterpret_cast<glm::vec2*>(ptr) = v;
            return true;
        }
        });

    // --- glm::vec3 ---
    reg.Register(typeid(glm::vec3), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            glm::vec3* v = reinterpret_cast<glm::vec3*>(ptr);
            out << "[" << v->x << ", " << v->y << ", " << v->z << "]";
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            glm::vec3 v{};
            char ch;
            in >> ch >> v.x >> ch >> v.y >> ch >> v.z >> ch; // [x, y, z]
            *reinterpret_cast<glm::vec3*>(ptr) = v;
            return true;
        }
        });

    // --- glm::vec4 ---
    reg.Register(typeid(glm::vec4), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            glm::vec4* v = reinterpret_cast<glm::vec4*>(ptr);
            out << "[" << v->x << ", " << v->y << ", " << v->z << ", " << v->w << "]";
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            glm::vec4 v{};
            char ch;
            in >> ch >> v.x >> ch >> v.y >> ch >> v.z >> ch >> v.w >> ch; // [x, y, z, w]
            *reinterpret_cast<glm::vec4*>(ptr) = v;
            return true;
        }
        });

    // --- size_t ---
    reg.Register(typeid(size_t), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            out << static_cast<uint64_t>(*reinterpret_cast<size_t*>(ptr));
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            uint64_t v; in >> v;
            *reinterpret_cast<size_t*>(ptr) = static_cast<size_t>(v);
            return true;
        }
        });

    // --- std::vector<glm::vec3> ---
    reg.Register(typeid(std::vector<glm::vec3>), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            auto& vecs = *reinterpret_cast<std::vector<glm::vec3>*>(ptr);
            out << "[";
            for (size_t i = 0; i < vecs.size(); ++i) {
                out << "[" << vecs[i].x << ", " << vecs[i].y << ", " << vecs[i].z << "]";
                if (i + 1 < vecs.size()) out << ", ";
            }
            out << "]";
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            auto& vecs = *reinterpret_cast<std::vector<glm::vec3>*>(ptr);
            vecs.clear();
            char ch;
            in >> ch; // '['
            while (true) {
                in >> std::ws;
                if (in.peek() == ']') { in.get(); break; } // end of array

                glm::vec3 v{};
                in >> ch >> v.x >> ch >> v.y >> ch >> v.z >> ch; // [x, y, z]
                vecs.push_back(v);

                in >> std::ws;
                if (in.peek() == ',') { in.get(); continue; }
                if (in.peek() == ']') { in.get(); break; }
            }
            return true;
        }
        });

    // --- std::vector<size_t> ---
    reg.Register(typeid(std::vector<size_t>), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            auto& vec = *reinterpret_cast<std::vector<size_t>*>(ptr);
            out << "[";
            for (size_t i = 0; i < vec.size(); ++i) {
                out << static_cast<uint64_t>(vec[i]);
                if (i + 1 < vec.size()) out << ", ";
            }
            out << "]";
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            auto& vec = *reinterpret_cast<std::vector<size_t>*>(ptr);
            vec.clear();
            char c;
            in >> c; // '['
            while (true) {
                in >> std::ws;
                if (in.peek() == ']') { in.get(); break; }
                uint64_t val;
                in >> val;
                vec.push_back(static_cast<size_t>(val));
                in >> std::ws;
                if (in.peek() == ',') { in.get(); continue; }
                if (in.peek() == ']') { in.get(); break; }
            }
            return true;
        }
        });

    // --- Color ---
    reg.Register(typeid(Color), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            auto& c = *reinterpret_cast<Color*>(ptr);
            out << "[" << c.rgba.x << ", " << c.rgba.y << ", "
                << c.rgba.z << ", " << c.rgba.w << "]";
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            char ch;
            Color c{};
            in >> ch >> c.rgba.x >> ch >> c.rgba.y >> ch >> c.rgba.z >> ch >> c.rgba.w >> ch;
            *reinterpret_cast<Color*>(ptr) = c;
            return true;
        }
        });

    // --- TextureCoordinate ---
    reg.Register(typeid(TextureCoordinate), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            auto& t = *reinterpret_cast<TextureCoordinate*>(ptr);
            out << "[";
            for (int i = 0; i < 4; ++i) {
                out << "[" << t.textureCoord[i][0] << ", " << t.textureCoord[i][1] << "]";
                if (i < 3) out << ", ";
            }
            out << "]";
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            TextureCoordinate t{};
            char ch;
            in >> ch; // '['
            for (int i = 0; i < 4; ++i) {
                in >> ch >> t.textureCoord[i][0] >> ch >> t.textureCoord[i][1] >> ch;
                if (i < 3) in >> ch; // comma
            }
            in >> ch; // closing ']'
            *reinterpret_cast<TextureCoordinate*>(ptr) = t;
            return true;
        }
        });

    // --- Transform2D ---
    reg.Register(typeid(Transform2D), TypeSerializer{
        [](std::ostream& out, void* ptr) -> bool {
            auto& trf = *reinterpret_cast<Transform2D*>(ptr);
            out << "{"
                << "\"Position\": [" << trf.position.x << ", " << trf.position.y << ", " << trf.position.z << "], "
                << "\"Scale\": [" << trf.scale.x << ", " << trf.scale.y << "], "
                << "\"Rotation\": " << trf.rotation
                << "}";
            return true;
        },
        [](std::istream& in, void* ptr) -> bool {
            Transform2D t{};
            char ch;
            std::string label;
            in >> ch; // '{'
            in >> std::ws;
            // position
            in.ignore(256, '[');
            in >> t.position.x >> ch >> t.position.y >> ch >> t.position.z;
            in.ignore(256, '[');
            in >> t.scale.x >> ch >> t.scale.y;
            in.ignore(256, ':');
            in >> t.rotation;
            *reinterpret_cast<Transform2D*>(ptr) = t;
            return true;
        }
        });

    reg.Register(typeid(MonoString*), TypeSerializer{
        // ---- WRITE ----
        [](std::ostream& out, void* ptr) -> bool {
            MonoString* monoStr = *reinterpret_cast<MonoString**>(ptr);
            if (!monoStr) {
                out << "\"\"";
                return true;
            }

            char* utf8 = mono_string_to_utf8(monoStr);
            out << "\"" << utf8 << "\"";
            mono_free(utf8);
            return true;
        },

        // ---- READ ----
        [](std::istream& in, void* ptr) -> bool {
            std::string value;
            in >> std::ws;

            char c = static_cast<char>(in.peek());
            if (c == '"') {
                in.get();
                std::getline(in, value, '"');
            }
             else {
              in >> value;
          }

            // FIX: Use the ScriptEngine's app domain instead of mono_domain_get()
            MonoDomain* domain = SCRIPTENGINE.getAppDomain();
            if (!domain) {
                std::cerr << "[MonoString deserialize] No active domain!\n";
                return false;
            }

            MonoString* monoStr = mono_string_new(domain, value.c_str());
            *reinterpret_cast<MonoString**>(ptr) = monoStr;
            return true;
        }
        });


	// --- Shared Pointer Serializers ---
    /*RegisterSharedPtrSerializer<Particle>();*/

    std::cout << "[SerializerRegistry] Registered text-based serializers.\n";
}
//
//// Ensure registration happens at program start
//static struct Registrar { Registrar() { RegisterDefaultSerializers(); } } s_registrar;

namespace AG {
	SerializerRegistry::SerializerRegistry() {
        RegisterDefaultSerializers();

		// --- int ---
		Register(typeid(int), JsonTypeSerializer{
			[](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
				rapidjson::Value value;
				rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const int data = *reinterpret_cast<const int*>(dataPtr);
				value.SetInt(data);
				obj.AddMember(key, value, allocator);
				return true;
			},
			[](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
				if (obj.HasMember(name.c_str())) {
					const auto& objRef = obj[name.c_str()];
					if (objRef.IsInt()) {
						*reinterpret_cast<int*>(dataPtr) = objRef.GetInt();
						return true;
					}
				}
                AG_CORE_WARN("Fail to deserialize int field '{}'", name);
				return false;
			}
		});

		// --- uint32_t ---
		Register(typeid(uint32_t), JsonTypeSerializer{
			[](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
				rapidjson::Value value;
				rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const uint32_t data = *reinterpret_cast<const uint32_t*>(dataPtr);
				value.SetUint(data);
				obj.AddMember(key, value, allocator);
				return true;
			},
			[](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
				if (obj.HasMember(name.c_str())) {
					const auto& objRef = obj[name.c_str()];
					if (objRef.IsUint()) {
						*reinterpret_cast<uint32_t*>(dataPtr) = objRef.GetUint();
						return true;
					}
				}
                AG_CORE_WARN("Fail to deserialize uint field '{}'", name);
				return false;
			}
		});

        // --- uint64_t ---
        Register(typeid(std::size_t), JsonTypeSerializer{
            [](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
                rapidjson::Value value;
                rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const std::size_t data = *reinterpret_cast<const std::size_t*>(dataPtr);
                value.SetUint64(data);
                obj.AddMember(key, value, allocator);
                return true;
            },
            [](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
                if (obj.HasMember(name.c_str())) {
                    const auto& objRef = obj[name.c_str()];
                    if (objRef.IsUint64()) {
                        *reinterpret_cast<std::size_t*>(dataPtr) = objRef.GetUint64();
                        return true;
                    }
                }
                AG_CORE_WARN("Fail to deserialize size_t field '{}'", name);
                return false;
            }
            });

		// --- float ---
		Register(typeid(float), JsonTypeSerializer{
			[](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
				rapidjson::Value value;
				rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const float data = *reinterpret_cast<const float*>(dataPtr);
				value.SetFloat(data);
				obj.AddMember(key, value, allocator);
				return true;
			},
			[](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
				if (obj.HasMember(name.c_str())) {
					const auto& objRef = obj[name.c_str()];
					if (objRef.IsFloat()) {
						*reinterpret_cast<float*>(dataPtr) = objRef.GetFloat();
						return true;
					}
				}
                AG_CORE_WARN("Fail to deserialize float field '{}'", name);
				return false;
			}
		});

		// --- bool ---
		Register(typeid(bool), JsonTypeSerializer{
			[](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
				rapidjson::Value value;
				rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const bool data = *reinterpret_cast<const bool*>(dataPtr);
				value.SetBool(data);
				obj.AddMember(key, value, allocator);
				return true;
			},
			[](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
				if (obj.HasMember(name.c_str())) {
					const auto& objRef = obj[name.c_str()];
					if (objRef.IsBool()) {
						*reinterpret_cast<bool*>(dataPtr) = objRef.GetBool();
						return true;
					}
				}
                AG_CORE_WARN("Fail to deserialize bool field '{}'", name);
				return false;
			}
		});

        // --- std::string ---
        Register(typeid(std::string), JsonTypeSerializer{
            [](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
                rapidjson::Value value;
                rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const std::string& data = *reinterpret_cast<const std::string*>(dataPtr);
                value.SetString(data.c_str(), static_cast<rapidjson::SizeType>(data.size()), allocator);
                obj.AddMember(key, value, allocator);
                return true;
            },
            [](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
                if (obj.HasMember(name.c_str())) {
                    const auto& objRef = obj[name.c_str()];
                    if (objRef.IsString()) {
                        *reinterpret_cast<std::string*>(dataPtr) = objRef.GetString();
                        return true;
                    }
                }
                AG_CORE_WARN("Fail to deserialize std::string field '{}'", name);
                return false;
            }
        });

        // --- glm::vec2 ---
        Register(typeid(glm::vec2), JsonTypeSerializer{
            [this](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
                rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const glm::vec2& data = *reinterpret_cast<const glm::vec2*>(dataPtr);

                rapidjson::Value field{ rapidjson::kObjectType };
                jsonRegistry[typeid(float)].write(allocator, field, &data.x, "x");
                jsonRegistry[typeid(float)].write(allocator, field, &data.y, "y");

                obj.AddMember(key, field, allocator);
                return true;
            },
            [this](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
                if (obj.HasMember(name.c_str())) {
                    const auto& objRef = obj[name.c_str()];
                    if (objRef.IsObject()) {
                        glm::vec2& data = *reinterpret_cast<glm::vec2*>(dataPtr);

                        bool successX = jsonRegistry[typeid(float)].read(objRef, &data.x, "x");
                        bool successY = jsonRegistry[typeid(float)].read(objRef, &data.y, "y");

                        if (successX && successY) {
                            return true;
                        }
                    }
                }
                AG_CORE_WARN("Fail to deserialize glm::vec2 field '{}'", name);
                return false;
            }
        });

        // --- glm::vec3 ---
        Register(typeid(glm::vec3), JsonTypeSerializer{
            [this](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
                rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const glm::vec3& data = *reinterpret_cast<const glm::vec3*>(dataPtr);

                rapidjson::Value field{ rapidjson::kObjectType };
                jsonRegistry[typeid(float)].write(allocator, field, &data.x, "x");
                jsonRegistry[typeid(float)].write(allocator, field, &data.y, "y");
                jsonRegistry[typeid(float)].write(allocator, field, &data.z, "z");

                obj.AddMember(key, field, allocator);
                return true;
            },
            [this](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
                if (obj.HasMember(name.c_str())) {
                    const auto& objRef = obj[name.c_str()];
                    if (objRef.IsObject()) {
                        glm::vec3& data = *reinterpret_cast<glm::vec3*>(dataPtr);

                        bool successX = jsonRegistry[typeid(float)].read(objRef, &data.x, "x");
                        bool successY = jsonRegistry[typeid(float)].read(objRef, &data.y, "y");
                        bool successZ = jsonRegistry[typeid(float)].read(objRef, &data.z, "z");

                        if (successX && successY && successZ) {
                            return true;
                        }
                    }
                }
                AG_CORE_WARN("Fail to deserialize glm::vec3 field '{}'", name);
                return false;
            }
        });

        // --- glm::vec4 ---
        Register(typeid(glm::vec4), JsonTypeSerializer{
            [this](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
                rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const glm::vec4& data = *reinterpret_cast<const glm::vec4*>(dataPtr);

                rapidjson::Value field{ rapidjson::kObjectType };
                jsonRegistry[typeid(float)].write(allocator, field, &data.x, "x");
                jsonRegistry[typeid(float)].write(allocator, field, &data.y, "y");
                jsonRegistry[typeid(float)].write(allocator, field, &data.z, "z");
                jsonRegistry[typeid(float)].write(allocator, field, &data.w, "w");

                obj.AddMember(key, field, allocator);
                return true;
            },
            [this](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
                if (obj.HasMember(name.c_str())) {
                    const auto& objRef = obj[name.c_str()];
                    if (objRef.IsObject()) {
                        glm::vec4& data = *reinterpret_cast<glm::vec4*>(dataPtr);

                        bool successX = jsonRegistry[typeid(float)].read(objRef, &data.x, "x");
                        bool successY = jsonRegistry[typeid(float)].read(objRef, &data.y, "y");
                        bool successZ = jsonRegistry[typeid(float)].read(objRef, &data.z, "z");
                        bool successW = jsonRegistry[typeid(float)].read(objRef, &data.w, "w");

                        if (successX && successY && successZ && successW) {
                            return true;
                        }
                    }
                }
                AG_CORE_WARN("Invalid type for glm::vec4 field '{}'", name);
                return false;
            }
         });

        // --- glm::mat3 ---
        Register(typeid(glm::mat3), JsonTypeSerializer{
            [this](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator,
                   rapidjson::Value& obj,
                   const void* dataPtr,
                   std::string name) -> bool
            {
                rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const glm::mat3& mat = *reinterpret_cast<const glm::mat3*>(dataPtr);

                rapidjson::Value field(rapidjson::kObjectType);

                // Each column is a vec3
                jsonRegistry[typeid(glm::vec3)].write(allocator, field, &mat[0], "col0");
                jsonRegistry[typeid(glm::vec3)].write(allocator, field, &mat[1], "col1");
                jsonRegistry[typeid(glm::vec3)].write(allocator, field, &mat[2], "col2");

                obj.AddMember(key, field, allocator);
                return true;
            },
            [this](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool
            {
                if (obj.HasMember(name.c_str())) {
                    const auto& objRef = obj[name.c_str()];
                    if (objRef.IsObject()) {
                        glm::mat3& mat = *reinterpret_cast<glm::mat3*>(dataPtr);

                        bool success0 = jsonRegistry[typeid(glm::vec3)].read(objRef, &mat[0], "col0");
                        bool success1 = jsonRegistry[typeid(glm::vec3)].read(objRef, &mat[1], "col1");
                        bool success2 = jsonRegistry[typeid(glm::vec3)].read(objRef, &mat[2], "col2");

                        if (success0 && success1 && success2)
                            return true;
                    }
                }
                AG_CORE_WARN("Invalid type for glm::mat3 field '{}'", name);
                return false;
            }
            });

        // --- std::vector<glm::vec3> ---
        Register(typeid(std::vector<glm::vec3>), JsonTypeSerializer{
            [this](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
                rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const std::vector<glm::vec3>& datas = *reinterpret_cast<const std::vector<glm::vec3>*>(dataPtr);

                rapidjson::Value field{ rapidjson::kArrayType };
                for (auto& data : datas) {
                    rapidjson::Value Value{rapidjson::kObjectType};
                    jsonRegistry[typeid(glm::vec3)].write(allocator, Value, &data, "");
                    field.PushBack(std::move(Value), allocator);
                }

                obj.AddMember(key, field, allocator);
                return true;
            },
            [this](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
                if (obj.HasMember(name.c_str())) {
                    const auto& objRef = obj[name.c_str()];
                    if (objRef.IsArray()) {
                        std::vector<glm::vec3>& datas = *reinterpret_cast<std::vector<glm::vec3>*>(dataPtr);
                        datas.clear();
                        datas.reserve(objRef.Size());

                        bool success = true;
                        for (auto& element : objRef.GetArray()) {
                            glm::vec3 value;
                            success = jsonRegistry[typeid(glm::vec3)].read(element, &value, "");
                            if (!success) break;
                            datas.push_back(value);
                        }

                        if (success) {
                            return true;
                        }
                    }
                }
                AG_CORE_WARN("Invalid type for std::vector<glm::vec3> field '{}'", name);
                return false;
            }
            });

        // --- std::vector<size_t> ---
        Register(typeid(std::vector<size_t>), JsonTypeSerializer{
            [this](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
                rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const std::vector<size_t>& datas = *reinterpret_cast<const std::vector<size_t>*>(dataPtr);

                rapidjson::Value field{ rapidjson::kArrayType };
                for (auto& data : datas) {
                    rapidjson::Value Value;
                    Value.SetUint64(data);
                    field.PushBack(std::move(Value), allocator);
                }

                obj.AddMember(key, field, allocator);
                return true;
            },
            [this](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
                if (obj.HasMember(name.c_str())) {
                    const auto& objRef = obj[name.c_str()];
                    if (objRef.IsArray()) {
                        std::vector<size_t>& datas = *reinterpret_cast<std::vector<size_t>*>(dataPtr);
                        datas.clear();
                        datas.reserve(objRef.Size());

                        bool success = true;
                        for (auto& element : objRef.GetArray()) {
                            if (!element.IsUint64()) {
                                success = false;
                                break;
                            }
                            datas.push_back(element.GetUint64());
                        }

                        if (success) {
                            return true;
                        }
                    }
                }
                AG_CORE_WARN("Invalid type for std::vector<size_t> field '{}'", name);
                return false;
            }
            });

        // --- std::vector<std::string> ---
        Register(typeid(std::vector<std::string>), JsonTypeSerializer{
            [this](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
                rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const std::vector<std::string>& datas = *reinterpret_cast<const std::vector<std::string>*>(dataPtr);

                rapidjson::Value field{ rapidjson::kArrayType };
                for (auto& data : datas) {
                    rapidjson::Value Value;
                    Value.SetString(data.c_str(), static_cast<rapidjson::SizeType>(data.size()), allocator);
                    field.PushBack(std::move(Value), allocator);
                }

                obj.AddMember(key, field, allocator);
                return true;
            },
            [this](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
                if (obj.HasMember(name.c_str())) {
                    const auto& objRef = obj[name.c_str()];
                    if (objRef.IsArray()) {
                        std::vector<std::string>& datas = *reinterpret_cast<std::vector<std::string>*>(dataPtr);
                        datas.clear();
                        datas.reserve(objRef.Size());

                        bool success = true;
                        for (auto& element : objRef.GetArray()) {
                            if (!element.IsString()) {
                                success = false;
                                break;
                            }
                            datas.push_back(element.GetString());
                        }

                        if (success) {
                            return true;
                        }
                    }
                }
                AG_CORE_WARN("Invalid type for std::vector<std::string> field '{}'", name);
                return false;
            }
            });

        // --- Color ---
        Register(typeid(Color), JsonTypeSerializer{
            [this](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
                rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const Color& data = *reinterpret_cast<const Color*>(dataPtr);

                rapidjson::Value field{ rapidjson::kObjectType };
                jsonRegistry[typeid(float)].write(allocator, field, &data.rgba.r, "r");
                jsonRegistry[typeid(float)].write(allocator, field, &data.rgba.g, "g");
                jsonRegistry[typeid(float)].write(allocator, field, &data.rgba.b, "b");
                jsonRegistry[typeid(float)].write(allocator, field, &data.rgba.a, "a");

                obj.AddMember(key, field, allocator);
                return true;
            },
            [this](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
                if (obj.HasMember(name.c_str())) {
                    const auto& objRef = obj[name.c_str()];
                    if (objRef.IsObject()) {
                        Color& data = *reinterpret_cast<Color*>(dataPtr);

                        bool successX = jsonRegistry[typeid(float)].read(objRef, &data.rgba.r, "r");
                        bool successY = jsonRegistry[typeid(float)].read(objRef, &data.rgba.g, "g");
                        bool successZ = jsonRegistry[typeid(float)].read(objRef, &data.rgba.b, "b");
                        bool successW = jsonRegistry[typeid(float)].read(objRef, &data.rgba.a, "a");

                        if (successX && successY && successZ && successW) {
                            return true;
                        }
                    }
                }
                AG_CORE_WARN("Invalid type for Color field '{}'", name);
                return false;
            }
        });

        Register(typeid(TextureCoordinate), JsonTypeSerializer{
            [this](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
                const TextureCoordinate& data = *reinterpret_cast<const TextureCoordinate*>(dataPtr);
                rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                rapidjson::Value arrayVal(rapidjson::kArrayType);

                for (int i = 0; i < 4; ++i) {
                    rapidjson::Value coordObj(rapidjson::kObjectType);
                    jsonRegistry[typeid(float)].write(allocator, coordObj, &data.textureCoord[i][0], "u");
                    jsonRegistry[typeid(float)].write(allocator, coordObj, &data.textureCoord[i][1], "v");
                    arrayVal.PushBack(std::move(coordObj), allocator);
                }

                obj.AddMember(key, arrayVal, allocator);
                return true;
            },
            [this](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
                if (!obj.HasMember(name.c_str())) {
                    AG_CORE_WARN("Missing TextureCoordinate field '{}'", name);
                    return false;
                }

                const auto& objRef = obj[name.c_str()];
                if (!objRef.IsArray()) {
                    AG_CORE_WARN("TextureCoordinate '{}' is not an array", name);
                    return false;
                }

                TextureCoordinate& data = *reinterpret_cast<TextureCoordinate*>(dataPtr);
                /*size_t count = std::min<size_t>(4, objRef.Size());*/
                rapidjson::SizeType count = std::min<rapidjson::SizeType>(4, objRef.Size());
                for (rapidjson::SizeType i = 0; i < count; ++i) {
                    const auto& coord = objRef[i]; // This is now a safe 32-bit to 32-bit access
                    if (coord.IsObject()) {
                        jsonRegistry[typeid(float)].read(coord, &data.textureCoord[i][0], "u");
                        jsonRegistry[typeid(float)].read(coord, &data.textureCoord[i][1], "v");
                    }
                }

                return true;
            }
         });

        Register(typeid(Transform2D), JsonTypeSerializer{
            [this](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
                rapidjson::Value key(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), allocator);
                const Transform2D& data = *reinterpret_cast<const Transform2D*>(dataPtr);

                rapidjson::Value field(rapidjson::kObjectType);
                jsonRegistry[typeid(glm::vec3)].write(allocator, field, &data.position, "position");
                jsonRegistry[typeid(glm::vec2)].write(allocator, field, &data.scale, "scale");
                jsonRegistry[typeid(float)].write(allocator, field, &data.rotation, "rotation");

                obj.AddMember(key, field, allocator);
                return true;
            },
            [this](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
                if (!obj.HasMember(name.c_str())) return false;
                const auto& objRef = obj[name.c_str()];
                if (!objRef.IsObject()) return false;

                Transform2D& data = *reinterpret_cast<Transform2D*>(dataPtr);
                bool successPos = jsonRegistry[typeid(glm::vec3)].read(objRef, &data.position, "position");
                bool successS = jsonRegistry[typeid(glm::vec2)].read(objRef, &data.scale, "scale");
                bool successR = jsonRegistry[typeid(float)].read(objRef, &data.rotation, "rotation");

                return successPos && successS && successR;
            }
        });

        Register(typeid(System::IObject), JsonTypeSerializer{
            [this](rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
                const System::IObject& data = *reinterpret_cast<const System::IObject*>(dataPtr);
                System::IObject::ID objID = data.GetID();

                jsonRegistry[typeid(std::string)].write(allocator, obj, &objID, name);

                return true;
            },
            [this](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
                if (obj.HasMember(name.c_str())) {
                    const auto& objRef = obj[name.c_str()];
                    if (objRef.IsString()) {
                        System::IObject::ID objID = objRef.GetString();
                        auto deserializeObj = System::ObjectManager::GetInstance().GetObjectByID(objID);
                        if (!deserializeObj) {
                            return true;
                        }
                        *reinterpret_cast<System::IObject*>(dataPtr) = *deserializeObj.get();
                        return true;
                    }
                }

                AG_CORE_WARN("Fail to deserialize Entity field '{}'", name);
                return false;
            }
        });

        Register(typeid(MonoString*), JsonTypeSerializer{
            // Write
            [](auto& alloc, rapidjson::Value& obj, const void* dataPtr, std::string name) -> bool {
                MonoString* monoStr = *reinterpret_cast<MonoString* const*>(dataPtr);
                const char* utf8 = monoStr ? mono_string_to_utf8(monoStr) : "";
                rapidjson::Value key(name.c_str(), alloc);
                rapidjson::Value val(utf8, alloc);
                obj.AddMember(key, val, alloc);
                if (monoStr) mono_free((void*)utf8);
                return true;
            },
            // Read
            [](const rapidjson::Value& obj, void* dataPtr, std::string name) -> bool {
                if (!obj.HasMember(name.c_str()) || !obj[name.c_str()].IsString())
                    return false;
                const char* value = obj[name.c_str()].GetString();
                MonoString* monoStr = mono_string_new(mono_domain_get(), value);
                *reinterpret_cast<MonoString**>(dataPtr) = monoStr;
                return true;
            }
            });
	}
}
