//
// Created by goforbroke on 10.11.2020.
//

#ifndef ADELANTADO_HTML_FILTER_H
#define ADELANTADO_HTML_FILTER_H

#include <string>
#include <vector>
#include <gumbo.h>
#include <cassert>

void recursiveFilterLinks(GumboNode *node, std::vector<std::string> &result);

std::vector<std::string> getLinkAddresses(const std::string &html);

void
normalizeHrefsToLinks(
        std::vector<std::string> links,
        const std::string &protocol,
        const std::string &domain
);

#endif //ADELANTADO_HTML_FILTER_H
