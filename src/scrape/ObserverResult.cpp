//
// Created by goforbroke on 28.11.2020.
//

#include "ObserverResult.h"

void ObserverResult::pushVisited(const Resource &resource) {
    std::lock_guard<std::mutex> guard(mVisitedLinksMX);
    mVisitedLinks.push_back(resource);
}

void ObserverResult::appendLinks(const std::vector<std::string> &links) {
    std::lock_guard<std::mutex> guard(mQueuedLinksMX);
    mQueuedLinks.insert(
            std::end(mQueuedLinks),
            std::begin(links), std::end(links)
    );
}

void ObserverResult::pushFailed(const std::string &url) {
    std::lock_guard<std::mutex> guard(mFailedLinksMX);
    mFailedLinks.push_back(url);
}

const std::vector<Resource> &ObserverResult::getMVisitedLinks() const {
    return mVisitedLinks;
}

const std::vector<std::string> &ObserverResult::getMQueuedLinks() const {
    return mQueuedLinks;
}

const std::vector<std::string> &ObserverResult::getMFailedLinks() const {
    return mFailedLinks;
}
