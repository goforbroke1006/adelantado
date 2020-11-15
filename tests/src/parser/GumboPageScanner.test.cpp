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
<head>

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
    ASSERT_EQ("Some title some title!", scanner->getBodyTitle());

    ASSERT_EQ(2, scanner->getBodyText().size());
}
