//
// Created by goforbroke on 10.11.2020.
//

#ifndef ADELANTADO_HTML_FILTER_H
#define ADELANTADO_HTML_FILTER_H

#include <string>
#include <vector>
#include <gumbo.h>
#include <cassert>

std::string getPageTitle(const std::string &html) {
    std::string resultTitle;

    GumboOutput *output = gumbo_parse(html.c_str());

    assert(output->root->type == GUMBO_NODE_ELEMENT);
    assert(output->root->v.element.children.length >= 2);

    const GumboVector *root_children = &output->root->v.element.children;
    GumboNode *head = nullptr;
    for (int i = 0; i < root_children->length; ++i) {
        GumboNode *child = (GumboNode *) root_children->data[i];
        if (child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_HEAD) {
            head = child;
            break;
        }
    }
    assert(head != nullptr);

    GumboVector *head_children = &head->v.element.children;
    for (int i = 0; i < head_children->length; ++i) {
        GumboNode *child = (GumboNode *) head_children->data[i];
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

    gumbo_destroy_output(&kGumboDefaultOptions, output);

    return resultTitle;
}

void recursiveFilterLinks(GumboNode *node, std::vector<std::string> &result);

std::vector<std::string> getLinkAddresses(const std::string &html) {
    GumboOutput *output = gumbo_parse(html.c_str());

    std::vector<std::string> contentLinks;
    recursiveFilterLinks(output->root, contentLinks);

    gumbo_destroy_output(&kGumboDefaultOptions, output);

    return contentLinks;
}

void recursiveFilterLinks(GumboNode *node, std::vector<std::string> &result) {
    const GumboVector *children = &node->v.element.children;
    if (children->length == 0) {
        return;
    }

    for (int i = 0; i < children->length; ++i) {
        auto *child = (GumboNode *) children->data[i];
        if (child->type != GUMBO_NODE_ELEMENT) {
            continue;
        }

        if (child->v.element.tag != GUMBO_TAG_A) {
            recursiveFilterLinks(child, result);
            continue;
        }

        GumboAttribute *href = gumbo_get_attribute(&child->v.element.attributes, "href");
        if (nullptr == href) {
            continue;
        }
        std::string link = href->value;

        if (link.empty()) continue;
        if (link.length() == 1 && link[0] == '#') continue;

        result.emplace_back(link);
    }
}

void
normalizeHrefsToLinks(
        std::vector<std::string> links,
        const std::string &protocol,
        const std::string &domain
) {
    std::string prefix = protocol + "://" + domain;

    for (size_t i = 0; i < links.size(); ++i) {
#ifdef DEBUG
        std::string before = links[i];
#endif

        // absolute link for current domain
        if (links[i][0] == '/' && links[i][1] != '/') {
            links[i] = prefix + links[i];
        }

        // absolute link for current protocol+domain
        if (links[i][0] == '/' && links[i][1] == '/') {
            links[i] = protocol + ":" + links[i];
        }

#ifdef DEBUG
        if (before != links[i]) {
            std::cout << "    " << before << " => " << links[i] << std::endl;
        }
#endif

    }

}

#endif //ADELANTADO_HTML_FILTER_H
