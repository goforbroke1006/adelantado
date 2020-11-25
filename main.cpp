#include <iostream>

#include <csignal>
#include "src/runtime.h"
#include "src/app.h"
#include "src/parser/AbstractPageScanner.h"
#include "src/parser/GumboPageScanner.h"
#include "src/parser/KeywordEntries.h"
#include "src/cfgloader.h"
#include "src/storage/LinkStorage.h"
#include "src/storage/common.h"
#include <postgresql/libpq-fe.h>

#include "src/helper.h"
#include "src/Metrics.h"

#include <prometheus/exposer.h>
#include <prometheus/registry.h>

bool isReady = true;
int statusCode = 0;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    isReady = false;
    statusCode = signum;
}

void
MultiThreadLinksObserver(
        const std::vector<std::string> &links,
        size_t multi,
        ObserverResult *pResult
);

int main() {
    signal(SIGINT, signalHandler);

    prometheus::Exposer exposer{"0.0.0.0:8080"};
    auto registry = std::make_shared<prometheus::Registry>();
    // TODO: register metrics
    Metrics::init("adelantado", registry.get());
    exposer.RegisterCollectable(registry);

    libconfig::Config *config = loadAppConfig("adelantado.cfg");

    std::cout << "Allowed CPUs: " << getCPUCount() << std::endl;

    PGconn *conn;
    try {
        conn = openDbConnection(config);
    } catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    int queue_limit_priority,
            queue_limit_unchecked,
            queue_limit_checked, queue_pause;
    config->lookupValue("queue_limit_priority", queue_limit_priority);
    config->lookupValue("queue_limit_unchecked", queue_limit_unchecked);
    config->lookupValue("queue_limit_checked", queue_limit_checked);
    config->lookupValue("queue_pause", queue_pause);

    auto *linkStorage = new LinkStorage(conn);

    auto initLinks = loadConfig("./links.txt");
    for (const auto &il : initLinks) {
        try {
            linkStorage->registerLink(il);
        } catch (DuplicateKeyException &ex) {
            // ignore
        }
    }

    while (isReady) {
        auto start = std::chrono::high_resolution_clock::now();

        auto priorityDomains = loadConfig("./domain-priority.txt");

        std::vector<std::string> links = linkStorage
                ->loadUncheckedLinks(queue_limit_priority, priorityDomains);

        auto uncheckedLinks = linkStorage->loadUncheckedLinks(queue_limit_unchecked);
        links.insert(links.end(), uncheckedLinks.begin(), uncheckedLinks.end());

        auto checkedLinks = linkStorage->loadCheckedLinks(queue_limit_checked);
        links.insert(links.end(), checkedLinks.begin(), checkedLinks.end());


        auto *observerResult = new ObserverResult;
        MultiThreadLinksObserver(links, getCPUCount(), observerResult);

        for (auto &res : observerResult->getMVisitedLinks()) {
            linkStorage->storeLink(
                    res.address,
                    res.domain,
                    res.metaTitle, res.metaDescr,
                    res.bodyTitle, res.bodyKeywords
            );
        }
        for (auto &queuedLink : observerResult->getMQueuedLinks()) {
            if (queuedLink.rfind("http", 0) != 0) { // FIXME: workaround
                continue;
            }
            if (queuedLink.find("'") != std::string::npos) { // FIXME: workaround
                continue;
            }
            if (
                    queuedLink.find(" ") != std::string::npos
                    || queuedLink.find("<") != std::string::npos
                    || queuedLink.find(">") != std::string::npos
                    ) {
                continue;
            }
            try {
                linkStorage->registerLink(queuedLink);
            } catch (DuplicateKeyException &ex) {
                // TODO:
            } catch (std::runtime_error &ex) {
                std::cerr << ex.what() << std::endl;
            }
        }

        for (auto &failedLink : observerResult->getMFailedLinks()) {
            linkStorage->postpone(failedLink);
        }

        delete observerResult;

        auto finish = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
        std::cout << "Spend time: " << (static_cast<double>(duration) / 1000) << " sec." << std::endl;

        sleep(queue_pause);
    }

    delete linkStorage;

    return statusCode;
}

void
MultiThreadLinksObserver(
        const std::vector<std::string> &links,
        size_t multi,
        ObserverResult *pResult
) {
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
                std::cerr << ex.what() << std::endl;
                Metrics::getFailedPageCount()->Increment();
                pResult->pushFailed(link);
                continue;
            }

            HTTPResponse response;
            try {
                response = HTTPClient::load(link);
            } catch (std::exception &ex) {
                std::cerr << ex.what() << std::endl;
                Metrics::getFailedPageCount()->Increment();
                pResult->pushFailed(link);
                continue;
            }

            if (response.statusCode >= 400) {
                std::cerr << link << " : status code " << response.statusCode << std::endl;
                Metrics::getFailedPageCount()->Increment();
                pResult->pushFailed(link);
                continue;
            }

            Metrics::getDownloadPageCount()->Increment();
            Metrics::getDownloadPageDuration()->Increment(Metrics::since(__processPageStart).count());

            auto __parsePageStart = Metrics::now();
            try {
                scanner->load(response.content);
            } catch (std::runtime_error &ex) {
                std::cerr << link << " : " << ex.what() << std::endl;
                Metrics::getFailedPageCount()->Increment();
                pResult->pushFailed(link);
                continue;
            }

            for (const auto &line : scanner->getBodyText()) {
                keywordEntries.appendPhrase(line);
            }


            std::map<std::string, unsigned int> keywordsMap = keywordEntries.getTop();
            if (keywordsMap.empty()) {
                Metrics::getFailedPageCount()->Increment();
                pResult->pushFailed(link);
                continue;
            }

            result->pushVisited(Resource{
                    link,
                    url.host,
                    scanner->getMetaTitle(),
                    scanner->getMetaDescription(),
                    scanner->getBodyTitle(),
                    keywordsMap
            });

            const std::vector<std::string> &contentLinks = getLinkAddresses(response.content);
            normalizeHrefsToLinks(contentLinks, url.protocol, url.host);
            if (!contentLinks.empty()) {
                result->appendLinks(contentLinks);
//                std::cout << link << " : ok" << std::endl;
            } else {
//                std::cout << link << " : no links" << std::endl;
            }
            Metrics::getParsePageDuration()->Increment(Metrics::since(__parsePageStart).count());
            Metrics::getProcessingPageTotalDuration()->Increment(Metrics::since(__processPageStart).count());

        }

        delete scanner;
    };


    for (size_t thi = 0; thi < multi; ++thi) {
        std::vector<std::string> chunk = splitter.getNext();
        std::thread thread(func, chunk, pResult);
        threads.emplace_back(std::move(thread));
    }
    for (auto &th : threads) {
        th.join();
    }
}
