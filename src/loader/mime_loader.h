#ifndef LOADER_H
#define LOADER_H

#include <istream>

#include "nodes/basic_mime_node.h"

namespace magic {

    mime_list load(std::istream& in);

    mime_list load(const std::string& filename);

} // magic

#endif //LOADER_H
