#include "mime_loader.h"

#include <sstream>

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
    for (char c : operands) {
        if (value.front() == c) {
            value.remove_prefix(1);
            return;
        }
    }
}

char parse_symcode(std::string_view line) {
    using namespace std::literals;

    if (line[1] == 'x') {
        return std::stoi(std::string(line.substr(2)), nullptr, 16);
    }

    if (isdigit(line[0])) {
        return std::stoi(std::string(line.substr(1)));
    }

    throw std::runtime_error {
        "Syntax error: Invalid symbol code\n"
        "In line: "s + std::string(line)
    };
}

/*
 * Don't support
 * \o
 * \u \U \N
 * \c
 */
char parse_escape(char c) {
    using namespace std::literals;
    switch (c) {
    case '\'': return '\'';
    case '\"': return '\"';
    case '\?': return '\?';
    case 'a': return '\a';
    case 'b': return '\b';
    case 'f': return '\f';
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    case 'v': return '\v';
    case '\\': return '\\';
    case '\0': return '\0';
    default:
        throw std::runtime_error {
            "Syntax error: Invalid escape sequence\n"
            "In line: "s + c
        };
    }
}

std::string parse_string(std::string_view line) {
    std::string result;
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '\\') {
            if (line[i + 1] == 'x' || (isdigit(line[i + 1]) && line[i + 2] != '\\')) {
                result += parse_symcode(line.substr(i, 4));
                i += 3;
                continue;
            }
            result += parse_escape(line[i + 1]);
            ++i;
            continue;
        }
        result += line[i];
    }
    return result;
}

uint32_t parse_raw_value(std::string_view raw_value) {
    using namespace std::literals;

    if (raw_value == "x") {
        return 1;
    }

    if (raw_value.front() == '0') {
        if (isdigit(raw_value[1])) {
            return std::stol(std::string(raw_value));
        }
        if (raw_value[1] == 'x') {
            return std::stol(std::string(raw_value.substr(2)), nullptr, 16);
        }
        throw std::runtime_error {
            "Syntax error: Invalid value\n"
            "In line: "s + std::string(raw_value)
        };
    }
    return std::stol(std::string(raw_value));
}

mime_node::value parse_value(std::string_view raw_type, std::string_view raw_value) {
    using namespace std::literals;

    mime_node::value result_value;

    if (raw_type == "string"s) {
        remove_operands(raw_value, "=!<>");
        return parse_string(raw_value);
    }
    remove_operands(raw_value, "=!<>&|^");

    size_t mask_pos {raw_type.find('&')};

    std::string_view mask;
    if (mask_pos != std::string_view::npos) {
        mask = raw_type.substr(mask_pos + 1);
        raw_type.remove_suffix(mask_pos + 1);
    }

    uint32_t value {parse_raw_value(raw_value)};

    if (raw_type == "byte") {
        return mime_data<uint8_t>(
            static_cast<uint8_t>(value),
            mask.empty() ? 0xFF : static_cast<uint8_t>(parse_raw_value(mask))
        );
    }
    if (raw_type == "short" || raw_type == "leshort" || raw_type == "beshort") {
        return mime_data<uint16_t>(
            static_cast<uint16_t>(value),
            mask.empty() ? 0xFF : static_cast<uint16_t>(parse_raw_value(mask))
        );
    }
    if (raw_type == "long" || raw_type == "lelong" || raw_type == "belong" || raw_type == "date" || raw_type ==
        "ledate" || raw_type == "bedate") {
        return mime_data<uint32_t>(
            value,
            mask.empty() ? 0xFF : parse_raw_value(mask)
        );
    }
    throw std::runtime_error {
        "Syntax error: Invalid type\n"
        "In line: "s + std::string(raw_type)
    };
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
    case 'x': return mime_node::operands::bit_or; // x is any value
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
    line.erase(0, level);
    return level;
}

std::vector<mime_node> load_nodes(std::istream& in, size_t level) {
    using namespace std::literals;
    std::string line;
    std::getline(in, line);

    if (line.empty() || line.front() == '\n' || line.front() == '#') {
        return {0, nullptr};
    }

    std::vector<mime_node> children;

    do {
        size_t current_level = extract_level(line);
        if (current_level > level) {
            throw std::runtime_error {
                "Syntax error: Invalid level\n"
                "In line: "s + line
            };
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
            load_nodes(in, current_level + 1),
            parse_operand(columns[2]),
            std::string {columns[3]}
      v   );
    } while (
        std::getline(in, line)
        || line.empty()
        || line.front() == '\n'
        || line.front() == '#'
    );

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
        nodes.emplace_back(0, nullptr, load_nodes(in, 0));
    }

    return nodes;
}
