#include "mime_loader.h"

#include <sstream>
#include <vector>
#include <optional>
#include <unordered_set>

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
const std::unordered_set<char> escape {
        '\'', '\"', '\?',
        'a', 'b', 'f',
        'n', 'r', 't',
        'v', '\\', '0'
};

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
            if (escape.find(line[i + 1]) != escape.end()) {
                result += parse_escape(line[i + 1]);
                ++i;
                continue;
            }
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
        if (raw_value.size() < 2) {
            return 0l;
        }
        if (isdigit(raw_value[1])) {
            return std::stoul(string(raw_value));
        }
        if (raw_value[1] == 'x') {
            return std::stoul(string(raw_value), nullptr, 16);
        }
        throw std::runtime_error {
                "Syntax error: Invalid value\n"
                "In line: "s + string(current_line)
                + "\tValue: "s + string(raw_value)
        };
    }
    return std::stoul(string(raw_value));
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

    string mask;
    if (mask_pos != string_view::npos) {
        mask = raw_type.substr(mask_pos + 1);
        raw_type.remove_suffix(raw_type.size() - mask_pos); // TODO: See
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
        if (mask.empty()) {
            return mime_data<uint8_t> {
                    static_cast<uint8_t>(value)
            };
        }
        return mime_data<uint8_t> {
                static_cast<uint8_t>(value),
                static_cast<uint8_t>(parse_raw_value(mask))
        };
    }

    if (raw_type.substr(0, 2) == "be"sv) {
        raw_type.remove_prefix(2);
        if (raw_type == "short"sv) {
            if (mask.empty()) {
                return mime_data<uint16_t> {
                        static_cast<uint16_t>(value),
                        mime_data<uint16_t>::be
                };
            }
            return mime_data<uint16_t> {
                    static_cast<uint16_t>(value),
                    static_cast<uint16_t>(parse_raw_value(mask)),
                    mime_data<uint16_t>::be
            };
        }
        if (raw_type == "date"sv || raw_type == "long"sv) {
            if (mask.empty()) {
                return mime_data<uint32_t> {
                        value,
                        mime_data<uint32_t>::be
                };
            }
            return mime_data<uint32_t> {
                    value,
                    parse_raw_value(mask),
                    mime_data<uint32_t>::be
            };
        }
        throw std::runtime_error {
                "Syntax error: Invalid type\n"
                "In line: "s + string(current_line) + '\n'
                + "Type: "s + string(raw_type)
        };
    }

    if (raw_type == "short"sv || raw_type == "leshort"sv) {
        if (mask.empty()) {
            return mime_data<uint16_t> {
                    static_cast<uint16_t>(value)
            };
        }
        return mime_data<uint16_t>(
                static_cast<uint16_t>(value),
                static_cast<uint16_t>(parse_raw_value(mask))
        );
    }
    if (raw_type == "long"sv || raw_type == "lelong"sv || raw_type == "date"sv || raw_type == "ledate"sv) {
        if (mask.empty()) {
            return mime_data<uint32_t> {value};
        }
        return mime_data<uint32_t>(
                value,
                parse_raw_value(mask)
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

    std::vector<std::string_view> columns;
    std::string_view::iterator left = line.begin();
    std::string_view::iterator right = std::find_if(left, line.end(), isspace);
    while (left != line.end() && columns.size() < 3) {
        columns.emplace_back(left, std::distance(left, right));
        left = right;
        char prev = ' ';
        left = std::find_if(left, line.end(), [](char c) { return !isspace(c); });
        right = std::find_if(left, line.end(),
                             [&prev](char c) {
                                 if (isspace(c) && prev != '\\') {
                                     prev = c;
                                     return true;
                                 }
                                 prev = c;
                                 return false;
                             });
    }

    if (columns.size() != 3) {
        throw std::runtime_error{"Syntax Error"}; // TODO(Pavel): Write reason
    }

    if (left < line.end()) {
        columns.emplace_back(left, std::distance(left, line.end()));
    }else {
        columns.emplace_back(""sv);
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

std::pair<mime_list, bool> load_nodes(std::istream& in, size_t level) {
    using namespace std::literals;
    string line;
    std::getline(in, line);

    if (line.size() <= 1 || line.front() == '\n' || line.front() == '\r') {
        ++current_line;
        return {{}, true};
    }

    mime_list current_level_nodes;
    bool end_of_node = false;

    do {
#ifdef LoadDebug
        std::cout << std::string(current_line) << " " << line << std::endl;
#endif
        if (line.front() == '#') {
            continue;
        }

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
            --current_line;
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

        auto [children, status] = load_nodes(in, current_level + 1);
        end_of_node = status;

        // Create a new node
        current_level_nodes.emplace_back(
                parse_offset(columns[0]),
                parse_value(columns[1], columns[2]),
                children,
                parse_operand(columns[2]),
                string {columns[3]}
        );

        if (end_of_node) {
            break;
        }

    } while (
            std::getline(in, line)
            && line.size() >= 1
            && line.front() != '\r'
            && line.front() != '\n'
            );

    return {current_level_nodes, end_of_node};
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
        //                                                                    First is a result
        //                                                                           |
        mime_list mimes = load_nodes(in, 0).first;
        if (mimes.empty()) {
            continue;
        }
        nodes.emplace_back(0, nullptr, mimes);
    }

    return nodes;
}
