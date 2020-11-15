//
// Created by goforbroke on 15.11.2020.
//

#ifndef ADELANTADO_src_storage_COMMON_H
#define ADELANTADO_src_storage_COMMON_H

#include <string>

inline bool isDuplicateError(const std::string &message) {
    return message.find("duplicate key value violates unique constraint") != std::string::npos;
}

class DuplicateKeyException : public std::exception {
public:
    DuplicateKeyException(const std::string &message)
            : mMessage(message) {}

    const char *what() const throw() {
        return mMessage.c_str();
    }

private:
    std::string mMessage;
};

#endif //ADELANTADO_src_storage_COMMON_H
