//
// Created by goforbroke on 29.11.2020.
//

#include <gtest/gtest.h>

#include "../../../src/scrape/utils.h"

TEST(shortify_string, positive_1) {
    std::string randStr50Chars = "cezyialsxicjcqvtiokdqcqrumaawkqcfffuwyekihplqehmug";

    ASSERT_EQ(randStr50Chars.size(), 50);
    shortify(randStr50Chars, 48);
    ASSERT_EQ(randStr50Chars.size(), 48);
    ASSERT_EQ(randStr50Chars.length(), 48);
}

TEST(shortify_string, positive_2) {
    std::string randStr25Cyrillic = "йцукенгшщзхъфывапролджэяч";

    ASSERT_EQ(randStr25Cyrillic.size(), 50);
    shortify(randStr25Cyrillic, 48);
    ASSERT_EQ(randStr25Cyrillic.size(), 48);
    ASSERT_EQ(randStr25Cyrillic.length(), 48);
}
