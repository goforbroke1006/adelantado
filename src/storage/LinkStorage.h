//
// Created by goforbroke on 15.11.2020.
//

#ifndef ADELANTADO_LINKSTORAGE_H
#define ADELANTADO_LINKSTORAGE_H

#include <string>
#include <vector>
#include <map>
#include <postgresql/libpq-fe.h>
#include "../domain.h"

class LinkStorage {
public:
    explicit LinkStorage(PGconn *conn);

    void registerLink(const std::string &address);

    void storeLink(const Resource &resource);

    std::vector<std::string> loadUncheckedLinks(
            unsigned int limit,
            const std::vector<std::string> &priorityDomains);

    std::vector<std::string> loadUncheckedLinks(unsigned int limit);

    std::vector<std::string> loadCheckedLinks(unsigned int limit);

    void postpone(const std::string &link);

private:
    PGconn *mConnection;
};


#endif //ADELANTADO_LINKSTORAGE_H
