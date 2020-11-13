//
// Created by goforbroke on 13.11.2020.
//

#ifndef ADELANTADO_src_html_GUMBO_PAGE_SCANNER_H
#define ADELANTADO_src_html_GUMBO_PAGE_SCANNER_H


#include "AbstractPageScanner.h"

#include <gumbo.h>

class GumboPageScanner : public AbstractPageScanner {
public:
    explicit GumboPageScanner();

    virtual ~GumboPageScanner();

    void load(const std::string &content) override;

    std::string getMetaTitle() override;

    std::string getMetaDescription() override;

    std::string getBodyTitle() override;

    std::map<std::string, unsigned int> getBodyKeywords() override;

private:
    GumboOutput *mOutput;

    void getBodyTitleRecursively(GumboNode *node, std::string &result);
};


#endif //ADELANTADO_src_html_GUMBO_PAGE_SCANNER_H
