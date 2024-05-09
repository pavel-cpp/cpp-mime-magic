#include <iostream>

#include <vector>

#include "../src/loader/mime_loader.h"
#include "../src/node/mime_node.h"

int main() {
    using namespace std;
    /*std::vector<char> str = {
        -1, 1, 0, 2,
    };*/

    std::string str = "HelloWorld!";
    str.push_back(4);
    magic::mime_node n (5, magic::mime_data<uint16_t>(5));

    magic::mime_node node(
        0,
        "Hello",
        {
            {
                5,
                magic::mime_data<uint8_t>(','),
                {
                    {
                        6,
                        magic::mime_data<uint8_t>(' '),
                        {
                            {
                                7,
                                "World!",
                                {
                                    {
                                        13,
                                        magic::mime_data<uint8_t>(5),
                                    },
                                    {
                                        13,
                                        magic::mime_data<uint8_t>(5),
                                        {},
                                        magic::mime_node::operands::less_than
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
                {
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
