#ifndef LOADER_H
#define LOADER_H

#include <istream>

#include "node/mime_node.h"

namespace magic {

    std::vector<mime_node> load(std::istream& in);


} // magic

#endif //LOADER_H
