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

            using types = std::variant<
                    uint8_t,
                    int16_t,
                    uint16_t,
                    int32_t,
                    uint32_t
            >;

            struct data_template {
                types value {};
                types mask {~0};
                operands operand {operands::equal};
                std::function<void(char * , size_t)> normalize_byte_order;
            };

            explicit numeric_node(
                    size_t offset,
                    const data_template& data,
                    std::string message,
                    mime_list children
            ) : basic_mime_node {offset, std::move(message), std::move(children)},
                value_ {data.value},
                mask_ {data.mask},
                operand_ {data.operand},
                normalize_byte_order_ {data.normalize_byte_order} {
            }

            numeric_node(numeric_node&&) noexcept = default;

            ~numeric_node() override = default;

        private:

            types value_ {};
            types mask_ {~0};
            operands operand_ {operands::equal};
            std::function<void(char * , size_t)> normalize_byte_order_;

            bool is_enough_data(size_t size) override {
                int size_of_type;
                std::visit([&](auto t) {
                    size_of_type = sizeof(t);
                }, value_);
                return size_of_type < size;
            }

            template<typename Type>
            void extract_value(Type& dst, const char *data) {
                std::string tmp {data, sizeof(Type)};
                normalize_byte_order_(tmp.data(), tmp.size());
                dst = utils::convert_raw<Type>(tmp.data());
            }

            response_t process_current(const char *data, size_t size) override {
                types tmp = value_;
                std::visit([&](auto& value) {
                    extract_value(value, data);
                }, tmp);


                std::string result;
                // TODO(pavel-cpp): It may be necessary to do something different for bit operations
                std::visit(
                        [&](auto value) {
                            result = utils::format(message_, value);
                        },
                        tmp
                );

                response_t response;
                std::visit(
                        [&](auto val){
                            switch (operand_) {
                                case operands::any:
                                    response = std::make_optional(result);
                                    break;
                                case operands::equal:
                                    response = (val & std::get<typeof(val)>(mask_)) == std::get<typeof(val)>(value_) ? std::make_optional(result) : std::nullopt;
                                    break;
                                case operands::not_equal:
                                    response = (val & std::get<typeof(val)>(mask_)) != std::get<typeof(val)>(value_) ? std::make_optional(result) : std::nullopt;
                                    break;
                                case operands::less_than:
                                    response = (val & std::get<typeof(val)>(mask_)) < std::get<typeof(val)>(value_) ? std::make_optional(result) : std::nullopt;
                                    break;
                                case operands::greater_than:
                                    response = (val & std::get<typeof(val)>(mask_)) > std::get<typeof(val)>(value_) ? std::make_optional(result) : std::nullopt;
                                    break;
                                case operands::bit_and:
                                    response = (val & std::get<typeof(val)>(mask_)) & std::get<typeof(val)>(value_) ? std::make_optional(result) : std::nullopt;
                                    break;
                                case operands::bit_or:
                                    response = (val & std::get<typeof(val)>(mask_)) | std::get<typeof(val)>(value_) ? std::make_optional(result) : std::nullopt;
                                    break;
                                case operands::bit_xor:
                                    response = (val & std::get<typeof(val)>(mask_)) ^ std::get<typeof(val)>(value_) ? std::make_optional(result) : std::nullopt;
                                    break;
                            }
                            }
                        ,tmp
                        );

                return response;
            }

    };

} // magic

#endif //_MIME_MAGIC_NUMERIC_NODE_H_
