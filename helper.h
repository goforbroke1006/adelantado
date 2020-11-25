//
// Created by goforbroke on 25.11.2020.
//

#ifndef ADELANTADO_HELPER_H
#define ADELANTADO_HELPER_H

#include <libconfig.h++>
#include <postgresql/libpq-fe.h>
#include <stdexcept>

PGconn *
openDbConnection(libconfig::Config *config) {
    std::string dbUser = config->lookup("db_username");
    std::string dbPass = config->lookup("db_password");
    std::string dbHost = config->lookup("db_host");
    int dbPort;
    config->lookupValue("db_port", dbPort);
    std::string dbName = config->lookup("db_name");
    std::string dbConnStr =
            "postgresql://" + dbUser + ":" + dbPass
            + "@" + dbHost + ":" + std::to_string(dbPort) + "/" + dbName +
            "?connect_timeout=10";
    PGconn *conn = PQconnectdb(dbConnStr.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s",
                PQerrorMessage(conn));
        PQfinish(conn);
        throw std::runtime_error("can't open db connection");
    }

    return conn;
}

#endif //ADELANTADO_HELPER_H
