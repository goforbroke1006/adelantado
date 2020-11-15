//
// Created by goforbroke on 13.11.2020.
//

#ifndef ADELANTADO_src_html_ABSTRACT_PAGE_SCANNER_H
#define ADELANTADO_src_html_ABSTRACT_PAGE_SCANNER_H

#include <string>
#include <map>
#include <vector>

class AbstractPageScanner {
public:
    virtual ~AbstractPageScanner() = default;

    virtual void load(const std::string &content) = 0;

    virtual std::string getMetaTitle() = 0;

    virtual std::string getMetaDescription() = 0;

    virtual std::string getBodyTitle() = 0;

    virtual std::vector<std::string> getBodyText() = 0;
};

#endif //ADELANTADO_src_html_ABSTRACT_PAGE_SCANNER_H
