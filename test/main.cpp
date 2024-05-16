#include <fstream>
#include <iostream>

#include <vector>

#include <chrono>

class Timer {
    public:
        Timer() {
            start = std::chrono::high_resolution_clock::now();
        }

        ~Timer() {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

#include "loader/mime_loader.h"

int main() {
    using namespace std;
    boolalpha(cout);
    system("chcp 1251");

    magic::mime_list nodes;
    {
        Timer t;
        nodes = magic::load("C:\\Sophus-NEW\\modules\\files.etl");
    }
//    ifstream file("magic", ios::in | ios::binary);

    cout << nodes.size() << " node workers SUCCESSFULLY LOADED!" << endl;
    system("pause");

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
        cout << dec << i++ << hex << ")\n" << node.process_data(data.data(), data.size()) << endl;
        cout << endl << std::string(80, '=') << endl << endl;
    }

    ifstream jpeg("jpeg-home.jpg", ios::in | ios::binary);
    jpeg.read(data.data(), data.size());

    cout << "JPEG" << endl;
    cout << std::string(80, '=') << endl << endl;
    i = 1;
    for (const auto& node: nodes) {
        cout << dec << i++ << hex << ")\n" << node.process_data(data.data(), data.size()) << endl;
        cout << endl << std::string(80, '=') << endl << endl;
    }


    return 0;
}
