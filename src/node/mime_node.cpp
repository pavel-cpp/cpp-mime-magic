#include "mime_node.h"

#include <cstring>
#include <utility>

using namespace magic;

namespace {
    template <typename T>
    T convert_raw(const void *ptr) {
        static_assert(std::is_trivially_copyable_v<T> == true);
        T val;
        std::memcpy(&val, ptr, sizeof(T));
        return val;
    }

    template <typename T>
    void is_enough_data(size_t size, T data, bool& res) {
        res = size >= data.size();
    }

    void is_enough_data(size_t size, std::nullptr_t, bool& res) {
        res = true;
    }

    bool is_enough_data(size_t size, const mime_node::value& node) {
        bool result;
        std::visit([&](const auto& val) {
            is_enough_data(size, val, result);
        }, node);
        return result;
    }
}

mime_node::mime_node(
    size_t offset,
    value val,
    const mime_list& children,
    operands operand,
    mime_string message
)
    : variant(std::move(val)),
      offset_(offset),
      operand_(operand),
      message_(std::move(message)),
      children_(children) {
//    if (std::holds_alternative<std::string>(*this) && (operand_ != operands::equal && operand_ != operands::not_equal)) {
//        throw std::invalid_argument("Invalid operand for string");
//    }
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
    void operator()(mime_data<Value> value) {
        Value tmp = convert_raw<Value>(data_);
        switch (operand_) {
        case mime_node::operands::equal: {
            result_ = value == tmp;
            break;
        }
        case mime_node::operands::not_equal: {
            result_ = value != tmp;
            break;
        }
        case mime_node::operands::less_than: {
            result_ = value > tmp;
            break;
        }
        case mime_node::operands::greater_than: {
            result_ = value < tmp;
            break;
        }
        case mime_node::operands::bit_and: {
            result_ = value & tmp;
            break;
        }
        case mime_node::operands::bit_or: {
            result_ = value | tmp;
            break;
        }
        case mime_node::operands::bit_xor: {
            result_ = value ^ tmp;
            break;
        }
        default: {
            throw std::runtime_error(std::string("Invalid operand ") + std::to_string(static_cast<int>(operand_)));
        }
        }
    }

    void operator()(const mime_string& value) {
        mime_string tmp(data_, value.size());
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
    if (!is_enough_data(size - offset_, *this)) {
        return false;
    }

    bool result;

    std::visit(mime_node_bool_processor(data + offset_, size - offset_, operand_, result), static_cast<value>(*this));

    if (result == false) {
        return result;
    }

#ifdef ProcDebug
    std::visit(
            [](const auto& val) {
                std::cout << "value = { " << std::hex << val << " }, ";
            },
            static_cast<value>(*this)
    );

    std::cout << "branch = { " << message_ << " }, result = { " << result << " }" << std::endl;
//    std::cout << ", data = '" << std::string(data + offset_, size - offset_) << "'" << std::endl;

#endif

    if (children_.empty()) {
        return result;
    }

    bool handler_result = false;
    for (const auto& node : children_) {
        handler_result |= node.process_data(data, size);
    }

    result &= handler_result;
    // Make switch statement for operands
    return result;
}

