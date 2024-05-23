#include "numeric_node.h"

using namespace magic;

numeric_node::numeric_node(size_t offset, const data_template& data, std::string message, mime_list children)
    : basic_mime_node {offset, std::move(message), std::move(children)},
      value_ {data.value},
      mask_ {data.mask},
      operand_ {data.operand},
      normalize_byte_order_ {data.normalize_byte_order}
{
}

bool numeric_node::is_enough_data(size_t size) {
    int size_of_type;
    std::visit(
        [&](auto t) {
            size_of_type = sizeof(t);
        },
        value_
    );
    return size_of_type < size;
}

basic_mime_node::response_t numeric_node::process_current(const char *data, size_t size) {
    type tmp = value_;
    std::visit(
        [&](auto& value) {
            extract_value(value, data);
        },
        tmp
    );


    std::string result;
    // TODO(pavel-cpp): It may be necessary to do something different for bit operations
    std::visit(
        [&](auto value) {
            result = utils::format(message_, value);
            if (std::holds_alternative<uint8_t>(value_) && std::get<uint8_t>(value_) == 6) {
                throw std::exception();
            }
            std::string tmp = utils::format(message_, value);
        },
        tmp
    );

    response_t response;
    std::visit(
        [&](auto val) {
            switch (operand_) {
            case operands::any:
                response = std::make_optional(result);
                break;
            case operands::equal:
                response = (val & std::get<typeof(val)>(mask_)) == std::get<typeof(val)>(value_)
                               ? std::make_optional(result)
                               : std::nullopt;
                break;
            case operands::not_equal:
                response = (val & std::get<typeof(val)>(mask_)) != std::get<typeof(val)>(value_)
                               ? std::make_optional(result)
                               : std::nullopt;
                break;
            case operands::less_than:
                response = (val & std::get<typeof(val)>(mask_)) < std::get<typeof(val)>(value_)
                               ? std::make_optional(result)
                               : std::nullopt;
                break;
            case operands::greater_than:
                response = (val & std::get<typeof(val)>(mask_)) > std::get<typeof(val)>(value_)
                               ? std::make_optional(result)
                               : std::nullopt;
                break;
            case operands::bit_and:
                response = (val & std::get<typeof(val)>(mask_)) & std::get<typeof(val)>(value_)
                               ? std::make_optional(result)
                               : std::nullopt;
                break;
            case operands::bit_or:
                response = (val & std::get<typeof(val)>(mask_)) | std::get<typeof(val)>(value_)
                               ? std::make_optional(result)
                               : std::nullopt;
                break;
            case operands::bit_xor:
                response = (val & std::get<typeof(val)>(mask_)) ^ std::get<typeof(val)>(value_)
                               ? std::make_optional(result)
                               : std::nullopt;
                break;
            }
        },
        tmp
    );

    return response;
}
