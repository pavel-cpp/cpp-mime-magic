#include "basic_mime_node.h"

using namespace magic;

basic_mime_node::basic_mime_node(size_t offset, std::string message, mime_list children)
        : offset_ {offset},
          message_ {std::move(message)},
          children_ {std::move(children)}
{
}

basic_mime_node::response_t basic_mime_node::process_data(const char *data, size_t size) {
    if (offset_ > size || !is_enough_data(size - offset_)) {
        return std::nullopt;
    }

    response_t result {process_current(data + offset_, size - offset_)};

    if (!result.has_value()) {
        return result;
    }

    if (children_.empty()) {
        return result;
    }

    response_t handler_result;
    for (const auto& node: children_) {
        response_t node_result = node->process_data(data, size);

        if (node_result.has_value()) {
            if (!handler_result.has_value()) {
                handler_result = node_result.value();
                continue;
            }
            handler_result.value() += node_result.value();
        }
    }

    if (!handler_result.has_value()) {
        return std::nullopt;
    }

    result.value() += handler_result.value();

    return result;
}

bool basic_mime_node::is_enough_data(size_t) {
    return true;
}

basic_mime_node::response_t basic_mime_node::process_current(const char *, size_t) {
    return message_;
}

