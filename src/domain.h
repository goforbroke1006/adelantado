//
// Created by goforbroke on 28.11.2020.
//

#ifndef ADELANTADO_DOMAIN_H
#define ADELANTADO_DOMAIN_H

#include <string>
#include <vector>
#include <map>

struct Resource {
    std::string address;
    std::string domain;
    std::string metaTitle;
    std::string metaDescr;
    std::vector<std::string> metaKeywords;
    std::string bodyTitle;
    std::map<std::string, unsigned int> bodyKeywords;

    std::string ogTitle;
    std::string ogImage;
    std::string ogDescription;
    std::string ogSiteName;

    long statusCode;
    long pageContentSize;
    std::string charset;
};

#endif //ADELANTADO_DOMAIN_H
