#include <fstream>
#include <iostream>
#include <locale>
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
//    {
//        Timer t;
//        nodes = std::move(magic::load("C:\\Sophus-NEW\\modules\\files.etl"));
//    }

//    nodes = magic::load("files.etl");
    nodes = magic::load("magic.etl");

    cout << nodes.size() << " node workers SUCCESSFULLY LOADED!" << endl;
//    system("pause");

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
        auto response = node->process_data(data.data(), data.size());
        if (response.has_value()) {
            cout << dec << i << hex << ")\n" << response.value() << endl;
            cout << endl << std::string(80, '=') << endl << endl;
        }
        ++i;
    }

    ifstream corrupt_png("corrupted-image.png", ios::in | ios::binary);
    corrupt_png.read(data.data(), data.size());

    cout << "CORRUPT PNG" << endl;
    cout << std::string(80, '=') << endl << endl;
    i = 1;
    for (const auto& node: nodes) {
        auto response = node->process_data(data.data(), data.size());
        if (response.has_value()) {
            cout << dec << i << hex << ")\n" << response.value() << endl;
            cout << endl << std::string(80, '=') << endl << endl;
        }
        ++i;
    }


    return 0;
}
