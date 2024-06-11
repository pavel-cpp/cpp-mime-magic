#ifndef _MIME_MAGIC_STRING_NODE_H_
#define _MIME_MAGIC_STRING_NODE_H_

#include "nodes/basic_mime_node.h"
#include "nodes/common.h"

namespace magic {
    class string_node final : public basic_mime_node {
        public:

            enum options {
                none,
                not_case_sensitive
            };

            struct data_template {
                std::string value {};
                options opt {options::none};
                operands operand {operands::equal};
            };

            explicit string_node(size_t offset, const data_template& data, std::string message, mime_list children);

            string_node(string_node&&) noexcept = default;

            ~string_node() override = default;

        private:

            bool is_enough_data(size_t size) override;

            response_t process_current(const char *data, size_t size) override;

            std::string value_ {};
            options opt_ {options::none};
            operands operand_ {operands::equal};

    };

}

#endif //_MIME_MAGIC_STRING_NODE_H_
