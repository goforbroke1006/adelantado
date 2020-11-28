//
// Created by goforbroke on 13.11.2020.
//

#include "GumboPageScanner.h"
#include "KeywordEntries.h"

#include <cassert>
#include <goxx-std-strings.h>

GumboPageScanner::GumboPageScanner()
        : mOutput(nullptr), mContent("") {}

GumboPageScanner::~GumboPageScanner() {
    if (nullptr != mOutput) {
        gumbo_destroy_output(&kGumboDefaultOptions, mOutput);
        mOutput = nullptr;
    }
}

void GumboPageScanner::load(const std::string &content) {
    if (content.empty()) {
        throw std::runtime_error("wrong html format");
    }
    if (nullptr != mOutput) {
        gumbo_destroy_output(&kGumboDefaultOptions, mOutput);
        mOutput = nullptr;
    }

    mOutput = gumbo_parse(content.c_str());
    if (nullptr == mOutput) {
        throw std::runtime_error("wrong html format");
    }

    mContent = content;
}

std::string GumboPageScanner::getMetaTitle() {
    if (nullptr == mOutput || mOutput->root->type != GUMBO_NODE_ELEMENT) {
        return "";
    }

    const GumboVector *root_children = &mOutput->root->v.element.children;
    GumboNode *head = nullptr;
    for (int i = 0; i < root_children->length; ++i) {
        auto *child = (GumboNode *) root_children->data[i];
        if (child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_HEAD) {
            head = child;
            break;
        }
    }
    if (nullptr == head)
        return "";

    std::string resultTitle;

    GumboVector *head_children = &head->v.element.children;
    for (int i = 0; i < head_children->length; ++i) {
        auto *child = (GumboNode *) head_children->data[i];
        if (child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_TITLE) {
            if (child->v.element.children.length != 1) {
                resultTitle = "";
                break;
            }
            auto *title_text = (GumboNode *) child->v.element.children.data[0];
            assert(title_text->type == GUMBO_NODE_TEXT || title_text->type == GUMBO_NODE_WHITESPACE);
            resultTitle = title_text->v.text.text;
            break;
        }
    }

    return resultTitle;
}

std::string GumboPageScanner::getMetaDescription() {
    const std::vector<GumboNode *> &nodes = getMetaNodes("name", "description");

    std::string metaDescription;
    for (const auto *child : nodes) {
        if (nullptr != child && child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_META) {
            GumboAttribute *contentAttr = gumbo_get_attribute(&child->v.element.attributes, "content");
            if (nullptr == contentAttr)
                continue;

            metaDescription += contentAttr->value;
            metaDescription += " ";
        }
    }

    metaDescription = goxx_std::strings::trimSpace(metaDescription);
    return metaDescription;
}

std::vector<std::string> GumboPageScanner::getMetaKeywords() {
    const std::vector<GumboNode *> &nodes = getMetaNodes("name", "keywords");

    for (const auto *child : nodes) {
        GumboAttribute *contentAttr = gumbo_get_attribute(&child->v.element.attributes, "content");
        if (nullptr == contentAttr)
            continue;

        auto keywords = goxx_std::strings::split(contentAttr->value, ",");
        for (auto &kw : keywords) {
            kw = goxx_std::strings::trimSpace(kw);
        }
        return keywords;
    }

    return {};
}

std::string GumboPageScanner::getBodyTitle() {
    std::string resultTitle;

    assert(mOutput->root->type == GUMBO_NODE_ELEMENT);
    assert(mOutput->root->v.element.children.length >= 1);

    const GumboVector *root_children = &mOutput->root->v.element.children;
    GumboNode *body = nullptr;
    for (int i = 0; i < root_children->length; ++i) {
        auto *child = (GumboNode *) root_children->data[i];
        if (child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_BODY) {
            body = child;
            break;
        }
    }
    if (nullptr == body) {
        return "";
    }

    getBodyTitleRecursively(body, resultTitle);

    resultTitle = goxx_std::strings::trimSpace(resultTitle);
    return resultTitle;
}

