//
// Created by goforbroke on 13.11.2020.
//

#include <goxx-std-strings.h>
#include "html-filter.h"

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

std::vector<std::string> getLinkAddresses(const std::string &html) {
    GumboOutput *output = gumbo_parse(html.c_str());

    std::vector<std::string> contentLinks;
    recursiveFilterLinks(output->root, contentLinks);

    gumbo_destroy_output(&kGumboDefaultOptions, output);

    return contentLinks;
}

std::vector<std::string>
normalizeHrefsToLinks(
        const std::vector<std::string> &links,
        const std::string &protocol,
        const std::string &domain
) {
    std::vector<std::string> result(links.size());

    std::string prefix = protocol + "://" + domain;

    for (auto &cl : links) {
        std::string link = cl;

        // absolute link for current domain
        if (link[0] == '/' && link[1] != '/') {
            link = prefix + link;
        }

        // absolute link for current protocol+domain
        if (link[0] == '/' && link[1] == '/') {
            link = protocol + ":" + link;
        }

        // TODO: remove fragment from link

        // TODO: remove utm params

        link = goxx_std::strings::replace(link, "\n", "");
        link = goxx_std::strings::replace(link, " ", "");
        link = goxx_std::strings::trimSpace(link);

        if (link.rfind("http", 0) != 0) { // FIXME: workaround
            continue;
        }
        if (link.find("'") != std::string::npos) { // FIXME: workaround
            continue;
        }
        if (link.find("/cgi-bin/") != std::string::npos) { // FIXME: workaround
            continue;
        }
        if (
                link.find(" ") != std::string::npos
                || link.find("<") != std::string::npos
                || link.find(">") != std::string::npos
                ) {
            continue;
        }

        result.push_back(link);
    }

    return result;
}
