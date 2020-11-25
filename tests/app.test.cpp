//
// Created by goforbroke on 11.11.2020.
//

#include <gtest/gtest.h>

#include "../src/app.h"

TEST(VectorBulkSplitter_getNext, positive_round_size_without_remainder) {
    std::vector<int> s = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    VectorBulkSplitter<int> vbs(s, 3);

    ASSERT_EQ(std::vector<int>({1, 2, 3}), vbs.getNext());
    ASSERT_EQ(std::vector<int>({4, 5, 6}), vbs.getNext());
    ASSERT_EQ(std::vector<int>({7, 8, 9}), vbs.getNext());
    ASSERT_EQ(std::vector<int>({}), vbs.getNext());
    ASSERT_EQ(std::vector<int>({}), vbs.getNext());
}

TEST(VectorBulkSplitter_getNext, positive_with_remainder) {
    std::vector<int> s = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    VectorBulkSplitter<int> vbs(s, 4);

    ASSERT_EQ(std::vector<int>({1, 2, 3, 4}), vbs.getNext());
    ASSERT_EQ(std::vector<int>({5, 6, 7, 8}), vbs.getNext());
    ASSERT_EQ(std::vector<int>({9}), vbs.getNext());
    ASSERT_EQ(std::vector<int>({}), vbs.getNext());
    ASSERT_EQ(std::vector<int>({}), vbs.getNext());
}
