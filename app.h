//
// Created by goforbroke on 11.11.2020.
//

#ifndef ADELANTADO_APP_H
#define ADELANTADO_APP_H

#include <vector>
#include <string>
#include <map>
#include <thread>
#include "HTTPClient.h"
#include "url.h"
#include "html-filter.h"
#include <mutex>

/**
 * Allow iterate over chunks of vector
 * @tparam ItemType
 */
template<typename ItemType>
class VectorBulkSplitter {
public:
    explicit VectorBulkSplitter(
            const std::vector<ItemType> &source,
            size_t chunkSize
    )
            : mSource(source),
              mChunkSize(chunkSize) {
        mNextElement = source.begin();
    }

    std::vector<ItemType> getNext() {
        // reach end of source
        if (mNextElement == mSource.end())
            return {};

        // return rest of source
        if ((mSource.end() - mNextElement) < mChunkSize) {
            const std::vector<ItemType> &chunk = std::vector<ItemType>(mNextElement, mSource.end());
            mNextElement = mSource.end();
            return chunk;
        }

        // take usual chunk
        const ConstItemIterator &right = mNextElement + mChunkSize;
        const std::vector<ItemType> &chunk = std::vector<ItemType>(mNextElement, right);
        mNextElement = right;
        return chunk;
    }

private:
    const std::vector<ItemType> &mSource;
    const size_t mChunkSize;
    typedef typename std::vector<ItemType>::const_iterator ConstItemIterator;
    ConstItemIterator mNextElement;
};

struct Resource {
    std::string address;
    std::string metaTitle;
    std::string metaDescr;
    std::string bodyTitle;
    std::map<std::string, unsigned int> bodyKeywords;
};

class ObserverResult {
public:
    void pushVisited(const Resource &resource) {
        std::lock_guard<std::mutex> guard(mVisitedLinksMX);
        mVisitedLinks.push_back(resource);
    }

    void appendLinks(const std::vector<std::string> &links) {
        std::lock_guard<std::mutex> guard(mQueuedLinksMX);
        mQueuedLinks.insert(
                std::end(mQueuedLinks),
                std::begin(links), std::end(links)
        );
    }

    const std::vector<Resource> &getMVisitedLinks() const {
        return mVisitedLinks;
    }

    const std::vector<std::string> &getMQueuedLinks() const {
        return mQueuedLinks;
    }

private:
    std::vector<Resource> mVisitedLinks;
    std::mutex mVisitedLinksMX;
    std::vector<std::string> mQueuedLinks;
    std::mutex mQueuedLinksMX;
};

ObserverResult *
MultiThreadLinksObserver(
        const std::vector<std::string> &links,
        size_t multi
) {
    VectorBulkSplitter splitter(links, multi);
    std::vector<std::thread> threads;

    auto func = [](const std::vector<std::string> &linksChunk, ObserverResult *result) {
        for (const auto &link : linksChunk) {
            const HTTPResponse &response = HTTPClient::load(link);
            URL url;
            try {
                url = parseURL(link);
            } catch (std::runtime_error &err) {
                // TODO: log errors
                continue;
            }

            std::string metaTitle = getPageTitle(response.content);
            // TODO: retrieve meta description
            // TODO: retrieve body title
            // TODO: retrieve body keywords
            result->pushVisited(Resource{link, metaTitle});

            const std::vector<std::string> &contentLinks = getLinkAddresses(response.content);
            normalizeHrefsToLinks(contentLinks, url.protocol, url.host);
            if (!contentLinks.empty()) {
                result->appendLinks(contentLinks);
                std::cout << link << " : ok" << std::endl;
            } else {
                std::cout << link << " : no links" << std::endl;
            }

        }
    };

    ObserverResult *observerResult = new ObserverResult;

    for (size_t thi = 0; thi < multi; ++thi) {
        std::vector<std::string> chunk = splitter.getNext();
        std::thread thread(func, chunk, observerResult);
        threads.emplace_back(std::move(thread));
    }
    for (auto &th : threads) {
        th.join();
    }

    return observerResult;
}

#endif //ADELANTADO_APP_H
