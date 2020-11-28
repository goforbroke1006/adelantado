//
// Created by goforbroke on 15.11.2020.
//

#include "LinkStorage.h"

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <goxx-std-strings.h>

#include "common.h"

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

void LinkStorage::storeLink(const Resource &resource) {
    std::string sql = ""
                      "INSERT INTO links ("
                      "   address, "
                      "   domain, "
                      "   meta_title, meta_description, meta_keywords, "
                      "   body_title, body_keywords, "
                      "   checked_at, "
                      "   og_title, og_image, og_description, og_site_name, "
                      "   last_status_code, page_content_size, charset "
                      ") VALUES ("
                      "   $1, "
                      "   $2, "
                      "   $3, $4, $5, "
                      "   $6, $7, "
                      "   NOW(), "
                      "   $8, $9, $10, $11, "
                      "   $12, $13, $14 "
                      ")"
                      "ON CONFLICT (address) "
                      "DO UPDATE SET "
                      "  domain           = $2, "
                      "  meta_title       = $3, "
                      "  meta_description = $4, "
                      "  meta_keywords    = $5, "
                      "  body_title       = $6, "
                      "  body_keywords    = $7, "
                      "  checked_at       = NOW(), "
                      "  og_title = $8, og_image = $9, og_description = $10, og_site_name = $11, "
                      "  last_status_code  = $12, "
                      "  page_content_size = $13, "
                      "  charset           = $14;";

    std::string metaKeywordsStr = goxx_std::strings::join(resource.metaKeywords, ", ");

    std::string keywordsStr;
    if (!resource.bodyKeywords.empty()) {
        nlohmann::json keywordsJson;
        for (const auto &kv : resource.bodyKeywords) {
            keywordsJson[kv.first] = kv.second;
        }
        keywordsStr = keywordsJson.dump();
    } else {
        keywordsStr = "{}";
    }

    const int paramsSize = 14;
    const char *paramValues[paramsSize] = {
            resource.address.c_str(),
            resource.domain.c_str(),
            resource.metaTitle.c_str(), resource.metaDescr.c_str(), metaKeywordsStr.c_str(),
            resource.bodyTitle.c_str(),
            keywordsStr.c_str(),
            //
            resource.ogTitle.c_str(),
            resource.ogImage.c_str(),
            resource.ogDescription.c_str(),
            resource.ogSiteName.c_str(),
            //
            std::to_string(resource.statusCode).c_str(),
            std::to_string(resource.pageContentSize).c_str(),
            resource.charset.c_str(),
    };
    PGresult *res = PQexecParams(
            mConnection, sql.c_str(),
            paramsSize, nullptr, paramValues, nullptr, nullptr, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        throw std::runtime_error(std::string() + "failed: " + PQerrorMessage(mConnection));
    }
    PQclear(res);
}

std::vector<std::string>
LinkStorage::loadUncheckedLinks(unsigned int limit, const std::vector<std::string> &priorityDomains) {
    std::string byDomainClause;
    if (!priorityDomains.empty()) {
        std::string enumStr;
        for (const auto &d : priorityDomains) {
            enumStr = enumStr.append("'").append(d).append("'").append(", ");
        }
        enumStr = enumStr.substr(0, enumStr.length() - 2);

        byDomainClause = " AND domain IN (" + enumStr + ") ";
    }

    std::string sql = std::string()
                      + "SELECT address "
                      + "FROM links "
                      + "WHERE "
                      + "  checked_at IS NULL " + byDomainClause + " "
                      + "  AND (next_check_at <= NOW() OR next_check_at IS NULL) "
                      + "LIMIT " + std::to_string(limit);
    PGresult *res = PQexec(mConnection, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        throw std::runtime_error(std::string() + "failed: " + PQerrorMessage(mConnection));
    }

    int rows = PQntuples(res);
    if (rows == 0) {
        return {};
    }
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
    std::string sql = std::string()
                      + "SELECT address "
                      + "FROM links "
                      + "WHERE "
                      + "  checked_at IS NULL "
                      + "  AND (next_check_at <= NOW() OR next_check_at IS NULL) "
                      + "LIMIT " + std::to_string(limit);
    PGresult *res = PQexec(mConnection, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        throw std::runtime_error(std::string() + "failed: " + PQerrorMessage(mConnection));
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
    std::string sql = std::string()
                      + "SELECT address "
                      + "FROM links "
                      + "WHERE "
                      + "  (next_check_at <= NOW() OR next_check_at IS NULL) "
                      + "  AND checked_at <= NOW() - INTERVAL '7 DAY'"
                      + "ORDER BY checked_at ASC "
                      + "LIMIT " + std::to_string(limit);
    PGresult *res = PQexec(mConnection, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        throw std::runtime_error(std::string() + "failed: " + PQerrorMessage(mConnection));
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

void LinkStorage::postpone(const std::string &link) {
    std::string sql = ""
                      "UPDATE links "
                      "SET next_check_at = NOW() + INTERVAL '24 hours' "
                      "WHERE address = '" + link + "';";

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
