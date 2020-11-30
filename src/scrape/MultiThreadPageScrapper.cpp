//
// Created by goforbroke on 28.11.2020.
//

#include <thread>
#include <logger.h>

#include "MultiThreadPageScrapper.h"
#include "VectorBulkSplitter.h"
#include "../parser/AbstractPageScanner.h"
#include "../parser/GumboPageScanner.h"
#include "../Metrics.h"
#include "../parser/KeywordEntries.h"
#include "../url.h"
#include "../cfgloader.h"
#include "../HTTPClient.h"
#include "../html-filter.h"

#include "utils.h"
#include "../char-conv.h"

MultiThreadPageScrapper::MultiThreadPageScrapper(
        const std::vector<std::string> &links,
        size_t multi
)
        : links(links), multi(multi) {}

std::shared_ptr<ObserverResult> MultiThreadPageScrapper::scrape() const {
    auto result = std::make_shared<ObserverResult>();

    VectorBulkSplitter<std::string> splitter(links, multi);
    std::vector<std::thread> threads;

    auto keywordIgnore = loadConfig("./keyword-ignore.txt");

    auto func = [&](const std::vector<std::string> &linksChunk, ObserverResult *result) {

        AbstractPageScanner *scanner = new GumboPageScanner();

        for (const auto &link : linksChunk) {
            auto processPageStart = Metrics::now();

            KeywordEntries keywordEntries(4, keywordIgnore);
            keywordEntries.clear();

            URL url;
            try {
                url = parseURL(link);
            } catch (std::runtime_error &ex) {
                Logger_Warn_F("validate url '%s' failed: %s", link.c_str(), ex.what());

                Metrics::getFailedPageCount()->Increment();
                result->pushFailed(link);
                continue;
            }

            HTTPResponse response;
            try {
                response = HTTPClient::load(link);
            } catch (std::exception &ex) {
                Logger_Warn_F("download '%s' failed: %s", link.c_str(), ex.what());
                Metrics::getFailedPageCount()->Increment();

                {
                    Resource resource;

                    resource.address = link;
                    resource.domain = url.host;
                    resource.statusCode = -1;

                    result->pushVisited(resource);
                }

                continue;
            }

            Metrics::getDownloadPageCount()->Increment();
            Metrics::getDownloadPageDuration()->Increment(Metrics::since(processPageStart));

            auto parsePageStart = Metrics::now();
            std::string &content = response.content;

            try {
                scanner->load(content);
            } catch (std::runtime_error &ex) {
                Logger_Warn_F("parse '%s' failed: %s", link.c_str(), ex.what());

                Metrics::getFailedPageCount()->Increment();
                result->pushFailed(link);
                continue;
            }

            for (const auto &line : scanner->getBodyText()) {
                keywordEntries.appendPhrase(line);
            }


            std::map<std::string, unsigned int> keywordsMap = keywordEntries.getTop();
            if (keywordsMap.empty()) {
                Metrics::getFailedPageCount()->Increment();
                result->pushFailed(link);
                continue;
            }

            if (Charset::UTF8 != scanner->getCharset()) {
                Logger_Warn_F("parse '%s' failed: unexpected charset %s",
                              link.c_str(), getCharsetLabel(scanner->getCharset()).c_str());
                {
                    Resource resource;

                    resource.address = link;
                    resource.domain = url.host;
                    resource.statusCode = -1;
                    resource.charset = getCharsetLabel(scanner->getCharset());

                    result->pushVisited(resource);
                }
                continue;
            }

            Resource resource;

            resource.address = link;
            resource.domain = url.host;
            //
            resource.metaTitle = scanner->getMetaTitle();
            resource.metaDescr = scanner->getMetaDescription();
            resource.metaKeywords = scanner->getMetaKeywords();
            resource.bodyTitle = scanner->getBodyTitle();
            resource.bodyKeywords = keywordsMap;
            //
            resource.ogTitle = scanner->getOGTitle();
            resource.ogImage = scanner->getOGImage();
            resource.ogDescription = scanner->getOGDescription();
            resource.ogSiteName = scanner->getOGSiteName();
            //
            resource.statusCode = response.statusCode;
            resource.pageContentSize = content.size();
            resource.charset = ""; // TODO:


            //

            shortify(resource.metaTitle, 2047);
            shortify(resource.metaDescr, 2047);
            shortify(resource.metaKeywords, 15);
            shortify(resource.bodyTitle, 2047);

            shortify(resource.ogTitle, 2047);
            shortify(resource.ogImage, 2047);
            shortify(resource.ogDescription, 2047);
            shortify(resource.ogSiteName, 2047);

            result->pushVisited(resource);

            std::vector<std::string> contentLinks = getLinkAddresses(content);
            contentLinks = normalizeHrefsToLinks(contentLinks, url.protocol, url.host);
            if (!contentLinks.empty()) {
                result->appendLinks(contentLinks);
            }
            Metrics::getParsePageDuration()->Increment(Metrics::since(parsePageStart));
            Metrics::getProcessingPageTotalDuration()->Increment(Metrics::since(processPageStart));

        }

        delete scanner;
    };


    for (size_t thi = 0; thi < multi; ++thi) {
        std::vector<std::string> chunk = splitter.getNext();
        std::thread thread(func, chunk, result.get());
        threads.emplace_back(std::move(thread));
    }
    for (auto &th : threads) {
        th.join();
    }

    return result;
}
