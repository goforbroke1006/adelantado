//
// Created by goforbroke on 15.11.2020.
//

#include "LinkStorage.h"

#include "common.h"
#include "../../modules/nlohmann-json/json.hpp"

LinkStorage::LinkStorage(PGconn *conn)
        : mConnection(conn) {}

void LinkStorage::registerLink(const std::string &address) {
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
LinkStorage::storeLink(
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
                      "  checked_at       = NOW() ";

    std::string keywordsStr;
    if (!keywords.empty()) {
        nlohmann::json keywordsJson;
        for (const auto &kv : keywords) {
            keywordsJson[kv.first] = kv.second;
        }
        keywordsStr = keywordsJson.dump();
    } else {
        keywordsStr = "{}";
    }

    const int paramsSize = 5;
    const char *paramValues[paramsSize] = {
            address.c_str(),
            metaTitle.c_str(),
            metaDescr.c_str(),
            bodyTitle.c_str(),
            keywordsStr.c_str()
    };
    PGresult *res = PQexecParams(
            mConnection, sql.c_str(),
            paramsSize, nullptr, paramValues, nullptr, nullptr, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "failed: %s", PQerrorMessage(mConnection));
    }
    PQclear(res);
}

std::vector<std::string>
LinkStorage::loadUncheckedLinks(unsigned int limit, const std::vector<std::string> &priorityDomains) {
    std::string byDomainClause;
    if (!priorityDomains.empty()) {
        std::string enumStr;
        for (const auto d : priorityDomains) {
            enumStr = enumStr.append("'").append(d).append("'").append(", ");
        }
        enumStr = enumStr.substr(0, enumStr.length() - 2);

        byDomainClause = " AND domain IN (" + enumStr + ") ";
    }

    std::string sql = std::string() +
                      "SELECT address "
                      + "FROM links WHERE checked_at IS NULL " + byDomainClause + " "
                      + "LIMIT " + std::to_string(limit);
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

std::vector<std::string>
LinkStorage::loadUncheckedLinks(unsigned int limit) {
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

std::vector<std::string>
LinkStorage::loadCheckedLinks(unsigned int limit) {
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