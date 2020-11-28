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

MultiThreadPageScrapper::MultiThreadPageScrapper(
        const std::vector<std::string> &links,
        size_t multi
)
        : links(links), multi(multi) {}

ObserverResult *MultiThreadPageScrapper::scrape() const {
    auto *result = new ObserverResult();

    VectorBulkSplitter<std::string> splitter(links, multi);
    std::vector<std::thread> threads;

    auto keywordIgnore = loadConfig("./keyword-ignore.txt");

    auto func = [&](const std::vector<std::string> &linksChunk, ObserverResult *result) {

        AbstractPageScanner *scanner = new GumboPageScanner();

        for (const auto &link : linksChunk) {
            auto __processPageStart = Metrics::now();

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
            Metrics::getDownloadPageDuration()->Increment(Metrics::since(__processPageStart).count());

            auto __parsePageStart = Metrics::now();
            try {
                scanner->load(response.content);
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
            resource.pageContentSize = response.content.size();
            resource.charset = ""; // TODO:

            result->pushVisited(resource);
            Logger_Info_F("'%s' ok", link.c_str());

            std::vector<std::string> contentLinks = getLinkAddresses(response.content);
            contentLinks = normalizeHrefsToLinks(contentLinks, url.protocol, url.host);
            if (!contentLinks.empty()) {
                result->appendLinks(contentLinks);
            }
            Metrics::getParsePageDuration()->Increment(Metrics::since(__parsePageStart).count());
            Metrics::getProcessingPageTotalDuration()->Increment(Metrics::since(__processPageStart).count());

        }

        delete scanner;
    };


    for (size_t thi = 0; thi < multi; ++thi) {
        std::vector<std::string> chunk = splitter.getNext();
        std::thread thread(func, chunk, result);
        threads.emplace_back(std::move(thread));
    }
    for (auto &th : threads) {
        th.join();
    }

    return result;
}
