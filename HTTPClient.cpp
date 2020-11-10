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

HTTPResponse HTTPClient::load(const std::string &url) {
    std::string content;

    CURL *handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeFunc);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &content);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 5); // wait 5 sec
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 5);

    CURLcode res = curl_easy_perform(handle);
    /* Check for errors */
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    /* always cleanup */
    curl_easy_cleanup(handle);

    return HTTPResponse{0, content};
}
