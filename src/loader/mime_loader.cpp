#include "mime_loader.h"

using namespace magic;

size_t parse_offset(std::string_view line) {

}

mime_node::value parse_value(std::string_view line) {

}

mime_node::operands parse_operand(std::string_view line) {

}

std::string parse_message(std::string_view line) {

}

size_t extract_level(std::string& line) {
    size_t level {0};
    for(char c: line) {
        if(c != '>') {
            break;
        }
        ++level;
    }
    return level;
}

std::vector<mime_node>  load_node(std::istream& in, size_t level) {
    std::string line;
    std::getline(in, line);

    if (line.empty() || line.front() == '\n' || line.front() == '#') {
        return {0, nullptr};
    }

    std::vector<mime_node> children;

    while(
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
        children.emplace_back(
            parse_offset(line),
            parse_value(line),
            load_node(in, current_level),
            parse_operand(line),
            parse_message(line)
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
