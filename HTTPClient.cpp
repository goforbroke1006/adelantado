//
// Created by goforbroke on 10.11.2020.
//

#include "HTTPClient.h"

#include <curl/curl.h>

size_t writeFunc(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t realsize = size * nmemb;
    auto *tmp = static_cast<std::string *>(userdata);
    tmp->append(ptr);
    return realsize;
}

HTTPResponse HTTPClient::load(const std::string &url, unsigned int timeout) {
    std::string content;

    CURL *handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeFunc);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &content);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 5);

    CURLcode res = curl_easy_perform(handle);
    /* Check for errors */
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    long response_code;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response_code);

    /* always cleanup */
    curl_easy_cleanup(handle);

    return HTTPResponse{response_code, content};
}
