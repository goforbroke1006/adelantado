#include <iostream>

#include <csignal>
#include "src/runtime.h"
#include "src/cfgloader.h"
#include "src/storage/common.h"

#include "src/storage/LinkStorage.h"
#include "src/storage/LinkBatchStorage.h"

#include <postgresql/libpq-fe.h>
#include "src/helper.h"
#include "src/Metrics.h"
#include "src/scrape/ObserverResult.h"
#include "src/scrape/MultiThreadPageScrapper.h"

#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include <logger.h>

bool isReady = true;
int statusCode = 0;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    isReady = false;
    statusCode = signum;
}

int main() {
    signal(SIGINT, signalHandler);
    Logger_Info("Initialization...");

    libconfig::Config *config = loadAppConfig("adelantado.cfg");

    std::string handleAddr;
    config->lookupValue("handle_addr", handleAddr);
    prometheus::Exposer exposer{handleAddr};
    Logger_Info_F("Start listen on '%s'", handleAddr.c_str());

    auto registry = std::make_shared<prometheus::Registry>();
    Metrics::init("adelantado", registry.get());
    exposer.RegisterCollectable(registry);

    Logger_Info_F("allowed CPUs: %d", getCPUCount());

    PGconn *conn;
    try {
        conn = openDbConnection(config);
    } catch (std::exception &ex) {
        Logger_Error_F("open db connection failed: %s", ex.what());
        exit(EXIT_FAILURE);
    }
    Logger_Info("Connected to DB...");

    int queue_limit_priority,
            queue_limit_unchecked,
            queue_limit_checked, queue_pause;
    config->lookupValue("queue_limit_priority", queue_limit_priority);
    config->lookupValue("queue_limit_unchecked", queue_limit_unchecked);
    config->lookupValue("queue_limit_checked", queue_limit_checked);
    config->lookupValue("queue_pause", queue_pause);

    auto *linkStorage = new LinkStorage(conn);
    auto *linkBatchStorage = new LinkBatchStorage(conn);

    auto initLinks = loadConfig("./links.txt");
    for (const auto &il : initLinks) {
        try {
            auto regLinkStart = Metrics::now();
            linkBatchStorage->registerLink(il);
            Metrics::getRegisterLinkDuration()->Increment(Metrics::since(regLinkStart));
            Metrics::getRegisterLinkCount()->Increment();
        } catch (DuplicateKeyException &ex) {
            // ignore
        } catch (std::exception &ex) {
            Logger_Error_F("can't register new link '%s': %s", il.c_str(), ex.what());
        }
    }
    linkBatchStorage->flush();

    while (isReady) {
        auto start = std::chrono::high_resolution_clock::now();

        auto priorityDomains = loadConfig("./domain-priority.txt");

        std::vector<std::string> links = linkStorage
                ->loadUncheckedLinks(queue_limit_priority, priorityDomains);

        auto uncheckedLinks = linkStorage->loadUncheckedLinks(queue_limit_unchecked);
        links.insert(links.end(), uncheckedLinks.begin(), uncheckedLinks.end());

        auto checkedLinks = linkStorage->loadCheckedLinks(queue_limit_checked);
        links.insert(links.end(), checkedLinks.begin(), checkedLinks.end());

        Logger_Info_F("Load %ld links from DB", links.size());

        auto pScrapper = std::make_shared<MultiThreadPageScrapper>(links, getCPUCount());
        auto observerResult = pScrapper->scrape();

        for (auto &res : observerResult->getMVisitedLinks()) {
            try {
                linkStorage->storeLink(res);
            } catch (std::exception &ex) {
                Logger_Error_F("can't visited link '%s': %s", res.address.c_str(), ex.what());
            }
        }
        for (auto &queuedLink : observerResult->getMQueuedLinks()) {
            try {
                auto regLinkStart = Metrics::now();
                linkBatchStorage->registerLink(queuedLink);
                Metrics::getRegisterLinkDuration()->Increment(Metrics::since(regLinkStart));
                Metrics::getRegisterLinkCount()->Increment();
            } catch (DuplicateKeyException &ex) {
                // TODO:
            } catch (std::runtime_error &ex) {
                Logger_Error_F("can't register link '%s': %s", queuedLink.c_str(), ex.what());
            }
        }
        linkBatchStorage->flush();

        for (auto &failedLink : observerResult->getMFailedLinks()) {
            linkStorage->postpone(failedLink);
        }

        auto finish = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(finish - start).count();

        Logger_Info_F("Spend time: '%ld' seconds", duration);

        sleep(queue_pause);
    }

    delete linkStorage;

    return statusCode;
}
