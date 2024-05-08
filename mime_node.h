#ifndef MIME_NODE_H
#define MIME_NODE_H

#include <iostream>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace magic {
    class mime_node;

    using mime_array = std::vector<mime_node>;

    namespace {
        template <typename T>
        T convert_raw(const void *ptr) {
            // static_assert(std::is_trivially_copyable_v<T> == true);
            T val;
            std::memcpy(&val, ptr, sizeof(T));
            return val;
        }

        template <typename D>
        D convert_raw_cast(const void *ptr, D _) {
            std::cout << "Debug: " << std::is_same<D, int>() << std::endl;
            return convert_raw<D>(ptr);
        }
    }

    class mime_node
        : private std::variant<
            std::nullptr_t,
            uint8_t,
            int8_t,
            uint16_t,
            int16_t,
            uint32_t,
            int32_t,
            uint64_t,
            int64_t,
            std::string
        > {
    public:
        using variant::variant;
        using value = variant;

        enum class operands {
            equal,
            not_equal,
            less_than,
            greater_than,
            bit_and,
            bit_or,
            bit_xor,
        };

        mime_node(
            value val,
            mime_array children = {},
            operands operand = operands::equal,
            std::string message = ""
        )
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

        bool process_data(const char *data, size_t size) const {
            bool result;
            if (std::holds_alternative<std::nullptr_t>(*this)) {
                return false;
            }

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
                if (std::holds_alternative<std::string>(*this)) {
                    throw 1;
                }

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

            std::vector<bool> pre_result;
            pre_result.reserve(children_.size());

            for (const auto& node : children_) {
                pre_result.emplace_back(result && node.process_data(data + processed_, size));
            }

            for (bool res : pre_result) {
                result = result || res;
            }
            // Make switch statement for operands
            return result;
        }

    private:
        mime_array children_ {};
        operands operand_ {operands::equal};
        std::string message_ {};
        uint64_t processed_ {0};
    };
} // magic

#endif //MIME_NODE_H
