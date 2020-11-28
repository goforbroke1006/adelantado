//
// Created by goforbroke on 10.11.2020.
//

#ifndef ADELANTADO_CFGLOADER_H
#define ADELANTADO_CFGLOADER_H

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <libconfig.h++>

inline
libconfig::Config *loadAppConfig(const std::string &filename) {
    auto *cfg = new libconfig::Config;
    try {
        cfg->readFile(filename.c_str());
    }
    catch (const libconfig::FileIOException &fioex) {
        std::cerr << "I/O error while reading file." << std::endl;
        exit(EXIT_FAILURE); // TODO: fix
    }
    return cfg;
}

inline
std::vector<std::string> loadConfig(const std::string &filename) {
    std::ifstream ifs(filename);
    if (!ifs || !ifs.is_open()) {
        throw std::runtime_error("file not found or locked");
    }

    std::vector<std::string> result;

    std::string line;
    while (std::getline(ifs, line)) {
        result.push_back(line);
    }

    ifs.close();

    return result;
}

inline
void saveConfig(const std::string &filename, const std::vector<std::string> &rows) {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        throw std::runtime_error("file not found or locked");
    }

    for (const auto &row : rows) {
        ofs << row << std::endl;
    }

    ofs.close();
}

#endif //ADELANTADO_CFGLOADER_H
