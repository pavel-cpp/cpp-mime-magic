#include "mime_node.h"

using namespace magic;

namespace {
    template <typename T>
    T convert_raw(const void *ptr) {
        // static_assert(std::is_trivially_copyable_v<T> == true);
        T val;
        std::memcpy(&val, ptr, sizeof(T));
        return val;
    }

    template <typename T>
    bool is_enough_data(size_t size, T _) {
        return size >= sizeof(T);
    }
}

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

class mime_node_bool_processor {
public:
    mime_node_bool_processor(const char *data, size_t size, mime_node::operands operand, bool& result)
        : data_(data),
          size_(size),
          operand_(operand),
          result_(result)
    {
    }

    void operator()(std::nullptr_t) {
        result_ = true;
    }

    template <typename Value>
    void operator()(Value value) {
        Value tmp = convert_raw<Value>(data_);
        switch (operand_) {
        case mime_node::operands::equal: {
            result_ = tmp == value;
            break;
        }
        case mime_node::operands::not_equal: {
            result_ = tmp != value;
            break;
        }
        case mime_node::operands::less_than: {
            result_ = tmp < value;
            break;
        }
        case mime_node::operands::greater_than: {
            result_ = tmp > value;
            break;
        }
        default: {
            throw std::runtime_error(std::string("Invalid operand ") + std::to_string(static_cast<int>(operand_)));
        }
        }
    }

    void operator()(const std::string& value) {
        std::string tmp(data_, value.size());
        switch (operand_) {
        case mime_node::operands::equal: {
            result_ = value == tmp;
            break;
        }
        case mime_node::operands::not_equal: {
            result_ = value != tmp;
            break;
        }
        default: {
            throw std::runtime_error(std::string("Invalid operand ") + std::to_string(static_cast<int>(operand_)));
        }
        }
    }

private:
    const char *data_;
    size_t size_;
    mime_node::operands operand_;
    bool& result_;
};

bool mime_node::process_data(const char *data, size_t size) const {

    std::visit(
        [](const auto& val) {
            std::cout << val << std::endl;
        },
        static_cast<value>(*this)
    );

    bool result;

    std::visit(mime_node_bool_processor(data, size, operand_, result), static_cast<value>(*this));

    if (result == false) {
        return result;
    }

    std::cout << result << " '" << std::string(data, size) << "'" << std::endl;

    if (children_.empty()) {
        return result;
    }

    bool handler_result = false;
    for (const auto& node : children_) {
        handler_result |= node.process_data(data + processed_, size - processed_);
    }

    result &= handler_result;
    // Make switch statement for operands
    return result;
}
