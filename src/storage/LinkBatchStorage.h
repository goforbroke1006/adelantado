//
// Created by goforbroke on 29.11.2020.
//

#ifndef ADELANTADO_LINKBATCHSTORAGE_H
#define ADELANTADO_LINKBATCHSTORAGE_H

#include <string>
#include <vector>

#include <postgresql/libpq-fe.h>

class LinkBatchStorage {
public:
    explicit LinkBatchStorage(PGconn *conn);

    void registerLink(const std::string &address);

    void flush();

private:
    PGconn *mConnection;
    std::vector<std::string> mQueries;
};


#endif //ADELANTADO_LINKBATCHSTORAGE_H
