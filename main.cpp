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

    libconfig::Config *config = loadAppConfig("adelantado.cfg");

    std::cout << "Allowed CPUs: " << getCPUCount() << std::endl;

    std::string dbUser = config->lookup("db_username");
    std::string dbPass = config->lookup("db_password");
    std::string dbHost = config->lookup("db_host");
    int dbPort;
    config->lookupValue("db_port", dbPort);
    std::string dbName = config->lookup("db_name");
    std::string dbConnStr =
            "postgresql://" + dbUser + ":" + dbPass
            + "@" + dbHost + ":" + std::to_string(dbPort) + "/" + dbName +
            "?connect_timeout=10";
    PGconn *conn = PQconnectdb(dbConnStr.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s",
                PQerrorMessage(conn));
        PQfinish(conn);
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
                    res.metaTitle,
                    res.metaDescr,
                    res.bodyTitle,
                    res.bodyKeywords
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
