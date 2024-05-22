#include <fstream>
#include <stdexcept>
#include <vector>

#include <loader/loading_error.h>
#include <loader/mime_loader.h>

#include "ext/libtest.h"

using namespace std;

void test_loading_correct_data() {
    {
        magic::mime_list nodes = magic::load("test-data/correct/correct-png-file.etl");
        ASSERT_EQUAL_HINT(nodes.size(), 2, "Invalid number of nodes");
    }
    {
        magic::mime_list nodes = magic::load("test-data/correct/hard-correct-png-file.etl");
        ASSERT_EQUAL_HINT(nodes.size(), 2, "Invalid number of nodes");
    }
}

void test_loading_incorrect_data() {
    {
        try {
            magic::mime_list nodes = magic::load("test-data/incorrect/byte-type-error-file.etl");
            ASSERT_HINT(false, "Byte type error file loaded");
        } catch (magic::loading_error& e) {
        }catch (std::invalid_argument& e) {
        }
    }
    {
        try {
            magic::mime_list nodes = magic::load("test-data/incorrect/string-type-error-file.etl");
            ASSERT_HINT(false, "String type error file loaded");
        } catch (std::invalid_argument& e) {
        } catch (magic::loading_error& e) {}
    }
    {
        try {
            magic::mime_list nodes = magic::load("test-data/incorrect/level-error-file.etl");
            ASSERT_HINT(false, "Level error file loaded");
        } catch (magic::loading_error& e) {}
    }
    {
        try {
            magic::mime_list nodes = magic::load("test-data/incorrect/value-error.etl");
            ASSERT_HINT(false, "Value error file loaded");
        } catch (magic::loading_error& e) {}
    }
}

void test_nodes_with_correct_data() {
    magic::mime_list nodes = magic::load("test-data/correct/correct-png-file.etl");
    ifstream correct_png("test-data/png/image.png", ios::binary);
    std::vector<char> image_header;
    image_header.resize(30);
    correct_png.read(image_header.data(), image_header.size());
    auto first_node = nodes.begin();
    {
        const auto result = (*first_node)->process_data(image_header.data(), image_header.size());
        ASSERT_EQUAL_HINT(result.has_value(), false, "Invalid result of processing data");
    }
    {
        const auto result = (*std::next(first_node))->process_data(image_header.data(), image_header.size());
        ASSERT_EQUAL_HINT(result.has_value(), false, "Invalid result of processing data");
    }
}

void test_nodes_with_incorrect_data() {
    magic::mime_list nodes = magic::load("test-data/correct/correct-png-file.etl");
    ifstream correct_png("test-data/png/corrupted-image.png", ios::binary);
    std::vector<char> image_header;
    image_header.resize(30);
    correct_png.read(image_header.data(), image_header.size());
    auto first_node = nodes.begin();
    {
        const auto result = (*first_node)->process_data(image_header.data(), image_header.size());
        ASSERT_EQUAL_HINT(result.has_value(), true, "Invalid result of processing data");
    }
    {
        const auto result = (*std::next(first_node))->process_data(image_header.data(), image_header.size());
        ASSERT_EQUAL_HINT(result.has_value(), false, "Invalid result of processing data");
    }
}

void test_all() {
    RUN_TEST(test_loading_correct_data);
    RUN_TEST(test_loading_incorrect_data);
    RUN_TEST(test_loading_correct_data);
    RUN_TEST(test_loading_incorrect_data);
}

int main() {
    test_all();
    cerr << "All tests passed" << endl;
    return 0;
}