#ifndef _MIME_MAGIC_UTILS_H_
#define _MIME_MAGIC_UTILS_H_

#include <string>
#include <cstdio>
#include <limits>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <iostream>

namespace magic::utils {

    inline std::string format(const std::string& format_s, const std::string& str) {
        std::string out;
        out.resize(format_s.size(), str.size());
        sprintf(out.data(), format_s.c_str(), str.c_str());
        return out;
    }

    template<typename Type>
    inline std::string format(const std::string& format_s, Type value) {
        std::string out;
        out.resize(format_s.size(), 20);
        sprintf(out.data(), format_s.c_str(), value);
        return out;
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

    inline void change_raw_order(char *data, size_t size) {
        assert(size % 2 == 0);

        for (size_t i = 0; i < size / 2; ++i) {
            char temp = data[i];
            data[i] = data[size - 1 - i];
            data[size - 1 - i] = temp;
        }
    }

}

#endif //_MIME_MAGIC_UTILS_H_
