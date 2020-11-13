#include <iostream>

#include <csignal>
#include "runtime.h"
#include "repo.h"
#include "app.h"
#include "src/html/AbstractPageScanner.h"
#include "src/html/GumboPageScanner.h"
#include <postgresql/libpq-fe.h>

bool isReady = true;
int statusCode = 0;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    isReady = false;
    statusCode = signum;
}

ObserverResult *
MultiThreadLinksObserver(
        const std::vector<std::string> &links,
        size_t multi
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

    Repo *repo = new Repo(conn);

    while (isReady) {
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<std::string> links = repo->loadUncheckedLinks(1000);
        if (links.empty()) {
            links = repo->loadCheckedLinks(1000);
        }

        ObserverResult *pResult = MultiThreadLinksObserver(links, getCPUCount());
        for (auto &res : pResult->getMVisitedLinks()) {
            repo->storeLink(
                    res.address,
                    res.metaTitle,
                    res.metaDescr,
                    res.bodyTitle,
                    res.bodyKeywords
            );
        }
        for (auto &queuedLink : pResult->getMQueuedLinks()) {
            if (queuedLink.rfind("http", 0) != 0) {
                continue;
            }
            repo->registerLink(queuedLink);
        }

        auto finish = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
        std::cout << "Spend time: " << (static_cast<double >(duration) / 1000) << " sec." << std::endl;

        sleep(10);
    }

    return statusCode;
}

ObserverResult *MultiThreadLinksObserver(const std::vector<std::string> &links, size_t multi) {
    VectorBulkSplitter splitter(links, multi);
    std::vector<std::thread> threads;

    auto func = [](const std::vector<std::string> &linksChunk, ObserverResult *result) {
        AbstractPageScanner *scanner = new GumboPageScanner();

        for (const auto &link : linksChunk) {
            const HTTPResponse &response = HTTPClient::load(link);
            URL url;
            try {
                url = parseURL(link);
            } catch (std::runtime_error &err) {
                // TODO: log errors
                continue;
            }

            scanner->load(response.content);

            result->pushVisited(Resource{
                    link,
                    scanner->getMetaTitle(),
                    scanner->getMetaDescription(),
                    scanner->getBodyTitle(),
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

    auto *observerResult = new ObserverResult;

    for (size_t thi = 0; thi < multi; ++thi) {
        std::vector<std::string> chunk = splitter.getNext();
        std::thread thread(func, chunk, observerResult);
        threads.emplace_back(std::move(thread));
    }
    for (auto &th : threads) {
        th.join();
    }

    return observerResult;
}
