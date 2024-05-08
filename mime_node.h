#ifndef MIME_NODE_H
#define MIME_NODE_H

#include <iostream>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace magic {
    class mime_node;
    class mime_node_base;

    using mime_array = std::vector<std::unique_ptr<mime_node_base>>;

    namespace {
        template <typename T>
        T convert_raw(const void *ptr) {
            static_assert(std::is_trivially_copyable_v<T> == true);
            T val;
            std::memcpy(&val, ptr, sizeof(T));
            return val;
        }

        template<typename D>
        D convert_raw_cast(const void *ptr, D _) {
            std::cout << std::is_same<D, int>() << std::endl;
            return convert_raw<D>(ptr);
        }
    }

    class mime_node_base {
    public:
        enum class operands {
            equal,
            not_equal,
            less_than,
            greater_than,
            bit_and,
            bit_or,
            bit_xor,
        };

        mime_node_base(
            mime_array children = {},
            operands operand = operands::equal,
            std::string message = ""
        )
            : children_(std::move(children)),
              operand_(operand),
              message_(std::move(message))
        {
        }

        virtual bool process_data(const char *data, size_t size) const {
            return true;
        }

        virtual ~mime_node_base() = default;

    protected:
        mime_array children_;
        operands operand_;
        std::string message_;
    };

    class mime_node
        : public mime_node_base, private std::variant<
              uint8_t,
              int8_t,
              uint16_t,
              int16_t,
              uint32_t,
              int32_t,
              uint64_t,
              int64_t
          > {
    public:
        using variant::variant;
        using value = variant;

        mime_node(
            value val,
            mime_array children = {},
            operands operand = operands::equal,
            std::string message = ""
        )
            : mime_node_base(std::move(children), operand, std::move(message)),
              variant(val)
        {
        }

        bool process_data(const char *data, size_t size) const override {
            bool result = false;



            switch (operand_) {
            case operands::equal: {
                std::visit(
                    [&](const auto& val) {
                        result = convert_raw_cast(data, val) == val;
                    }
                    , static_cast<value>(*this)
                    );
                break;
            }
            case operands::not_equal: {
                std::visit(
                    [&](const auto& val) {
                        result = convert_raw_cast(data, val) == val;
                    }
                    , static_cast<value>(*this)
                    );
                break;
            }
            case operands::less_than: {
                static_assert(std::holds_alternative<std::string>(*this));
                std::visit(
                    [&](const auto& val) {
                        result = convert_raw_cast(data, val) == val;
                    }
                    , static_cast<value>(*this)
                    );
                break;
            }
            case operands::greater_than: {
                static_assert(std::holds_alternative<std::string>(*this));
                std::visit(
                    [&](const auto& val) {
                        result = convert_raw_cast(data, val) == val;
                    }
                    , static_cast<value>(*this)
                    );
                break;
            }
            }

            for (const auto& node : children_) {
                result &= node->process_data(data, size);
                if (result == true) {
                    break;
                }
            }
            // Make switch statement for operands
            return result;
        }

    private:

    };
} // magic

#endif //MIME_NODE_H
