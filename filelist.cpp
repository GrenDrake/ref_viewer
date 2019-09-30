#include <algorithm>
#include <fstream>
#include <random>
#include <string>
#include <vector>


void shuffleFileList(std::vector<std::string> &dest) {
    static std::random_device rd;
    static auto rng = std::default_random_engine{rd()};

    std::shuffle(std::begin(dest), std::end(dest), rng);
}

bool getFileList(std::vector<std::string> &dest) {

    std::ifstream inf("list.txt");

    std::string line;
    while (std::getline(inf, line)) {
        if (line.empty()) continue;
        dest.push_back(line);
    }

    shuffleFileList(dest);
    return true;
}
