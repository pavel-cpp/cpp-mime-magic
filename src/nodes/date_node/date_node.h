#ifndef _MIME_MAGIC_DATE_NODE_H_
#define _MIME_MAGIC_DATE_NODE_H_

#include "nodes/basic_mime_node.h"
#include "nodes/utils.h"
#include "nodes/common.h"

#include <functional>

namespace magic {

    class date_node final : public basic_mime_node {
        public:

            struct data_template {
                time_t value {};
                std::function<time_t(time_t)> normalize_byte_order;
                operands operand {operands::equal};
            };

            explicit date_node(size_t offset, const data_template& data, std::string message, mime_list children);

            date_node(date_node&&) noexcept = default;

            ~date_node() override = default;

        private:

            time_t value_ {};
            std::function<time_t(time_t)> normalize_byte_order_{};
            operands operand_ {};

            bool is_enough_data(size_t size) override;

            response_t process_current(const char *data, size_t size) override;

    };

} // magic

#endif //_MIME_MAGIC_DATE_NODE_H_
