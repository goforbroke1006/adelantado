//
// Created by goforbroke on 14.11.2020.
//

#include "KeywordEntries.h"

#include <regex>
#include <iconv.h>

bool atLeastOneLetter(const std::string &word) {
    if (word.empty())
        return false;

    for (const auto &letter : word) {
        if (
                ('a' <= letter && letter <= 'z')
                || ('A' <= letter && letter <= 'Z')
                )
            return true;
    }

    std::string intersection;
    std::string cyrillicAbc = "абвгдеёжзийклмнопрстуфхцчшщъыьэюяАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
    for (size_t ui = 0; ui < cyrillicAbc.size() / 2; ui += 2) {
        const std::string &singleUnicodeLetter = cyrillicAbc.substr(ui, 2);
        if (word.find(singleUnicodeLetter) != std::string::npos) {
            return true;
        }
    }

    return false;
}

KeywordEntries::KeywordEntries(size_t minLength, const std::vector<std::string> &exclusions)
        : mMinLength(minLength) {
    for (const auto &w : exclusions) {
        mExclusions.insert(w);
    }
}

void KeywordEntries::appendPhrase(std::string phrase) {
    if (phrase.empty())
        return;

    std::regex punctuationMarksRE(R"(!|@|#|\$|%|\^|&|\*|\(|\)|_|=|\+|\[|\]|"|:|;|\\|/|,|<|\.|>)");
    phrase = std::regex_replace(phrase, punctuationMarksRE, " ");

    std::vector<std::string> words;

    std::istringstream iss(phrase);
    while (!iss.eof()) {
        std::string w;
        iss >> w;

        std::transform(
                w.begin(), w.end(),
                w.begin(),
                [](unsigned char c) { return std::tolower(c); }
        );

        if (w.length() < mMinLength)
            continue;
        if (!atLeastOneLetter(w))
            continue;
        if (mExclusions.count(w) > 0)
            continue;

        words.push_back(w);
    }

    for (const auto &word : words) {
        if (mEntries.count(word) == 0) {
            mEntries[word] = 0;
        }

        ++mEntries[word];
    }
}

std::map<std::string, unsigned int> KeywordEntries::getTop(size_t limit) const {
    std::vector<std::pair<std::string, unsigned int>> tmp;
    for (auto &it : mEntries) {
        tmp.emplace_back(it);
    }
    auto descCmp = [](
            const std::pair<std::string, int> &a,
            const std::pair<std::string, int> &b
    ) -> bool {
        return a.second > b.second;
    };
    sort(tmp.begin(), tmp.end(), descCmp);

    std::map<std::string, unsigned int> result;
    size_t counter = 0;
    for (const auto &pr : tmp) {
        if (counter > limit)
            break;
        result.insert(pr);
        ++counter;
    }
    return result;
}

void KeywordEntries::clear() {
    mEntries.clear();
}
