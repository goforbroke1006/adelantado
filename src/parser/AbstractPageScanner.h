//
// Created by goforbroke on 13.11.2020.
//

#ifndef ADELANTADO_src_html_ABSTRACT_PAGE_SCANNER_H
#define ADELANTADO_src_html_ABSTRACT_PAGE_SCANNER_H

#include <string>
#include <map>
#include <vector>

enum Charset {
    UNDEFINED,
    UTF8,
    CP1251,
};

inline std::string getCharsetLabel(Charset cs) {
    std::vector<std::string> label({
                                           "UNDEFINED",
                                           "UTF8",
                                           "CP1251",
                                   });
    return label[cs];
}

class AbstractPageScanner {
public:
    virtual ~AbstractPageScanner() = default;

    virtual void load(const std::string &content) = 0;

    virtual Charset getCharset() = 0;

    virtual std::string getMetaTitle() = 0;

    virtual std::string getMetaDescription() = 0;

    virtual std::vector<std::string> getMetaKeywords() = 0;

    virtual std::string getBodyTitle() = 0;

    virtual std::vector<std::string> getBodyText() = 0;

    virtual std::string getOGTitle() = 0;

    virtual std::string getOGImage() = 0;

    virtual std::string getOGDescription() = 0;

    virtual std::string getOGSiteName() = 0;
};

#endif //ADELANTADO_src_html_ABSTRACT_PAGE_SCANNER_H
