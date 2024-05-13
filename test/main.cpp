#include <fstream>
#include <iostream>

#include <vector>

#include "../src/loader/mime_loader.h"
#include "../src/node/mime_node.h"

int main() {
    using namespace std;

    ifstream file("magic", ios::in | ios::binary);

    auto nodes = magic::load(file);

    string str {"Hello World"};

    boolalpha(cout);

    cout << nodes.front().process_data(str.data(), str.size()) << endl;

    return 0;
}
