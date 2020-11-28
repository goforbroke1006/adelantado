//
// Created by goforbroke on 28.11.2020.
//

#ifndef ADELANTADO_VECTORBULKSPLITTER_H
#define ADELANTADO_VECTORBULKSPLITTER_H

#include <vector>

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


#endif //ADELANTADO_VECTORBULKSPLITTER_H
