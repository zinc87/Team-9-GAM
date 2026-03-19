#pragma once
#include <typeindex>
#include <functional>
#include <unordered_map>
#include <string>
#include <iostream>

namespace AG {

    struct JsonTypeSerializer {
        std::function<bool(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>&, rapidjson::Value&, const void*, std::string name)> write;
        std::function<bool(const rapidjson::Value&, void*, std::string name)> read;
    };

    struct TypeSerializer {
        std::function<bool(std::ostream&, void*)> write;
        std::function<bool(std::istream&, void*)> read;
    };

    class SerializerRegistry {
    public:
        void RegisterDefaultSerializers();
        SerializerRegistry();

        TypeSerializer* Get(const std::type_index& idx){
            auto it = registry.find(idx);
            return it == registry.end() ? nullptr : &it->second;
        }

        JsonTypeSerializer* GetJsonReg(const std::type_index& idx) {
            auto it = jsonRegistry.find(idx);
            return it == jsonRegistry.end() ? nullptr : &it->second;
        }
    private:
        void Register(const std::type_index& idx, TypeSerializer ts) {
            registry[idx] = std::move(ts);
        }
        void Register(const std::type_index& idx, JsonTypeSerializer ts) {
            jsonRegistry[idx] = std::move(ts);
        }
    private:
        std::unordered_map<std::type_index, TypeSerializer> registry;
        std::unordered_map<std::type_index, JsonTypeSerializer> jsonRegistry;
    };
}
