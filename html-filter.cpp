//
// Created by goforbroke on 13.11.2020.
//

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

void normalizeHrefsToLinks(std::vector<std::string> links, const std::string &protocol, const std::string &domain) {
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

        // TODO: remove fragment from link

        // TODO: remove utm params

#ifdef DEBUG
        if (before != links[i]) {
            std::cout << "    " << before << " => " << links[i] << std::endl;
        }
#endif

    }

}
