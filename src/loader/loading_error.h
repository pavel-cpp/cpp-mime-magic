#ifndef _MIME_MAGIC_LOADING_ERROR_H_
#define _MIME_MAGIC_LOADING_ERROR_H_

#include <stdexcept>
#include <string>

namespace magic {

    class loading_error : public std::exception {
        public:

            loading_error() = default;

            explicit loading_error(
                    size_t line,
                    const std::string& message,
                    const std::string& recommendation = ""
            ) :
            line_ {line},
            message_ {message},
            recommendation_ {recommendation}
            {
            }

            loading_error(const loading_error& other) = default;

            loading_error(loading_error&& other) = default;

            [[nodiscard]] const char *what() const noexcept override {
                std::string result {"Loading error: "};

                result += message_;
                result += "\nIn line: ";
                result += std::to_string(line_);
                if (!recommendation_.empty()) {
                    result += "\nRecommendation";
                    result += recommendation_;
                }
                return result.c_str();
            }

        private:

            size_t line_ {};
            std::string message_ {};
            std::string recommendation_ {};
    };
}

#endif //_MIME_MAGIC_LOADING_ERROR_H_
