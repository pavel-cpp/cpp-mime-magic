#ifndef _MIME_MAGIC_DATE_NODE_H_
#define _MIME_MAGIC_DATE_NODE_H_

#include "basic_mime_node.h"
#include "utils.h"

namespace magic {

    class date_node final : public basic_mime_node {
        public:

            enum class orders {
                    big_endian,
                    little_endian
            };

            enum class operands {
                    any,
                    equal,
                    not_equal,
                    less_than,
                    greater_than
            };

            struct data_template {
                time_t value {};
                orders byte_order{};
                operands operand {operands::equal};
            };

            explicit date_node(size_t offset, const data_template& data, std::string message, mime_list children);

        private:

            time_t value_ {};
            orders byte_order_{};
            operands operand_ {};

            bool is_enough_data(size_t size) override;

            response_t process_current(const char *data, size_t size) override;

    };

} // magic

#endif //_MIME_MAGIC_DATE_NODE_H_
