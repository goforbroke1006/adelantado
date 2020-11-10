#include <iostream>

#include "runtime.h"
#include "cfgloader.h"
#include "HTTPClient.h"
#include "html-filter.h"
#include "url.h"
#include "repo.h"
#include <postgresql/libpq-fe.h>
#include <thread>

int main() {
    std::cout << "Allowed CPUs: " << getCPUCount() << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    const std::vector<std::string> &keywords = loadConfig("keywords.txt");
    const std::vector<std::string> &links = loadConfig("links.txt");
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

    {
        for (const auto &link : links) {
            const HTTPResponse &response = HTTPClient::load(link);
            URL url = parseURL(link);

            std::string metaTitle = getPageTitle(response.content);

            const std::vector<std::string> &contentLinks = getLinkAddresses(response.content);
            normalizeHrefsToLinks(contentLinks, url.protocol, url.host);

            repo->storeLink(link, metaTitle, {});

            if (!contentLinks.empty()) {
                for (auto &cl : contentLinks) {
                    repo->registerLink(cl);
                }
                std::cout << link << " : ok" << std::endl;
            } else {
                std::cout << link << " : no links" << std::endl;
            }

        }
    }

    auto finish = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    std::cout << "Spend time: " << (static_cast<double >(duration) / 1000) << " sec." << std::endl;

    return 0;
}
