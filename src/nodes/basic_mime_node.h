#ifndef _MIME_MAGIC_BASIC_MIME_NODE_H_
#define _MIME_MAGIC_BASIC_MIME_NODE_H_

#include <string>
#include <list>
#include <memory>
#include <optional>
#include <utility>

namespace magic {

    class basic_mime_node;

    using mime_string = std::string;
    using mime_list = std::list<std::unique_ptr<basic_mime_node>>;

    class basic_mime_node {
        public:
            using response_t = std::optional<std::string>;

            basic_mime_node() = delete;

            basic_mime_node(size_t offset, std::string message, mime_list children);

            basic_mime_node(basic_mime_node&&) noexcept = default;

            basic_mime_node(const basic_mime_node&) = delete;

            response_t process_data(const char *data, size_t size);

            virtual ~basic_mime_node() = default;

        protected:
            virtual bool is_enough_data(size_t);

            virtual response_t process_current(const char *, size_t);

        private:
            size_t offset_ {0};
        protected:
            std::string message_ {};
        private:
            mime_list children_ {};


    };

}

#endif