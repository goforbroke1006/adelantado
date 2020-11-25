//
// Created by goforbroke on 10.11.2020.
//

#ifndef ADELANTADO_HTTPCLIENT_H
#define ADELANTADO_HTTPCLIENT_H


#include <string>

struct HTTPResponse {
    long statusCode;
    std::string content;
};

size_t writeFunc (char *ptr, size_t size, size_t nmemb, void *userdata);

class HTTPClient {
public:
    static HTTPResponse load(const std::string & url, unsigned int timeout = 10);
};


#endif //ADELANTADO_HTTPCLIENT_H
