#ifndef _MIME_MAGIC_NUMERIC_NODE_H_
#define _MIME_MAGIC_NUMERIC_NODE_H_

#include "basic_mime_node.h"
#include "utils.h"

namespace magic {

    template<typename Type>
    class numeric_node final : public basic_mime_node {
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
                greater_than,
                bit_and,
                bit_or,
                bit_xor
            };

            struct data_template {
                Type value {};
                Type mask {~0};
                operands operand {operands::equal};
                orders byte_order {orders::little_endian};
            };

            explicit numeric_node(
                    size_t offset,
                    const data_template& data,
                    std::string message,
                    mime_list children
                    ) : basic_mime_node {offset, message, std::move(children)},
                        value_ {data.value},
                        mask_ {data.mask},
                        operand_ {data.operand},
                        byte_order_ {data.byte_order} {
            }

        private:

            Type value_ {};
            Type mask_ {~0};
            operands operand_ {operands::equal};
            orders byte_order_ {orders::little_endian};

            bool is_enough_data(size_t size) override {
                return sizeof(Type) < size;
            }

            response_t process_current(const char *data, size_t size) override {
                Type tmp = utils::convert_raw<Type>(data);
                if (byte_order_ == orders::big_endian) {
                    tmp = utils::change_order(tmp);
                }

                // TODO(pavel-cpp): It may be necessary to do something different for bit operations
                std::string result = utils::format(message_, tmp);

                switch(operand_) {
                    case operands::any: return result;
                    case operands::equal: return (tmp & mask_) == value_ ? result : std::nullopt;
                    case operands::not_equal: return (tmp & mask_) != value_ ? result : std::nullopt;
                    case operands::less_than: return (tmp & mask_) < value_ ? result : std::nullopt;
                    case operands::greater_than: return (tmp & mask_) > value_ ? result : std::nullopt;
                    case operands::bit_and: return (tmp & mask_) & value_ ? result : std::nullopt;
                    case operands::bit_or: return (tmp & mask_) | value_ ? result : std::nullopt;
                    case operands::bit_xor: return (tmp & mask_) ^ value_ ? result : std::nullopt;
                }
            }

    };

} // magic

#endif //_MIME_MAGIC_NUMERIC_NODE_H_
