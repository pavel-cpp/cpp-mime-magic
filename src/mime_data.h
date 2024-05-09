#ifndef MIME_DATA_H
#define MIME_DATA_H

namespace magic {
    template <typename Type>
    class mime_data final {
    public:

        mime_data(Type val)
            : value(val) {
            mask = ~mask;
        }

        mime_data(Type val, Type mask)
            : value(val),
              mask(mask) {
        }

        bool operator<(Type other) const {
            return (value & mask) < other;
        }

        bool operator>(Type other) const {
            return (value & mask) > other;
        }

        bool operator==(Type other) const {
            return (value & mask) == other;
        }

        bool operator!=(Type other) const {
            return (value & mask) != other;
        }

        bool operator<=(Type other) const {
            return (value & mask) <= other;
        }

        bool operator>=(Type other) const {
            return (value & mask) <= other;
        }

        operator Type() const {
            return value & mask;
        }

        [[nodiscard]] size_t size() const {
            return sizeof(Type);
        }

    private:
        Type value {};
        Type mask {};
    };
}

#endif //MIME_DATA_H
