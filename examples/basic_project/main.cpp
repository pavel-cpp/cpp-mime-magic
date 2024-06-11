#include <fstream>
#include <iostream>
#include <locale>
#include <vector>
#include <chrono>

#include "loader/mime_loader.h"

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

int main() {
    using namespace std;
    boolalpha(cout);
    system("chcp 1251");

    magic::mime_list nodes;
//    You can check how it fast
//    {
//        Timer t;
//        nodes = std::move(magic::load("C:\\Sophus-NEW\\modules\\files.etl"));
//    }

    nodes = magic::load("magic.etl");

    cout << nodes.size() << " node workers SUCCESSFULLY LOADED!" << endl;

    vector<char> data;
    data.resize(30);

    {
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
                break;
            }
            ++i;
        }
    }
    data.clear();
    data.shrink_to_fit();
    data.resize(30);

    {
        ifstream corrupt_png("corrupted-image.png", ios::in | ios::binary);
        corrupt_png.read(data.data(), data.size());

        cout << "CORRUPT PNG" << endl;
        cout << std::string(80, '=') << endl << endl;
        size_t i = 1;
        for (const auto& node: nodes) {
            auto response = node->process_data(data.data(), data.size());
            if (response.has_value()) {
                cout << dec << i << hex << ")\n" << response.value() << endl;
                cout << endl << std::string(80, '=') << endl << endl;
                break;
            }
            ++i;
        }
    }

    return 0;
}
