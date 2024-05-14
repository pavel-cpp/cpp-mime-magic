#include "mime_loader.h"

#include <sstream>
#include <vector>

using std::string;
using std::string_view;

using namespace magic;

class line_counter {
    public:

        operator string() {
            return cnt_view;
        }

        string operator++() {
            return cnt_view = std::to_string(++line_cnt);
        }

        string operator--() {
            return cnt_view = std::to_string(--line_cnt);
        }

    private:
        size_t line_cnt {};
        string cnt_view {'0'};
} current_line;

size_t parse_offset(string_view line) {
    size_t offset {0};
    for (char c: line) {
        if (c == '\t') {
            break;
        }
        offset = offset * 10 + (c - '0');
    }
    return offset;
}

void remove_operands(string_view& value, string_view operands) {
    for (char c: operands) {
        if (value.front() == c) {
            value.remove_prefix(1);
            return;
        }
    }
}

char parse_symcode(string_view line) {
    using namespace std::literals;

    if (line[1] == 'x') {
        return std::stoi(string(line.substr(2)), nullptr, 16);
    }

    if (isdigit(line[1])) {
        return std::stoi(string(line.substr(1)));
    }

    throw std::runtime_error {
            "Syntax error: Invalid symbol code\n"
            "In line: "s + string(current_line) + "\n"
            + "\tCode: "s + line[0]
    };
}

/*
 * Don't support
 * \o
 * \u \U \N
 * \c
 * TODO(Pavel): Add operands
 */
char parse_escape(char c) {
    using namespace std::literals;
    switch (c) {
        case '\'':
            return '\'';
        case '\"':
            return '\"';
        case '\?':
            return '\?';
        case 'a':
            return '\a';
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        case 'v':
            return '\v';
        case '\\':
            return '\\';
        case '0':
            return '\0';
        default:
            throw std::runtime_error {
                    "Syntax error: Invalid escape sequence\n"
                    "In line: "s + string(current_line) + "\n"s
                    + "\tSequence: \\"s + c
            };
    }
}

string parse_string(string_view line) {
    string result;
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '\\') {
            if (line[i + 1] == 'x' || (isdigit(line[i + 1]) && i + 2 < line.size() && line[i + 2] != '\\')) {
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

uint32_t parse_raw_value(string_view raw_value) {
    using namespace std::literals;

    if (raw_value == "x") {
        return 1;
    }

    if (raw_value.front() == '0') {
        if (isdigit(raw_value[1])) {
            return std::stol(string(raw_value));
        }
        if (raw_value[1] == 'x') {
            return std::stol(string(raw_value), nullptr, 16);
        }
        throw std::runtime_error {
                "Syntax error: Invalid value\n"
                "In line: "s + string(current_line)
                + "\tValue: "s + string(raw_value)
        };
    }
    return std::stol(string(raw_value));
}

mime_node::value parse_value(string_view raw_type, string_view raw_value) {
    using namespace std::literals;

    mime_node::value result_value;

    if (raw_type == "string"s) {
        remove_operands(raw_value, "=!<>");
        return parse_string(raw_value);
    }
    remove_operands(raw_value, "=!<>&|^");

    size_t mask_pos {raw_type.find('&')};

    string_view mask;
    if (mask_pos != string_view::npos) {
        mask = raw_type.substr(mask_pos + 1);
        raw_type.remove_suffix(mask_pos + 1);
    }

    uint32_t value;

    try {
        value = parse_raw_value(raw_value);
    } catch (std::invalid_argument&) {
        throw std::runtime_error {
                "Syntax Error: Invalid raw value\n"
                "In line: "s + string {current_line} + '\n'
                + "\tRaw value: "s + string {raw_value}
        };
    }

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
                                                                                                    "ledate" ||
        raw_type == "bedate") {
        return mime_data<uint32_t>(
                value,
                mask.empty() ? 0xFF : parse_raw_value(mask)
        );
    }
    throw std::runtime_error {
            "Syntax error: Invalid type\n"
            "In line: "s + string(current_line) + '\n'
            + "Type: "s + string(raw_type)
    };
}

mime_node::operands parse_operand(string_view line) {
    switch (line.front()) {
        case '<':
            return mime_node::operands::less_than;
        case '>':
            return mime_node::operands::greater_than;
        case '=':
            return mime_node::operands::equal;
        case '!':
            return mime_node::operands::not_equal;
        case '&':
            return mime_node::operands::bit_and;
        case '|':
            return mime_node::operands::bit_or;
        case '^':
            return mime_node::operands::bit_xor;
        case 'x':
            return mime_node::operands::bit_or; // x is any value
        default:
            return mime_node::operands::equal;
    }
}

std::vector<string_view> split_by_columns(string_view line) {
    using namespace std::literals;

    std::vector<string_view> columns;
    size_t start {0};
    size_t end {line.find('\t')};

    while (end != string_view::npos) {
        columns.push_back(line.substr(start, end - start));
        start = line.find_first_not_of('\t', end + 1);
        if (line[start] == ' ' && columns.size() < 3) {
            throw std::runtime_error {
                    "Syntax Error: TAB Error\n"
                    "In line: "s + std::string {current_line} + '\n'
                    + "\tSpace between columns must be tab"s
            };
        }
        end = line.find('\t', start);
    }

    // Add the last column if any
    if (start != string_view::npos) {
        columns.push_back(line.substr(start));
    }

    if (columns.size() == 3) {
        columns.push_back(""sv);
    }

    return columns;
}

size_t extract_level(string& line) {
    size_t level {0};
    for (char c: line) {
        if (c != '>') {
            break;
        }
        ++level;
    }
    line.erase(0, level);
    return level;
}

mime_list load_nodes(std::istream& in, size_t level) {
    using namespace std::literals;
    string line;
    std::getline(in, line);

    if (line.size() <= 1 || line.front() == '\n' || line.front() == '\r' || line.front() == '#') {
        return {};
    }

    mime_list children;

    do {
        std::cout << line << std::endl;
        ++current_line;
        size_t current_level = extract_level(line);
        if (current_level > level) {
            throw std::runtime_error {
                    "Syntax error: Invalid level\n"
                    "In line: "s + string(current_line)
            };
        }
        if (current_level < level) {
            in.seekg(-static_cast<int64_t>(current_level + line.size() + 1), std::istream::cur);
            break;
        }

        auto columns {split_by_columns(line)};
        if (columns.size() < 4) {
            throw std::runtime_error {
                    "Syntax error: Invalid number of columns\n"
                    "In line: "s + string(current_line)
            };
        }

        if (columns[3].back() == '\r') {
            columns[3].remove_suffix(1);
        }

        // Create a new node
        children.emplace_back(
                parse_offset(columns[0]),
                parse_value(columns[1], columns[2]),
                load_nodes(in, current_level + 1),
                parse_operand(columns[2]),
                string {columns[3]}
        );
    } while (
            std::getline(in, line)
            && line.size() >= 1
            && line.front() != '\r'
            && line.front() != '\n'
            && line.front() != '#'
            );

    return children;
}

mime_list magic::load(std::istream& in) {
    string buffer;

    mime_list nodes;
    while (std::getline(in, buffer)) {
        if (buffer.size() <= 1 || buffer.front() == '#') {
            ++current_line;
            continue;
        }
        in.seekg(-static_cast<int64_t>(buffer.size() + 1), std::istream::cur);
        nodes.emplace_back(0, nullptr, load_nodes(in, 0));
    }

    return nodes;
}
