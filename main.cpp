#include <iostream>

#include <csignal>
#include "runtime.h"
#include "app.h"
#include "src/parser/AbstractPageScanner.h"
#include "src/parser/GumboPageScanner.h"
#include "src/parser/KeywordEntries.h"
#include "cfgloader.h"
#include "src/storage/LinkStorage.h"
#include "src/storage/common.h"
#include <postgresql/libpq-fe.h>

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

    std::cout << "Allowed CPUs: " << getCPUCount() << std::endl;

    std::string dbConnStr = "postgresql://adelantado:adelantado@localhost:25432/adelantado?connect_timeout=10";
    PGconn *conn = PQconnectdb(dbConnStr.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s",
                PQerrorMessage(conn));
        PQfinish(conn);
        exit(EXIT_FAILURE);
    }


    auto *linkStorage = new LinkStorage(conn);

    while (isReady) {
        auto start = std::chrono::high_resolution_clock::now();

        auto priorityDomains = loadConfig("./domain-priority.txt");

        std::vector<std::string> links = linkStorage
                ->loadUncheckedLinks(1000, priorityDomains);
        if (links.empty()) {
            links = linkStorage->loadCheckedLinks(1000);
        }

        auto *observerResult = new ObserverResult;
        MultiThreadLinksObserver(links, getCPUCount(), observerResult);

        for (auto &res : observerResult->getMVisitedLinks()) {
            linkStorage->storeLink(
                    res.address,
                    res.metaTitle,
                    res.metaDescr,
                    res.bodyTitle,
                    res.bodyKeywords
            );
        }
        for (auto &queuedLink : observerResult->getMQueuedLinks()) {
            if (queuedLink.rfind("http", 0) != 0) {
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

        delete observerResult;

        auto finish = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
        std::cout << "Spend time: " << (static_cast<double>(duration) / 1000) << " sec." << std::endl;

        sleep(10);
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
    VectorBulkSplitter splitter(links, multi);
    std::vector<std::thread> threads;

    auto keywordIgnore = loadConfig("./keyword-ignore.txt");

    auto func = [&](const std::vector<std::string> &linksChunk, ObserverResult *result) {
        KeywordEntries keywordEntries(4, keywordIgnore);

        AbstractPageScanner *scanner = new GumboPageScanner();

        for (const auto &link : linksChunk) {
            keywordEntries.clear();

            URL url;
            try {
                url = parseURL(link);
            } catch (std::runtime_error &err) {
                // TODO: log errors
                continue;
            }

            const HTTPResponse &response = HTTPClient::load(link);

            if (response.statusCode >= 400) {
                continue;
            }

            try {
                scanner->load(response.content);
            } catch (std::runtime_error &ex) {
                // TODO:
                continue;
            }

            for (const auto &line : scanner->getBodyText()) {
                keywordEntries.appendPhrase(line);
            }


            std::map<std::string, unsigned int> keywordsMap = keywordEntries.getTop();
            if (keywordsMap.empty()) {
                continue;
            }

            result->pushVisited(Resource{
                    link,
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
