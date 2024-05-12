#include "mime_loader.h"

using namespace magic;

size_t parse_offset(std::string_view line) {
    size_t offset {0};
    for (char c : line) {
        if (c == '\t') {
            break;
        }
        offset = offset * 10 + (c - '0');
    }
    return offset;
}

void remove_operands(std::string_view& value, std::string_view operands) {
    for(char c: operands) {
        if (value.front() == c) {
            value.remove_prefix(1);
            return;
        }
    }
}

mime_node::value parse_value(std::string_view type, std::string_view value) {
    using namespace std::literals;

    mime_node::value result_value;

    if (type == "string"s) {
        remove_operands(value, "=!<>");
        result_value = std::string{value};

        // TODO(Pavel): Обработай строку

        return result_value;
    }
    remove_operands(value, "=!<>&|^");

}

mime_node::operands parse_operand(std::string_view line) {
    switch (line.front()) {
    case '<': return mime_node::operands::less_than;
    case '>': return mime_node::operands::greater_than;
    case '=': return mime_node::operands::equal;
    case '!': return mime_node::operands::not_equal;
    case '&': return mime_node::operands::bit_and;
    case '|': return mime_node::operands::bit_or;
    case '^': return mime_node::operands::bit_xor;
    default: return mime_node::operands::equal;
    }
}

std::vector<std::string_view> split_by_columns(std::string_view line) {
    using namespace std::literals;

    std::vector<std::string_view> columns;
    size_t start {0};
    size_t end {line.find('\t')};

    while (end != std::string_view::npos) {
        columns.push_back(line.substr(start, end - start));
        start = line.find_first_not_of('\t', end + 1);
        end = line.find('\t', start);
    }

    // Add the last column if any
    if (start != std::string_view::npos) {
        columns.push_back(line.substr(start));
    }

    if (columns.size() == 3) {
        columns.push_back(""sv);
    }

    return columns;
}

size_t extract_level(std::string& line) {
    size_t level {0};
    for (char c : line) {
        if (c != '>') {
            break;
        }
        ++level;
    }
    return level;
}

std::vector<mime_node> load_node(std::istream& in, size_t level) {
    using namespace std::literals;
    std::string line;
    std::getline(in, line);

    if (line.empty() || line.front() == '\n' || line.front() == '#') {
        return {0, nullptr};
    }

    std::vector<mime_node> children;

    while (
        std::getline(in, line)
        || line.empty()
        || line.front() == '\n'
        || line.front() == '#'
    ) {
        size_t current_level = extract_level(line);
        if (current_level > level) {
            in.seekg(in.cur - (current_level + line.size()));
            load_node(in, ++level);
        }
        if (current_level < level) {
            in.seekg(in.cur - (current_level + line.size()));
            break;
        }

        auto columns {split_by_columns(line)};
        if (columns.size() < 4) {
            throw std::runtime_error {
                "Syntax error: Invalid number of columns\n"
                "In line: "s + line
            };
        }

        // Create a new node
        children.emplace_back(
            parse_offset(columns[0]),
            parse_value(columns[1], columns[2]),
            load_node(in, current_level),
            parse_operand(columns[2]),
            std::string{columns[3]}
        );
    }

    return children;
}

std::vector<mime_node> magic::load(std::istream& in) {
    std::string buffer;

    std::vector<mime_node> nodes;
    while (std::getline(in, buffer)) {
        if (buffer.empty() || buffer.front() == '#') {
            continue;
        }
        in.seekg(in.cur - buffer.size());
        nodes.emplace_back(0, nullptr, load_node(in, 0));
    }

    return nodes;
}
