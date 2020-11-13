//
// Created by goforbroke on 10.11.2020.
//

#ifndef ADELANTADO_URL_H
#define ADELANTADO_URL_H

#include <string>
#include <stdexcept>

struct URL {
    std::string protocol;
    std::string host;
    int port;
    std::string path;
    std::string query;
};

URL parseURL(const std::string &url);

#endif //ADELANTADO_URL_H
