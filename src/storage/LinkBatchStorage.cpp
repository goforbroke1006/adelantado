//
// Created by goforbroke on 29.11.2020.
//

#include "LinkBatchStorage.h"

#include <stdexcept>
#include "common.h"

LinkBatchStorage::LinkBatchStorage(PGconn *conn)
        : mConnection(conn) {}

void LinkBatchStorage::registerLink(const std::string &address) {
    std::string sql = ""
                      "INSERT INTO links (address) "
                      "VALUES ('" + address + "') ON CONFLICT DO NOTHING";
    mQueries.push_back(sql);
}

void LinkBatchStorage::flush() {
    std::string sql;
    for (const auto &query : mQueries) {
        sql.append(query).append("; ");
    }

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
