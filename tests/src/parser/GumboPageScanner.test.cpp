//
// Created by goforbroke on 13.11.2020.
//

#include <gtest/gtest.h>

#include "../../../src/parser/GumboPageScanner.h"

TEST(GumboPageScanner_all, positive_simplest) {
    const char *content = R"(

<html>
<head>
    <title>Hello Gumbo, thank Google!</title>
    <meta name="description" content="Some short page descRipTioN!!!">
    <meta name="keywords" content="glsl, hlsl, metal, generator">

    <meta property="og:title" content="How quickly to create ..."/>
    <meta property="og:image" content="https://vaaalera.org/uploads/image.png"/>
    <meta property="og:description" content="How to create even in Calendar"/>
</head>

<body>
    <h1>Some title some title!</h1>

    <div>Ut enim ad minim veniam,
        quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat</div>

    <p>Lorem ipsum dolor sit amet,
        consectetur adipiscing elit,
        sed do eiusmod tempor incididunt ut labore et dolore magna aliqua</p>
</body>
<html>

)";
    AbstractPageScanner *scanner = new GumboPageScanner();
    scanner->load(content);

    ASSERT_EQ("Hello Gumbo, thank Google!", scanner->getMetaTitle());
    ASSERT_EQ("Some short page descRipTioN!!!", scanner->getMetaDescription());
    ASSERT_EQ(std::vector<std::string>({"glsl", "hlsl", "metal", "generator"}), scanner->getMetaKeywords());
    ASSERT_EQ("Some title some title!", scanner->getBodyTitle());

    ASSERT_EQ("How quickly to create ...", scanner->getOGTitle());
    ASSERT_EQ("https://vaaalera.org/uploads/image.png", scanner->getOGImage());
    ASSERT_EQ("How to create even in Calendar", scanner->getOGDescription());

    ASSERT_EQ(2, scanner->getBodyText().size());
}

TEST(GumboPageScanner_getCharset, positive_utf_8) {
    const char *content = R"(
<html>
<head>
    <meta charset="UTF-8">
</head>
<body>HELLO</body>
<html>

)";
    AbstractPageScanner *scanner = new GumboPageScanner();
    scanner->load(content);

    ASSERT_EQ(Charset::UTF8, scanner->getCharset());
}
TEST(GumboPageScanner_getCharset, positive_2_utf_8) {
    const char *content = R"(
<html>
<head>
    <meta charset="utf-8">
</head>
<body>HELLO</body>
<html>

)";
    AbstractPageScanner *scanner = new GumboPageScanner();
    scanner->load(content);

    ASSERT_EQ(Charset::UTF8, scanner->getCharset());
}

TEST(GumboPageScanner_getCharset, positive_windows_1251) {
    const char *content = R"(
<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=windows-1251">
</head>
<body>HELLO</body>
<html>

)";
    AbstractPageScanner *scanner = new GumboPageScanner();
    scanner->load(content);

    ASSERT_EQ(Charset::CP1251, scanner->getCharset());
}

TEST(GumboPageScanner_getCharset, positive_youtube) {
    const char *content = R"(
<html>
<head>
    <div inlined-html>
        <meta charset="UTF-8">
    </div>
</head>
<body>HELLO</body>
<html>

)";
    AbstractPageScanner *scanner = new GumboPageScanner();
    scanner->load(content);

    ASSERT_EQ(Charset::CP1251, scanner->getCharset());
}
