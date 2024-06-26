#ifndef MIME_DATA_H
#define MIME_DATA_H

#include <functional>
#include <limits>

namespace magic {
    template<typename Type>
    class mime_data;

    template<typename Type>
    Type like(Type data, mime_data<Type> src);

    template<typename Type>
    class mime_data {
        public:

            static Type le(Type value) {
                return value;
            }

            static Type be(Type value) {
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

            mime_data(Type val, std::function<Type(Type)> endian = le)
                    : value_(val),
                      endian_(endian) {
                mask_ = ~0;
            }

            mime_data(Type val, Type mask, std::function<Type(Type)> endian = le)
                    : value_(val),
                      mask_(mask),
                      endian_(endian) {
            }

            bool operator<(Type other) const {
                return (endian_(value_) & mask_) < other;
            }

            bool operator>(Type other) const {
                return (endian_(value_) & mask_) > other;
            }

            bool operator==(Type other) const {
                return (endian_(value_) & mask_) == other;
            }

            bool operator!=(Type other) const {
                auto res = endian_(value_);
                auto rres = res & mask_;
                return (endian_(value_) & mask_) != other;
            }

            bool operator<=(Type other) const {
                return (endian_(value_) & mask_) <= other;
            }

            bool operator>=(Type other) const {
                return (endian_(value_) & mask_) <= other;
            }

            bool operator&(Type other) const {
                return (endian_(value_) & mask_) & other;
            }

            bool operator|(Type other) const {
                return (endian_(value_) & mask_) | other;
            }

            bool operator^(Type other) const {
                return (endian_(value_) & mask_) ^ other;
            }

            operator Type() const {
                return endian_(value_) & mask_;
            }

            [[nodiscard]] size_t size() const {
                return sizeof(Type);
            }

        private:
            Type value_ {};
            Type mask_ {};

            std::function<Type(Type)> endian_;

            template<typename T>
            friend T like(T, mime_data<T>);
    };

    template<typename Type>
    Type like(Type data, mime_data<Type> src) {
        return src.endian_(data);
    }

    template<typename Type>
    Type like(Type data, std::function<Type(Type)> endian) {
        return endian(data);
    }
}

#endif //MIME_DATA_H
