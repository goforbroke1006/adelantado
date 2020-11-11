//
// Created by goforbroke on 10.11.2020.
//

#ifndef ADELANTADO_REPO_H
#define ADELANTADO_REPO_H

#include <string>
#include <map>
#include <vector>
#include <postgresql/libpq-fe.h>

class Repo {
public:
    explicit Repo(PGconn *conn);

    void registerLink(const std::string &address);

    void storeLink(
            const std::string &address,
            const std::string &metaTitle,
            const std::map<std::string, unsigned int> &keywords
    );

    std::vector<std::string > loadUncheckedLinks(unsigned int limit);

private:
    PGconn *mConnection;
};

#endif //ADELANTADO_REPO_H
