#include "string_node.h"

#include "nodes/utils.h"

#include <algorithm>

using namespace magic;

void tolower(std::string& str) {
    for (char& c: str) {
        c = tolower(c);
    }
}

basic_mime_node::response_t string_node::process_current(const char *data, size_t size) {
    using namespace std::literals;
    if (!value_.empty() && value_.size() < size && value_ != "\0"s) {
        size = value_.size();
    }
    std::string temp {data, size};
    if (opt_ == options::not_case_sensitive) {
        tolower(temp);
        tolower(value_);
    }
    std::string result {utils::format(message_, std::string(data, size))};
    switch (operand_) {
        case operands::any:             return result;
        case operands::equal:           return temp == value_ ? response_t {message_} : std::nullopt;
        case operands::less_than:       return temp < value_ ? response_t {message_} : std::nullopt;
        case operands::greater_than:    return temp > value_ ? response_t {message_} : std::nullopt;
        default:
            throw std::invalid_argument("Unknown operand");
    }
    return std::nullopt;
}

bool string_node::is_enough_data(size_t size) {
    return value_.size() < size;
}

string_node::string_node(
        size_t offset,
        const string_node::data_template& data,
        std::string message,
        mime_list children
        ) : basic_mime_node {offset, std::move(message), std::move(children)},
        value_ {data.value},
        opt_ {data.opt},
        operand_ {data.operand}
        {}
