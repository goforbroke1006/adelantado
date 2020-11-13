//
// Created by goforbroke on 13.11.2020.
//

#include "url.h"

URL parseURL(const std::string &url) {

    URL result{"", "", 0, "", ""};
    size_t nextSymbol = 0;

    // take schema
    for (auto letter : url) {
        if (!isalpha(letter)) break;
        result.protocol += letter;
        ++nextSymbol;
    }

    // take :// delimiter
    if (url.at(result.protocol.length() + 0) != ':') throw std::runtime_error("wrong format");
    if (url.at(result.protocol.length() + 1) != '/') throw std::runtime_error("wrong format");
    if (url.at(result.protocol.length() + 2) != '/') throw std::runtime_error("wrong format");
    nextSymbol += 3;

    for (size_t i = nextSymbol; i < url.length(); ++i) {
        char letter = url.at(i);
        if (
                isalnum(letter)
                || letter == '.'
                || letter == '-'
                || letter == '_'
                ) {
            result.host += letter;
            ++nextSymbol;
        } else
            break;
    }

    if (nextSymbol >= url.length())
        return result; // if domain name only

    // check port is defined
    if (url.at(nextSymbol) == ':') {
        ++nextSymbol;
        std::string port;
        for (size_t i = nextSymbol; i < url.length(); ++i) {
            if (!isdigit(url.at(i)))
                break;
            port += url.at(i);
            ++nextSymbol;
        }
        if (port.empty())
            throw std::runtime_error("port can't be empty");
        result.port = atoi(port.c_str());
    }

    for (size_t i = nextSymbol; i < url.length(); ++i) {
        char letter = url.at(i);
        if (letter == '?')
            break;
        result.path += letter;
    }

    result.query = url.substr(nextSymbol);

    return result;
}
