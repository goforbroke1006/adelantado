//
// Created by goforbroke on 15.11.2020.
//

#ifndef ADELANTADO_LINKSTORAGE_H
#define ADELANTADO_LINKSTORAGE_H

#include <string>
#include <vector>
#include <map>
#include <postgresql/libpq-fe.h>

class LinkStorage {
public:
    explicit LinkStorage(PGconn *conn);

    void registerLink(const std::string &address);

    void storeLink(
            const std::string &address,
            const std::string &domain,
            const std::string &metaTitle,
            const std::string &metaDescr,
            const std::string &bodyTitle,
            const std::map<std::string, unsigned int> &keywords
    );

    std::vector<std::string> loadUncheckedLinks(
            unsigned int limit,
            const std::vector<std::string> &priorityDomains);

    std::vector<std::string> loadUncheckedLinks(unsigned int limit);

    std::vector<std::string> loadCheckedLinks(unsigned int limit);

private:
    PGconn *mConnection;
};


#endif //ADELANTADO_LINKSTORAGE_H
