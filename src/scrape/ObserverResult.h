//
// Created by goforbroke on 28.11.2020.
//

#ifndef ADELANTADO_OBSERVERRESULT_H
#define ADELANTADO_OBSERVERRESULT_H

#include <string>
#include <vector>
#include <mutex>
#include "../domain.h"

class ObserverResult {
public:
    void pushVisited(const Resource &resource);

    void appendLinks(const std::vector<std::string> &links);

    void pushFailed(const std::string &url);

    const std::vector<Resource> &getMVisitedLinks() const;

    const std::vector<std::string> &getMQueuedLinks() const;

    const std::vector<std::string> &getMFailedLinks() const;

private:
    std::vector<Resource> mVisitedLinks;
    std::mutex mVisitedLinksMX;

    std::vector<std::string> mQueuedLinks;
    std::mutex mQueuedLinksMX;

    std::vector<std::string> mFailedLinks;
    std::mutex mFailedLinksMX;
};


#endif //ADELANTADO_OBSERVERRESULT_H
