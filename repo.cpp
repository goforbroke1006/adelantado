//
// Created by goforbroke on 10.11.2020.
//

#include "repo.h"

#include <stdexcept>

bool isDuplicateError(const std::string &message) {
    return message.find("duplicate key value violates unique constraint") != std::string::npos;
}

Repo::Repo(PGconn *conn)
        : mConnection(conn) {}

void
Repo::registerLink(const std::string &address) {
    std::string sql = ""
                      "INSERT INTO links (address, meta_title, meta_description, body_title, body_keywords) "
                      "VALUES ('" + address + "', '', '', '', '{}')";

    PGresult *res = PQexec(mConnection, sql.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        char *message = PQerrorMessage(mConnection);
        if (isDuplicateError(message)) {
            throw DuplicateKeyException(message);
        } else {
            throw std::runtime_error(message);
        }
    }
    PQclear(res);
}

void
Repo::storeLink(
        const std::string &address,
        const std::string &metaTitle,
        const std::string &metaDescr,
        const std::string &bodyTitle,
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
                      "   $2, "
                      "   $3, "
                      "   $4, "
                      "   $5, "
                      "   NOW() "
                      ")"
                      "ON CONFLICT (address) "
                      "DO UPDATE SET "
                      "  meta_title       = $2, "
                      "  meta_description = $3, "
                      "  body_title       = $4, "
                      "  body_keywords    = $5, "
                      "  checked_at       = now() ";

    const int paramsSize = 5;
    const char *paramValues[paramsSize] = {
            address.c_str(),
            metaTitle.c_str(),
            metaDescr.c_str(),
            bodyTitle.c_str(),
            "{}"
    };
    PGresult *res = PQexecParams(
            mConnection, sql.c_str(),
            paramsSize, nullptr, paramValues, nullptr, nullptr, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "failed: %s", PQerrorMessage(mConnection));
    }
    PQclear(res);
}

std::vector<std::string> Repo::loadUncheckedLinks(unsigned int limit) {
    std::string sql = "SELECT address FROM links WHERE checked_at IS NULL LIMIT " + std::to_string(limit);
    PGresult *res = PQexec(mConnection, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "failed: %s", PQerrorMessage(mConnection));
    }

    int rows = PQntuples(res);
    PQgetvalue(res, 0, 0);

    std::vector<std::string> links(rows);
    for (int i = 0; i < rows; i++) {
        links[i] = (PQgetvalue(res, i, 0));
    }

    PQclear(res);
    return links;
}

std::vector<std::string> Repo::loadCheckedLinks(unsigned int limit) {
    std::string sql = "SELECT address FROM links ORDER BY checked_at ASC LIMIT " + std::to_string(limit);
    PGresult *res = PQexec(mConnection, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "failed: %s", PQerrorMessage(mConnection));
    }

    int rows = PQntuples(res);
    PQgetvalue(res, 0, 0);

    std::vector<std::string> links(rows);
    for (int i = 0; i < rows; i++) {
        links[i] = (PQgetvalue(res, i, 0));
    }

    PQclear(res);
    return links;
}
