#include <fstream>
#include <iostream>

#include <vector>

#include "../src/loader/mime_loader.h"
#include "../src/node/mime_node.h"

int main() {
    using namespace std;
    boolalpha(cout);
    system("chcp 1251");

//    ifstream file("C:\\Sophus-NEW\\modules\\files.etl", ios::in | ios::binary);
    ifstream file("magic", ios::in | ios::binary);
    auto nodes = magic::load(file);

//    string str {"HelloWorld"};
//
//    vector<char> data {'H', 'e', 'l', 'l', 'o', 'W', 'o', 'r', 'l', 'd', static_cast<char>(0xFF), static_cast<char>(0xFF)};

    vector<char> data;
    data.resize(29);

    ifstream png("image.png", ios::in | ios::binary);
    png.read(data.data(), data.size());

    cout << "PNG" << endl;
    cout << std::string(80, '=') << endl << endl;
    int i = 1;
    for (const auto& node: nodes) {
        cout << i++ << ")\n" << node.process_data(data.data(), data.size()) << endl;
        cout << endl << std::string(80, '=') << endl << endl;
    }

    ifstream jpeg("image.jpeg", ios::in | ios::binary);
    jpeg.read(data.data(), data.size());

    cout << "JPEG" << endl;
    cout << std::string(80, '=') << endl << endl;
    i = 1;
    for (const auto& node: nodes) {
        cout << i++ << ")\n" << node.process_data(data.data(), data.size()) << endl;
        cout << endl << std::string(80, '=') << endl << endl;
    }


    return 0;
}
