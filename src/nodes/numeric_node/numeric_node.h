#ifndef _MIME_MAGIC_NUMERIC_NODE_H_
#define _MIME_MAGIC_NUMERIC_NODE_H_

#include <functional>
#include "nodes/basic_mime_node.h"
#include "nodes/utils.h"
#include "nodes/common.h"

#include <variant>


namespace magic {


    class numeric_node final : public basic_mime_node {
        public:

            using type = std::variant<
                    uint8_t,
                    int16_t,
                    uint16_t,
                    int32_t,
                    uint32_t
            >;

            struct data_template {
                type value {};
                type mask {~0};
                operands operand {operands::equal};
                std::function<void(char * , size_t)> normalize_byte_order;
            };

            explicit numeric_node(
                    size_t offset,
                    const data_template& data,
                    std::string message,
                    mime_list children
            );

            numeric_node(numeric_node&&) noexcept = default;

            ~numeric_node() override = default;

        private:

            type value_ {};
            type mask_ {~0};
            operands operand_ {operands::equal};
            std::function<void(char * , size_t)> normalize_byte_order_;

        private:

            bool is_enough_data(size_t size) override;

            template<typename Type>
            void extract_value(Type& dst, const char *data) {
                std::string tmp {data, sizeof(Type)};
                normalize_byte_order_(tmp.data(), tmp.size());
                dst = utils::convert_raw<Type>(tmp.data());
            }

            response_t process_current(const char *data, size_t) override;

    };

} // magic

#endif //_MIME_MAGIC_NUMERIC_NODE_H_
