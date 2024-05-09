#include <iostream>

#include <vector>
#include "../mime_node.h"

int main() {
    using namespace std;
    /*std::vector<char> str = {
        -1, 1, 0, 2,
    };*/

    std::string str = "HelloWorld!";
    str.push_back(5);
    magic::mime_node n (5, magic::mime_data<uint16_t>(5));

    magic::mime_node node(
        0,
        "Hello",
        magic::mime_array{
            {
                5,
                magic::mime_data<uint8_t>(','),
                magic::mime_array {
                    {
                        6,
                        magic::mime_data<uint8_t>(' '),
                        {
                            {
                                7,
                                "World!",
                                magic::mime_array {
                                    magic::mime_node {
                                        13,
                                        magic::mime_data<uint8_t>(5),
                                    }
                                }
                            }
                        }
                    },
                    {
                        6,
                        "World!"
                    }
                }
            },
            {
                5,
                magic::mime_data<uint8_t>(' '),
                magic::mime_array{
                    {
                        6,
                        "World!"
                    }
                }
            },
            {
                5,
                "World!"
            }
        }
    );
    boolalpha(cout);
    cout << node.process_data(str.data(), str.size()) << endl;
    return 0;
}
