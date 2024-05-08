#include <iostream>

#include <vector>
#include "../mime_node.h"

int main() {
    using namespace std;
    std::vector<char> str = {
        -1, 1, 0, 2,
    };
    magic::mime_node node(
    uint8_t(-1)
    );
    cout << node.process_data(str.data(), str.size()) << endl;
    return 0;
}
