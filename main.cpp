#include <iostream>

#include <csignal>
#include "runtime.h"
#include "repo.h"
#include "app.h"
#include <postgresql/libpq-fe.h>

bool isReady = true;
int statusCode = 0;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    isReady = false;
    statusCode = signum;
}

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
            repo->storeLink(res.address, res.metaTitle, res.bodyKeywords);
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
