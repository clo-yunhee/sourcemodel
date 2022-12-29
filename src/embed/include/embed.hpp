#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace embedded {
namespace detail {
#if EMBED_ARRAY
// Long string literals are slower to compile on MSVC, use an array of u32 literals.
template <std::size_t byteCount, std::size_t u32count,
          std::array<std::uint32_t, u32count> arr>
class embedded {
   public:
    inline constexpr std::size_t size() const { return byteCount; }

    template <typename T>
    inline constexpr operator const T *() const {
        return reinterpret_cast<const T *>(arr.data());
    }

    template <typename T>
    inline constexpr operator T *() const {
        return const_cast<T *>((const T *)(*this));
    }
};

template <std::size_t byteCount, std::size_t u32count,
          std::array<std::uint32_t, u32count> arr>
inline constexpr auto makeEmbedded() {
    return embedded<byteCount, u32count, arr>();
}
#else   // !EMBED_ARRAY
template <std::size_t byteCount, const char data[byteCount]>
class embedded {
   public:
    inline constexpr std::size_t size() const { return byteCount; }

    template <typename T>
    inline constexpr operator const T *() const {
        return reinterpret_cast<const T *>(data);
    }

    template <typename T>
    inline constexpr operator T *() const {
        return const_cast<T *>((const T *)(*this));
    }
};

template <std::size_t byteCount, const char data[byteCount]>
inline constexpr auto makeEmbedded() {
    return embedded<byteCount, data>();
}
#endif  // EMBED_ARRAY
}  // namespace detail

inline constexpr std::size_t operator""_uz(unsigned long long num) {
    return std::size_t(num);
}

}  // namespace embedded
