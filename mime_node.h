#ifndef MIME_NODE_H
#define MIME_NODE_H

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace magic {
    class mime_node;

    using array = std::vector<mime_node>;

    class mime_node
        : private std::variant<
            std::nullptr_t,
            std::pair<uint8_t, array>,
            std::pair<uint16_t, array>,
            std::pair<uint32_t, array>,
            std::pair<uint64_t, array>,
            std::pair<std::string, array>
    >{
    public:
        using variant::variant;
        using value = variant;

        template<typename input_it>
        bool process_data(input_it begin, input_it end);
    };
} // magic

#endif //MIME_NODE_H
