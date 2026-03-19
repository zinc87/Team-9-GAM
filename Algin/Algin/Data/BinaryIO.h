#pragma once
#include <ostream>
#include <istream>
#include <cstdint>
#include <string>
#include <vector>

namespace AG {
    namespace IO {

        inline bool WriteUInt32(std::ostream& out, uint32_t v) {
            out.write(reinterpret_cast<const char*>(&v), sizeof(v));
            return out.good();
        }
        inline bool ReadUInt32(std::istream& in, uint32_t& outv) {
            in.read(reinterpret_cast<char*>(&outv), sizeof(outv));
            return in.good();
        }

        inline bool WriteGUID(std::ostream& out, const AG::Data::GUID& guid) {
            out.write(reinterpret_cast<const char*>(guid.data()), 16);
            return out.good();
        }
        inline bool ReadGUID(std::istream& in, AG::Data::GUID& guid) {
            in.read(reinterpret_cast<char*>(guid.data()), 16);
            return in.good();
        }

        // Deterministic sort by GUID string (lexicographic)
        inline bool GUIDLess(const AG::Data::GUID& a, const AG::Data::GUID& b) {
            return a < b;
        }

        inline bool WriteString(std::ostream& out, const std::string& s) {
            uint32_t len = static_cast<uint32_t>(s.size());
            if (!WriteUInt32(out, len)) return false;
            if (len > 0) out.write(s.data(), len);
            return out.good();
        }
        inline bool ReadString(std::istream& in, std::string& s) {
            uint32_t len = 0;
            if (!ReadUInt32(in, len)) return false;
            s.resize(len);
            if (len > 0) in.read(&s[0], len);
            return in.good();
        }

        inline bool WriteBytes(std::ostream& out, const void* data, size_t size) {
            out.write(reinterpret_cast<const char*>(data), size);
            return out.good();
        }
        inline bool ReadBytes(std::istream& in, void* data, size_t size) {
            in.read(reinterpret_cast<char*>(data), size);
            return in.good();
        }

        inline bool WriteFloat(std::ostream& out, float value) {
            out.write(reinterpret_cast<const char*>(&value), sizeof(value));
            return out.good();
        }

        inline bool ReadFloat(std::istream& in, float& value) {
            in.read(reinterpret_cast<char*>(&value), sizeof(value));
            return in.good();
        }

    }
}
