#include <iostream>

#include <csignal>
#include "runtime.h"
#include "cfgloader.h"
#include "HTTPClient.h"
#include "html-filter.h"
#include "url.h"
#include "repo.h"
#include <postgresql/libpq-fe.h>
#include <thread>


bool isReady = true;
int statusCode = 0;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";

    // cleanup and close up stuff here
    // terminate program

    isReady = false;
    statusCode = signum;
}

int main() {
    signal(SIGINT, signalHandler);

    std::cout << "Allowed CPUs: " << getCPUCount() << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    const std::vector<std::string> &keywords = loadConfig("keywords.txt");
//    const std::vector<std::string> &links = loadConfig("links.txt");
    /*const std::vector<std::string> &links = {
//            "https://finance.rambler.ru/",
            "https://postgrespro.ru/docs/postgresql/9.6/sql-insert",
    };*/

    std::string dbConnStr = "postgresql://adelantado:adelantado@localhost:25432/adelantado?connect_timeout=10";
    PGconn *conn = PQconnectdb(dbConnStr.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s",
                PQerrorMessage(conn));
        PQfinish(conn);
        exit(EXIT_FAILURE);
    }

    Repo *repo = new Repo(conn);

//    {
//        auto func = [](const std::vector<std::string> &linksChunk, Repo *repo) {
//            for (const auto &link : linksChunk) {
//                const HTTPResponse &response = HTTPClient::load(link);
//                URL url = parseURL(link);
//
//                std::string metaTitle = getPageTitle(response.content);
//
//                const std::vector<std::string> &contentLinks = getLinkAddresses(response.content);
//                normalizeHrefsToLinks(contentLinks, url.protocol, url.host);
//
//                repo->storeLink(link, metaTitle, {});
//
//                if (!contentLinks.empty()) {
//                    for (auto &cl : contentLinks) {
//                        repo->registerLink(cl);
//                    }
//                    std::cout << link << " : ok" << std::endl;
//                } else {
//                    std::cout << link << " : no links" << std::endl;
//                }
//
//            }
//        };
//
//        size_t chunkSize = links.size() / getCPUCount() + 1;
//        std::vector<std::thread> threads;
//        for (size_t thi = 0; thi < getCPUCount(); ++thi) {
//            if (links.size() <= thi * chunkSize)
//                break;
//
//            const auto &first = links.begin() + thi * chunkSize;
//            const auto &last = ((links.end() - links.begin()) + thi * chunkSize) < chunkSize
//                               ? links.end()
//                               : links.begin() + (thi + 1) * chunkSize;
//            std::vector<std::string> chunk(
//                    first,
//                    last
//            );
//            std::thread thread(func, chunk, repo);
//            threads.emplace_back(std::move(thread));
//        }
//
//        for (auto &th : threads) {
//            th.join();
//        }
//    }

    while (isReady) {
        std::vector<std::string> links = repo->loadUncheckedLinks(1000);
        if (links.empty()) {
            links = loadConfig("links.txt");
        }

        for (const auto &link : links) {
            const HTTPResponse &response = HTTPClient::load(link);
            URL url;
            try {
                url = parseURL(link);
            } catch (std::runtime_error &err) {
                std::cerr << "Can't parse URL " << link << " : " << err.what() << std::endl;
                continue;
            }

            std::string metaTitle = getPageTitle(response.content);

            const std::vector<std::string> &contentLinks = getLinkAddresses(response.content);
            normalizeHrefsToLinks(contentLinks, url.protocol, url.host);

            repo->storeLink(link, metaTitle, {});

            if (!contentLinks.empty()) {
                for (auto &cl : contentLinks) {
                    if (cl.rfind("http", 0) != 0) {
                        break;
                    }
                    repo->registerLink(cl);
                }
                std::cout << link << " : ok" << std::endl;
            } else {
                std::cout << link << " : no links" << std::endl;
            }

        }

        sleep(5);
    }

    auto finish = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    std::cout << "Spend time: " << (static_cast<double >(duration) / 1000) << " sec." << std::endl;

    return statusCode;
}