std::vector<std::string> GumboPageScanner::getBodyText() {
    const GumboVector *root_children = &mOutput->root->v.element.children;
    GumboNode *body = nullptr;
    for (int i = 0; i < root_children->length; ++i) {
        auto *child = (GumboNode *) root_children->data[i];
        if (child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_BODY) {
            body = child;
            break;
        }
    }
    if (nullptr == body) {
        return {};
    }

    std::vector<std::string> lines;
    getTextLinesRecursively(body, lines);

    return lines;
}

void GumboPageScanner::getBodyTitleRecursively(GumboNode *node, std::string &result) {
    if (node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == GUMBO_TAG_H1) {
        auto *title_text = (GumboNode *) node->v.element.children.data[0];
        if (nullptr == title_text)
            return;
        if (title_text->type != GUMBO_NODE_TEXT && title_text->type != GUMBO_NODE_WHITESPACE)
            return;

        if (!result.empty())
            result.append(" ");
        result.append(title_text->v.text.text);
        return;
    }

    const GumboVector *children = &node->v.element.children;
    for (int i = 0; i < children->length; ++i) {
        auto *child = (GumboNode *) children->data[i];
        if (
                nullptr != child
                && child->type == GUMBO_NODE_ELEMENT
                && node->v.element.children.length >= 1
                ) {
            getBodyTitleRecursively(child, result);
        }
    }
}

void GumboPageScanner::getTextLinesRecursively(GumboNode *node, std::vector<std::string> &result) {
    if (node->type == GUMBO_NODE_ELEMENT
        && (
                node->v.element.tag == GUMBO_TAG_DIV
                || node->v.element.tag == GUMBO_TAG_P
                || node->v.element.tag == GUMBO_TAG_SPAN
        )
            ) {

        auto *title_text = (GumboNode *) node->v.element.children.data[0];
        if (nullptr != title_text && title_text->type == GUMBO_NODE_TEXT) {
            const char *line = title_text->v.text.text;
            result.emplace_back(line);
        }
    }

    const GumboVector *children = &node->v.element.children;
    for (int i = 0; i < children->length; ++i) {
        auto *child = (GumboNode *) children->data[i];
        if (child->type == GUMBO_NODE_ELEMENT && node->v.element.children.length >= 1) {
            getTextLinesRecursively(child, result);
        }
    }
}

std::vector<GumboNode *> GumboPageScanner::getMetaNodes(const std::string &attr, const std::string &name) {
    if (mOutput->root->type != GUMBO_NODE_ELEMENT)
        return {};
    if (mOutput->root->v.element.children.length < 2)
        return {};

    const GumboVector *root_children = &mOutput->root->v.element.children;
    GumboNode *head = nullptr;
    for (int i = 0; i < root_children->length; ++i) {
        auto *child = (GumboNode *) root_children->data[i];
        if (child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_HEAD) {
            head = child;
            break;
        }
    }
    if (nullptr == head)
        return {};

    std::vector<GumboNode *> result;

    GumboVector *head_children = &head->v.element.children;
    for (int i = 0; i < head_children->length; ++i) {
        auto *child = (GumboNode *) head_children->data[i];

        if (nullptr != child && child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_META) {

            GumboAttribute *nameAttr = gumbo_get_attribute(&child->v.element.attributes, attr.c_str());
            if (nullptr == nameAttr)
                continue;

            if (name != nameAttr->value)
                continue;

            result.push_back(child);
        }
    }

    return result;
}

std::string GumboPageScanner::getMetaNodesContent(const std::string &attr, const std::string &name) {
    const std::vector<GumboNode *> &nodes = getMetaNodes(attr, name);
    std::string allNodesContent;
    for (const auto &ogtNode : nodes) {
        GumboAttribute *contentAttr = gumbo_get_attribute(&ogtNode->v.element.attributes, "content");
        if (nullptr == contentAttr)
            continue;

        allNodesContent += contentAttr->value;
        allNodesContent += " ";
    }

    allNodesContent = goxx_std::strings::trimSpace(allNodesContent);
    return allNodesContent;
}

std::string GumboPageScanner::getOGTitle() {
    return getMetaNodesContent("property", "og:title");
}

std::string GumboPageScanner::getOGImage() {
    return getMetaNodesContent("property", "og:image");
}

std::string GumboPageScanner::getOGDescription() {
    return getMetaNodesContent("property", "og:description");
}

std::string GumboPageScanner::getOGSiteName() {
    return getMetaNodesContent("property", "og:og:site_name");
}
