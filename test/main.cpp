#include <fstream>
#include <iostream>

#include <vector>

#include "../src/loader/mime_loader.h"
#include "../src/node/mime_node.h"

int main() {
    using namespace std;

//    ifstream file("C:\\Sophus-NEW\\modules\\files.etl", ios::in | ios::binary);
    ifstream file("magic", ios::in | ios::binary);

    auto nodes = magic::load(file);

    string str {"HelloWorld"};

    vector<char> data {'H', 'e', 'l', 'l', 'o', 'W', 'o', 'r', 'l', 'd', static_cast<char>(0xFF), static_cast<char>(0xFF)};

    boolalpha(cout);

    cout << nodes.front().process_data(data.data(), data.size()) << endl;

    return 0;
}
