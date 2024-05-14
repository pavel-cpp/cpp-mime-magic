#ifndef MIME_NODE_H
#define MIME_NODE_H

#include <iostream>

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <list>

#include "mime_data.h"

namespace magic {
    class mime_node;

    using mime_string = std::string;
    using mime_list = std::list<mime_node>;

    class mime_node final
            : private std::variant<
                    std::nullptr_t,
                    mime_data<uint8_t>,
                    mime_data<uint16_t>,
                    mime_data<uint32_t>,
                    mime_string
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
                bit_xor
            };

            mime_node() = delete;

            mime_node(
                    size_t offset,
                    value val,
                    const mime_list& children = {},
                    operands operand = operands::equal,
                    std::string message = ""
            );

            mime_node(const mime_node& other) = default;

            mime_node(mime_node&& other) = default;


            bool process_data(const char *data, size_t size) const;

        private:
            size_t offset_ {0};
            operands operand_ {operands::equal};
            std::string message_ {};
            mime_list children_ {};
    };
} // magic

#endif //MIME_NODE_H
