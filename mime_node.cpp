#include "mime_node.h"

using namespace magic;

mime_node::mime_node(value val, mime_array children, operands operand, std::string message)
    : variant(std::move(val)),
      children_(std::move(children)),
      operand_(operand),
      message_(std::move(message)) {
    if (std::holds_alternative<std::string>(*this)) {
        if (operand_ != operands::equal && operand_ != operands::not_equal) {
            throw std::invalid_argument("Invalid operand for string");
        }
        processed_ = std::get<std::string>(*this).size();
        return;
    }
    std::visit(
        [&](const auto& val) {
            processed_ = sizeof(val);
        },
        static_cast<value>(*this)
    );
}

bool mime_node::process_data(const char *data, size_t size) const {
    // if (std::holds_alternative<std::nullptr_t>(*this)) {
    //     return true;
    // }

    bool result = process_current(data, size);

    if (result == false) {
        return result;
    }

    std::cout << result << " '" << std::string(data, size) << "'" << std::endl;

    if (children_.empty()) {
        return result;
    }

    bool handler_result = false;
    for (const auto& node : children_) {
        handler_result |= node.process_data(data + processed_, size - processed_));
    }

    result &= handler_result;
    // Make switch statement for operands
    return result;
}

bool mime_node::process_current(const char *data, size_t size) const {
    bool result;
    switch (operand_) {
    case operands::equal: {
        if (std::holds_alternative<std::string>(*this)) {
            std::string tmp = std::get<std::string>(*this);
            result = tmp == std::string(data, tmp.size());
            break;
        }
        std::visit(
            [&](const auto& val) {
                result = convert_raw_cast(data, val) == val;
            },
            static_cast<value>(*this)
        );
        break;
    }
    case operands::not_equal: {
        if (std::holds_alternative<std::string>(*this)) {
            std::string tmp = std::get<std::string>(*this);
            result = tmp == std::string(data, tmp.size());
            break;
        }
        std::visit(
            [&](const auto& val) {
                result = convert_raw_cast(data, val) == val;
            },
            static_cast<value>(*this)
        );
        break;
    }
    case operands::less_than: {
        std::visit(
            [&](const auto& val) {
                result = convert_raw_cast(data, val) == val;
            },
            static_cast<value>(*this)
        );
        break;
    }
    case operands::greater_than: {
        std::visit(
            [&](const auto& val) {
                result = convert_raw_cast(data, val) == val;
            },
            static_cast<value>(*this)
        );
        break;
    }
    default: {
        throw std::runtime_error(std::string("Invalid operand ") + std::to_string(static_cast<int>(operand_)));
    }
    }
    return result;
}
