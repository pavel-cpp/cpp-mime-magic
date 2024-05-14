#ifndef LOADER_H
#define LOADER_H

#include <istream>

#include "node/mime_node.h"

namespace magic {

    mime_list load(std::istream& in);


} // magic

#endif //LOADER_H
