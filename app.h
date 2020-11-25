//
// Created by goforbroke on 11.11.2020.
//

#ifndef ADELANTADO_APP_H
#define ADELANTADO_APP_H

#include <vector>
#include <string>
#include <map>
#include <thread>
#include "src/HTTPClient.h"
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
    std::string domain;
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

    void pushFailed(const std::string &url) {
        std::lock_guard<std::mutex> guard(mFailedLinksMX);
        mFailedLinks.push_back(url);
    }

    const std::vector<Resource> &getMVisitedLinks() const {
        return mVisitedLinks;
    }

    const std::vector<std::string> &getMQueuedLinks() const {
        return mQueuedLinks;
    }

    const std::vector<std::string> &getMFailedLinks() const {
        return mFailedLinks;
    }

private:
    std::vector<Resource> mVisitedLinks;
    std::mutex mVisitedLinksMX;

    std::vector<std::string> mQueuedLinks;
    std::mutex mQueuedLinksMX;

    std::vector<std::string> mFailedLinks;
    std::mutex mFailedLinksMX;
};


#endif //ADELANTADO_APP_H
