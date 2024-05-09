#ifndef MIME_NODE_H
#define MIME_NODE_H

#include <iostream>

#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace magic {
    class mime_node;

    using mime_array = std::vector<mime_node>;



    class mime_node
        : private std::variant<
            std::nullptr_t,
            uint8_t,
            int8_t,
            uint16_t,
            int16_t,
            uint32_t,
            int32_t,
            uint64_t,
            int64_t,
            std::string
        > {
    public:
        using variant::variant;
        using value = variant;

        enum class operands {
            equal,
            not_equal,
            less_than,
            greater_than,
            bit_and,
            bit_or,
            bit_xor,
        };

        mime_node(
            value val,
            mime_array children = {},
            operands operand = operands::equal,
            std::string message = ""
        );

        bool process_data(const char *data, size_t size) const;

    private:

        mime_array children_ {};
        operands operand_ {operands::equal};
        std::string message_ {};
        uint64_t processed_ {0};
    };
} // magic

#endif //MIME_NODE_H
