//
// Created by goforbroke on 14.11.2020.
//

#ifndef ADELANTADO_src_parser_KEYWORD_ENTRIES_H
#define ADELANTADO_src_parser_KEYWORD_ENTRIES_H

#include <string>
#include <vector>
#include <map>
#include <set>

bool atLeastOneLetter(const std::string &word);

class KeywordEntries {
public:
    explicit KeywordEntries(size_t minLength = 4, const std::vector<std::string> &exclusions = {});

    void appendPhrase(std::string phrase);

    [[nodiscard]] std::map<std::string, unsigned int> getTop(size_t limit = 20) const;

    void clear();

private:
    std::map<std::string, unsigned int> mEntries;
    size_t mMinLength;
    std::set<std::string> mExclusions;
};


#endif //ADELANTADO_src_parser_KEYWORD_ENTRIES_H
