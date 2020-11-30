//
// Created by goforbroke on 29.11.2020.
//

#ifndef ADELANTADO_UTILS_H
#define ADELANTADO_UTILS_H

#include <string>
#include <vector>

inline
void shortify(std::string &target, size_t len) {
    if (target.length() > len)
        target.resize(len);
}

inline
void shortify(std::vector<std::string> &target, size_t len) {
    if (target.size() > len)
        target.resize(len);
}

#endif //ADELANTADO_UTILS_H
