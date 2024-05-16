#ifndef _MIME_MAGIC_UTILS_H_
#define _MIME_MAGIC_UTILS_H_

#include <string>
#include <cstdio>
#include <limits>
#include <cstdint>
#include <cstring>

namespace magic::utils {

    inline std::string format(const std::string& format, const std::string& str) {
        std::string out;
        out.resize(format.size(), str.size());
        sprintf(out.data(), format.c_str(), str.c_str());
        return out;
    }

    template<typename Type>
    inline std::string format(const std::string& format, Type value) {
        return format(format, std::to_string(value));
    }

    template<typename T>
    T convert_raw(const void *ptr) {
        static_assert(std::is_trivially_copyable_v<T> == true);
        T val;
        std::memcpy(&val, ptr, sizeof(T));
        return val;
    }

    template<typename Type>
    inline Type change_order(Type value) {
        static_assert(std::numeric_limits<Type>::is_integer);
        union {
            Type val;
            uint8_t arr[sizeof(Type)];
        } result;
        result.val = 0;

        auto value_ptr {reinterpret_cast<const uint8_t *>(&value)};
        for (size_t i {0}; i < sizeof(Type); ++i)
            result.arr[sizeof(Type) - 1 - i] = *(value_ptr++);
        return result.val;
    }

}

#endif //_MIME_MAGIC_UTILS_H_
