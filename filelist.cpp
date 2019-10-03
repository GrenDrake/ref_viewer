#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>


void shuffleFileList(std::vector<std::string> &dest) {
    static std::random_device rd;
    static auto rng = std::default_random_engine{rd()};

    std::shuffle(std::begin(dest), std::end(dest), rng);
}

bool getFileList(std::vector<std::string> &dest, const std::string &filename) {

    std::ifstream inf(filename);
    if (!inf) {
        std::cerr << "Could not open image file list " << filename << "\n";
        return false;
    }

    std::string line;
    while (std::getline(inf, line)) {
        if (line.empty()) continue;
        dest.push_back(line);
    }

    shuffleFileList(dest);
    return true;
}
