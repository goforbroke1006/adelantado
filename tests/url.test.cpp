//
// Created by goforbroke on 10.11.2020.
//

#include <gtest/gtest.h>

#include "../url.h"

TEST(url_parseURL_all, positive1) {
    URL u = parseURL("https://www.cyberforum.ru/cpp-beginners/thread349754.html");

    ASSERT_EQ("https", u.protocol);
    ASSERT_EQ("www.cyberforum.ru", u.host);
    ASSERT_EQ(0, u.port);
    ASSERT_EQ("/cpp-beginners/thread349754.html", u.path);
    ASSERT_EQ("", u.query);
}

TEST(url_parseURL_all, positive2) {
    URL u = parseURL("https://www.cyberforum.ru/cpp-beginners/thread349754.html?hello=world");

    ASSERT_EQ("https", u.protocol);
    ASSERT_EQ("www.cyberforum.ru", u.host);
    ASSERT_EQ(0, u.port);
    ASSERT_EQ("/cpp-beginners/thread349754.html", u.path);
    ASSERT_EQ("?hello=world", u.query);
}

TEST(url_parseURL_host, positive1) {
    ASSERT_EQ("google.com", parseURL("https://google.com/q/").host);
    ASSERT_EQ("en.wikipedia.org", parseURL("https://en.wikipedia.org/wiki/Main_Page").host);
    ASSERT_EQ("2stocks.ru", parseURL("https://2stocks.ru/2.0/top100").host);
    ASSERT_EQ("TEST-test.d3.d2.d1", parseURL("http://TEST-test.d3.d2.d1/").host);
}

TEST(url_parseURL_port, positive1) {
    ASSERT_EQ(2345, parseURL("https://my-infra.local:2345/").port);
    ASSERT_EQ(32456, parseURL("https://my-infra.local:32456/wiki/Main_Page").port);
}

TEST(url_parseURL_path, positive1) {
    ASSERT_EQ("/q/", parseURL("https://google.com:80/q/").path);
    ASSERT_EQ("/wiki/Main_Page", parseURL("https://en.wikipedia.org:80/wiki/Main_Page").path);
    ASSERT_EQ("/2.0/top100", parseURL("https://2stocks.ru:80/2.0/top100").path);
    ASSERT_EQ("/", parseURL("http://TEST-test.d3.d2.d1:80/").path);
}
