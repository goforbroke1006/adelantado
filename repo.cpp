//
// Created by goforbroke on 10.11.2020.
//

#include "repo.h"

#include <fmt/core.h>

Repo::Repo(PGconn *conn)
        : mConnection(conn) {}


void
Repo::registerLink(const std::string &address) {
    std::string sql = ""
                      "INSERT INTO links (address, meta_title, meta_description, body_title, body_keywords) "
                      "VALUES ('" + address + "', '', '', '', '{}')"
                                              "ON CONFLICT DO NOTHING";

    PGresult *res = PQexec(mConnection, sql.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "failed: %s", PQerrorMessage(mConnection));
    }
    PQclear(res);
}

void
Repo::storeLink(
        const std::string &address,
        const std::string &metaTitle,
        const std::map<std::string, unsigned int> &keywords
) {
    std::string sql = ""
                      "INSERT INTO links ("
                      "   address, "
                      "   meta_title, "
                      "   meta_description, "
                      "   body_title, "
                      "   body_keywords, "
                      "   checked_at"
                      ") VALUES ("
                      "   $1, "
                      "   $2 , "
                      "   '',   "
                      "   '',   "
                      "   $3, "
                      "   NOW() "
                      ")"
                      "ON CONFLICT (address) "
                      "DO UPDATE SET "
                      "  meta_title       = $2, "
                      "  meta_description = '', "
                      "  body_title       = '', "
                      "  body_keywords    = $3  ";

    const char *paramValues[3] = {
            address.c_str(),
            metaTitle.c_str(),
            "{}"
    };
    PGresult *res = PQexecParams(mConnection, sql.c_str(),
                                 3, nullptr, paramValues, nullptr, nullptr, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "failed: %s", PQerrorMessage(mConnection));
    }
    PQclear(res);
}
