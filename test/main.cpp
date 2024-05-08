#include <iostream>

#include <vector>
#include "../mime_node.h"

int main() {
    using namespace std;
    /*std::vector<char> str = {
        -1, 1, 0, 2,
    };*/
    std::string str = "Hello  World!";
    magic::mime_node node(
        "Hello",
        {
            {
                uint8_t(','),
                {
                    {
                        uint8_t(' '),
                        {
                            {
                                "World!"
                            }
                        }
                    },
                    {
                        "World!"
                    }
                }
            },
            {
                uint8_t(' '),
                {
                    {
                        "World!"
                    }
                }
            },
            {
                "World!"
            }
        }
    );
    cout << node.process_data(str.data(), str.size()) << endl;
    return 0;
}
