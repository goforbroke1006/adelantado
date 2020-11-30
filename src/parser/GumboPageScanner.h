//
// Created by goforbroke on 13.11.2020.
//

#ifndef ADELANTADO_src_html_GUMBO_PAGE_SCANNER_H
#define ADELANTADO_src_html_GUMBO_PAGE_SCANNER_H


#include "AbstractPageScanner.h"

#include <gumbo.h>
#include <vector>

class GumboPageScanner : public AbstractPageScanner {
public:
    explicit GumboPageScanner();

    ~GumboPageScanner() override;

    void load(const std::string &content) override;

    Charset getCharset() override;

    std::string getMetaTitle() override;

    std::string getMetaDescription() override;

    std::vector<std::string> getMetaKeywords() override;

    std::string getBodyTitle() override;

    std::vector<std::string> getBodyText() override;

    std::string getOGTitle() override;

    std::string getOGImage() override;

    std::string getOGDescription() override;

    std::string getOGSiteName() override;

private:
    std::string mContent;
    GumboOutput *mOutput;

    void getBodyTitleRecursively(GumboNode *node, std::string &result);

    void getTextLinesRecursively(GumboNode *node, std::vector<std::string> &result);

    std::vector<GumboNode *> getMetaNodes(const std::string &attr, const std::string &name);

    std::string getMetaNodesContent(const std::string &attr, const std::string &name);
};


#endif //ADELANTADO_src_html_GUMBO_PAGE_SCANNER_H
