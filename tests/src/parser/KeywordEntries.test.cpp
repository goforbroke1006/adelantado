//
// Created by goforbroke on 15.11.2020.
//

#include <gtest/gtest.h>

#include "../../../src/parser/KeywordEntries.h"

TEST(atLeastOneLetter, positive_simplest) {
    ASSERT_TRUE( atLeastOneLetter("h"));
    ASSERT_TRUE( atLeastOneLetter("h..."));
    ASSERT_TRUE( atLeastOneLetter("2020hello"));
    ASSERT_TRUE( atLeastOneLetter("world"));
}

TEST(atLeastOneLetter, negative) {
    ASSERT_FALSE( atLeastOneLetter(""));
    ASSERT_FALSE( atLeastOneLetter("..."));
    ASSERT_FALSE( atLeastOneLetter("!@#$%^&*()"));
    ASSERT_FALSE( atLeastOneLetter("_+{}:\"<>?"));
}

TEST(atLeastOneLetter, positive_cyrillic) {
    ASSERT_TRUE( atLeastOneLetter("п"));
    ASSERT_TRUE( atLeastOneLetter("р..."));
    ASSERT_TRUE( atLeastOneLetter("и2020"));
    ASSERT_TRUE( atLeastOneLetter(",,в.."));
    ASSERT_TRUE( atLeastOneLetter("_е"));
    ASSERT_TRUE( atLeastOneLetter(" т "));
    ASSERT_TRUE( atLeastOneLetter("ё"));
    ASSERT_TRUE( atLeastOneLetter(" ё "));
//    ASSERT_TRUE( atLeastOneLetter("Ё")); // FIXME: problem with this rune
//    ASSERT_TRUE( atLeastOneLetter(" Ё ")); // FIXME: problem with this rune
}

TEST(KeywordEntries_all, positive_simplest) {
    KeywordEntries entries;
    entries.appendPhrase("hello world");
    ASSERT_EQ(1, entries.getTop().at("hello"));
    ASSERT_EQ(1, entries.getTop().at("world"));
}

TEST(KeywordEntries_all, positive_duplicates) {
    KeywordEntries entries(3);
    entries.appendPhrase("hello world.");
    entries.appendPhrase("world cup.");
    ASSERT_EQ(1, entries.getTop().at("hello"));
    ASSERT_EQ(2, entries.getTop().at("world"));
    ASSERT_EQ(1, entries.getTop().at("cup"));
}

TEST(KeywordEntries_all, positive_skip_non_letter) {
    KeywordEntries entries(4);
    entries.appendPhrase("hello, world");
    entries.appendPhrase("world cup!");
    ASSERT_EQ(1, entries.getTop().at("hello"));
    ASSERT_EQ(2, entries.getTop().at("world"));
    ASSERT_EQ(0, entries.getTop().count("cup"));
}

TEST(KeywordEntries_all, positive_skip_non_letter_2) {
    KeywordEntries entries(3);
    entries.appendPhrase("hello,,,,#%^&*(world");
    entries.appendPhrase("/\\world<><>,.,.,.,[cup]!%^&:;");
    ASSERT_EQ(1, entries.getTop().at("hello"));
    ASSERT_EQ(2, entries.getTop().at("world"));
    ASSERT_EQ(1, entries.getTop().at("cup"));
}

TEST(KeywordEntries_all, positive_skip_quotes) {
    KeywordEntries entries(3);
    entries.appendPhrase("\"hello\"  world");
    entries.appendPhrase("world cup!");
    ASSERT_EQ(1, entries.getTop().at("hello"));
    ASSERT_EQ(2, entries.getTop().at("world"));
    ASSERT_EQ(1, entries.getTop().at("cup"));
}

TEST(KeywordEntries_all, negative_empty_lines) {
    KeywordEntries entries;
    entries.appendPhrase("");
    entries.appendPhrase("");
    ASSERT_TRUE(entries.getTop().empty());
}

TEST(KeywordEntries_all, negative_lines_without_text) {
    KeywordEntries entries;
    entries.appendPhrase("#$#$#$#$");
    entries.appendPhrase("_-_-_=+");
    ASSERT_TRUE(entries.getTop().empty());
}
