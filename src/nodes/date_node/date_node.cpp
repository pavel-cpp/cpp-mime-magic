#include <ctime>
#include "date_node.h"


using namespace magic;

bool date_node::is_enough_data(size_t size) {
    return sizeof(value_) < size;
}

basic_mime_node::response_t date_node::process_current(const char *data, size_t size) {
    time_t tmp = utils::convert_raw<time_t>(data);
    if (byte_order_ == orders::big_endian) {
        tmp = utils::change_order(tmp);
    }
    response_t response;
    std::string result = utils::format(message_, std::string {std::ctime(&tmp)});
    switch (operand_) {
        case operands::any: return response_t { result };
        case operands::equal: return tmp == value_ ? response_t { result } : std::nullopt;
        case operands::not_equal: return tmp != value_ ? response_t { result } : std::nullopt;
        case operands::less_than: return tmp < value_ ? response_t { result } : std::nullopt;
        case operands::greater_than: return tmp > value_ ? response_t { result } : std::nullopt;
    }
}

date_node::date_node(size_t offset, const date_node::data_template& data, std::string message, mime_list children)
        : basic_mime_node {offset, message, std::move(children)},
          value_ {data.value},
          byte_order_ {data.byte_order},
          operand_ {data.operand} {
}
