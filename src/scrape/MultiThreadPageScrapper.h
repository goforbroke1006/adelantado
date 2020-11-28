//
// Created by goforbroke on 28.11.2020.
//

#ifndef ADELANTADO_MULTITHREADPAGESCRAPPER_H
#define ADELANTADO_MULTITHREADPAGESCRAPPER_H

#include <string>
#include <vector>


#include "../domain.h"
#include "ObserverResult.h"


class MultiThreadPageScrapper {
public:
    MultiThreadPageScrapper(
            const std::vector<std::string> &links,
            size_t multi
    );

    ObserverResult *scrape() const;

private:
    const std::vector<std::string> &links;
    const size_t multi;

    static void shortify(std::string &target, size_t len);

    static void shortify(std::vector<std::string> &target, size_t len);
};


#endif //ADELANTADO_MULTITHREADPAGESCRAPPER_H
