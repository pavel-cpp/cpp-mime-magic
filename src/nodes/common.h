#ifndef _MIME_MAGIC_COMMON_H_
#define _MIME_MAGIC_COMMON_H_

#include <memory>

#include "basic_mime_node.h"

namespace magic {

    enum class operands {
        any,
        equal,
        not_equal,
        less_than,
        greater_than,
        bit_and,
        bit_or,
        bit_xor
    };

}
#endif //_MIME_MAGIC_COMMON_H_
