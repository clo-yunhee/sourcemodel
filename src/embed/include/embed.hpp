#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace embedded {
namespace detail {
template <size_t byteCount, size_t u32count, std::array<uint32_t, u32count> arr>
class embedded {
   public:
    inline constexpr size_t size() const { return byteCount; }

    template <typename T>
    inline constexpr operator const T *() const {
        return reinterpret_cast<const T *>(arr.data());
    }

    template <typename T>
    inline constexpr operator T *() const {
        return const_cast<T *>((const T*) (*this));
    }
};

template <size_t byteCount, size_t u32count, std::array<uint32_t, u32count> arr>
inline constexpr auto makeEmbedded() {
    return embedded<byteCount, u32count, arr>();
}
}  // namespace detail

inline constexpr size_t operator""_uz(unsigned long long num) { return size_t(num); }

}  // namespace embedded
