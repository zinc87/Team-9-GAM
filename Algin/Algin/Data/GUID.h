
#include "pch.h"

namespace AG::Data {

    using GUID = std::string; // Store as human-readable UUID string (with dashes)

    //=================================================================
    // Generate a new UUID v4 (RFC 4122)
    //=================================================================
    inline GUID GenerateGUID(bool uppercase = false) {
        static thread_local std::mt19937_64 rng{ std::random_device{}() };
        std::uniform_int_distribution<uint64_t> dist;

        uint64_t a = dist(rng);
        uint64_t b = dist(rng);

        // Apply version (4) and variant (10xx)
        b = (b & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
        a = (a & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;

        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        if (uppercase)
            oss.setf(std::ios::uppercase);

        // UUID format: 8-4-4-4-12 (36 chars)
        oss << std::setw(8) << ((a >> 32) & 0xFFFFFFFF) << '-'
            << std::setw(4) << ((a >> 16) & 0xFFFF) << '-'
            << std::setw(4) << (a & 0xFFFF) << '-'
            << std::setw(4) << ((b >> 48) & 0xFFFF) << '-'
            << std::setw(12) << (b & 0xFFFFFFFFFFFFULL);

        return oss.str();
    }

    //=================================================================
    // Return Zero GUID (all zeros)
    //=================================================================
    inline GUID GUIDZero() {
        return "00000000-0000-0000-0000-000000000000";
    }

    //=================================================================
    // Check if GUID is zero
    //=================================================================
    inline bool IsZero(const GUID& g) {
        return g == "00000000-0000-0000-0000-000000000000";
    }

    //=================================================================
    // Shortened version (useful for ImGui or debug)
    //=================================================================
    inline std::string GUIDShort(const GUID& g) {
        return g.size() > 8 ? g.substr(0, 8) + "..." : g;
    }

    //=================================================================
    // Equality comparator for unordered_map
    //=================================================================
    struct GUIDEqual {
        bool operator()(const GUID& lhs, const GUID& rhs) const noexcept {
            return lhs == rhs;
        }
    };

    //=================================================================
    // Hasher for unordered_map
    //=================================================================
    struct GUIDHasher {
        std::size_t operator()(const GUID& g) const noexcept {
            return std::hash<std::string>{}(g);
        }
    };

} // namespace AG::Data

//=================================================================
// spdlog / fmtlib formatter for logging GUID strings
//=================================================================
#include "../spdlog/include/spdlog/fmt/bundled/format.h"
namespace fmt {
    template <>
    struct formatter<AG::Data::GUID> : formatter<std::string_view> {
        template <typename FormatContext>
        auto format(const AG::Data::GUID& guid, FormatContext& ctx) {
            return formatter<std::string_view>::format(guid, ctx);
        }
    };
}