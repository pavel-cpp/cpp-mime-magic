#include "mime_loader.h"

#include "nodes/date_node/date_node.h"
#include "nodes/numeric_node/numeric_node.h"
#include "nodes/string_node/string_node.h"

#include <sstream>
#include <vector>
#include <optional>
#include <unordered_set>
#include <fstream>
#include <variant>

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

void remove_operands(string_view& value, string_view operands) {
    for (char c: operands) {
        if(c == 'x' && value == "x") {
            value.remove_prefix(1);
            return;
        }
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
        case ' ':
            return ' ';
        default:
            throw std::runtime_error {
                    "Syntax error: Invalid escape sequence\n"
                    "In line: "s + string(current_line) + "\n"s
                    + "\tSequence: \\"s + c
            };
    }
}

string parse_string(string_view line) {
    remove_operands(line, "=!<>x");
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

int64_t parse_raw_value(string_view raw_value) {
    using namespace std::literals;

    if (raw_value == "x") {
        return 1;
    }

    if (raw_value.front() == '0') {
        if (raw_value.size() < 2) {
            return 0l;
        }
        if (isdigit(raw_value[1])) {
            return std::stoll(string(raw_value));
        }
        if (raw_value[1] == 'x') {
            return std::stoll(string(raw_value), nullptr, 16);
        }
        throw std::runtime_error {
                "Syntax error: Invalid value\n"
                "In line: "s + string(current_line)
                + "\tValue: "s + string(raw_value)
        };
    }

    if (raw_value.empty()) {
        return 0;
    }

    return std::stoll(string(raw_value));
}

operands parse_operand(string_view line) {
    if (line == "x") {
        return operands::any;
    }
    switch (line.front()) {
        case '<':
            return operands::less_than;
        case '>':
            return operands::greater_than;
        case '=':
            return operands::equal;
        case '!':
            return operands::not_equal;
        case '&':
            return operands::bit_and;
        case '|':
            return operands::bit_or;
        case '^':
            return operands::bit_xor;
        default:
            return operands::equal;
    }
}

struct node_context {
    size_t offset {};
    std::string message {};
    mime_list mimes {};

    node_context() = default;

    node_context(size_t offset, std::string message, mime_list&& mimes) : offset(offset), message(message),
                                                                          mimes(std::move(mimes)) {}

    node_context(const node_context&) = delete;

    node_context(node_context&&) = default;
};

std::unique_ptr<basic_mime_node> create_string(node_context context, std::string_view raw_type, string_view raw_value) {
    return std::make_unique<string_node>(
            context.offset,
            string_node::data_template {
                    parse_string(raw_value),
                    string_node::options::none,
                    parse_operand(raw_value)
            },
            context.message,
            std::move(context.mimes)
    );
}

std::unique_ptr<basic_mime_node> create_date(node_context context, std::string_view raw_type, string_view raw_value) {
    using namespace std::literals;
    date_node::data_template data;
    if (raw_type.substr(2) == "be"sv) {
        data.normalize_byte_order = utils::change_order<time_t>;
    } else {
        data.normalize_byte_order = [](auto val) { return val; };
    }
    data.operand = parse_operand(raw_value);
    remove_operands(raw_value, "=!<>x");
    data.value = parse_raw_value(raw_value);
    return std::make_unique<date_node>(
            context.offset,
            data,
            context.message,
            std::move(context.mimes)
    );
}

std::unique_ptr<basic_mime_node>
create_numeric(node_context context, std::string_view raw_type, string_view raw_value) {
    using namespace std::literals;
    operands operand = parse_operand(raw_value);
    remove_operands(raw_value, "=!<>&|^x");

    uint64_t mask {~0ull};
    {
        size_t mask_pos = raw_type.find('&');
        string tmp_mask;
        if (mask_pos != string_view::npos) {
            tmp_mask = raw_type.substr(mask_pos + 1);
            raw_type.remove_suffix(raw_type.size() - mask_pos);
        }
        if (!tmp_mask.empty()) {
            mask = parse_raw_value(tmp_mask);
        }
    }

    numeric_node::types final_value;
    numeric_node::types final_mask;
    std::function<void(char *, size_t)> byte_order_normalizer = [](char *, size_t) {};

    if (raw_type == "byte"sv) {
        return std::make_unique<numeric_node>(
                context.offset,
                numeric_node::data_template {
                        static_cast<uint8_t>(parse_raw_value(raw_value)),
                        static_cast<uint8_t>(mask),
                        operand,
                        byte_order_normalizer
                },
                context.message,
                std::move(context.mimes)
        );
    }

    bool sign = raw_type.front() != 'u';
    if (!sign) {
        raw_type.remove_prefix(1);
    }

    if (raw_type.substr(0, 2) == "be") {
        raw_type.remove_prefix(2);
        byte_order_normalizer = utils::change_raw_order;
    } else if (raw_type.substr(0, 2) == "le") {
        raw_type.remove_prefix(2);
    }

    if (raw_type == "short") {
        if (sign) {
            final_value = static_cast<int16_t>(parse_raw_value(raw_value));
            final_mask = static_cast<int16_t>(mask);
        } else {
            final_value = static_cast<uint16_t>(parse_raw_value(raw_value));
            final_mask = static_cast<uint16_t>(mask);
        }
    } else if (raw_type == "long") {
        if (sign) {
            final_value = static_cast<int32_t>(parse_raw_value(raw_value));
            final_mask = static_cast<int32_t>(mask);
        } else {
            final_value = static_cast<uint32_t>(parse_raw_value(raw_value));
            final_mask = static_cast<uint32_t>(mask);
        }
    } else {
        throw std::runtime_error {"Tyta Syntax Error: Unknown type\n"s +
                                  "In line: " + std::string(current_line) + "\n" +
                                  "Type: " + std::string(raw_type)
        };
    }
    return std::make_unique<numeric_node>(
            context.offset,
            numeric_node::data_template {
                    final_value,
                    final_mask,
                    operand,
                    byte_order_normalizer
            },
            context.message,
            std::move(context.mimes)
    );
}

std::unique_ptr<basic_mime_node> create(node_context context, std::string_view raw_type, string_view raw_value) {
    std::unique_ptr<basic_mime_node> result;
    using namespace std::literals;
    if (raw_type.find("string"s) == 0) {
        return create_string(std::move(context), raw_type, raw_value);
    }

    if (raw_type.find("date") != string_view::npos) {
        return create_date(std::move(context), raw_type, raw_value);
    }

    if (
            raw_type.find("byte") != string_view::npos
            || raw_type.find("short") != string_view::npos
            || raw_type.find("long") != string_view::npos
            ) {
        return create_numeric(std::move(context), raw_type, raw_value);
    }

    throw std::runtime_error {"Zdesya Syntax Error: Unknown type\n"s +
                              "In line: " + std::string(current_line) + "\n" +
                              "Type: " + std::string(raw_type)
    };

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
        throw std::runtime_error {"Syntax Error"}; // TODO(Pavel): Write reason
    }

    if (left < line.end()) {
        columns.emplace_back(left, std::distance(left, line.end()));
    } else {
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

    mime_list current_level_nodes;
    bool end_of_node = false;

    do {
        ++current_line;
        if (line.front() == '#'
            || line.size() <= 1
            || line.front() == '\n'
            || line.front() == '\r'
                ) {
            continue;
        }

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
            if (current_level == 0) {
                return {std::move(mime_list {}), true};
            }
            break;
        }

        auto columns {split_by_columns(line)};
        if (columns.size() < 4) {
            throw std::runtime_error {
                    "Syntax error: Invalid number of columns\n"
                    "In line: "s + string(current_line)
            };
        }

        std::string message {columns[3]};
        if (message.back() == '\r') {
            message.back() = ' ';
        }

        auto [children, status] = load_nodes(in, current_level + 1);
        end_of_node = status;
        // Create a new node
        current_level_nodes.emplace_back(std::move(
                create(
                        {
                                static_cast<size_t>(parse_raw_value(columns[0])),
                                message,
                                std::move(children)
                        },
                        columns[1],
                        columns[2]
                ))
        );

    } while (!end_of_node && std::getline(in, line));

    return {std::move(current_level_nodes), end_of_node};
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
        mime_list mimes {std::move(load_nodes(in, 0).first)};
        if (mimes.empty()) {
            continue;
        }
        nodes.emplace_back(std::make_unique<basic_mime_node>(0, "", std::move(mimes)));
    }

    return std::move(nodes);
}

mime_list magic::load(const string& filename) {
    std::ifstream file {filename, std::ios::in | std::ios::binary};
    return std::move(load(file));
}
