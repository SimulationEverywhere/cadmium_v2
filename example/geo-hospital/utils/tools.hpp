#ifndef CADMIUM_TOOLS_HPP
#define CADMIUM_TOOLS_HPP

#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>

#include <boost/algorithm/string/join.hpp>

#include "../external/json.hpp"


namespace tools {
    static int random(int min, int max) {
        if (min == max) return min;

        return std::rand() % (max - min) + min;
    }

    static int round_to_int(float x) {
        // 1.2 >> 1.7 >> 1
        // 1.6 >> 2.1 >> 2
        // -1.2 >> -0.7 >> -1.7 >> -1
        // -1.6 >> -1.1 >> -2.1 >> -2
        return int(x + 0.5 - (x < 0));
    }

    static std::string trim(std::string s){
        // holy crap this is ugly.
        s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

        return s;
    }

    static std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> splits;
        std::string split;
        std::istringstream ss(s);

        while (std::getline(ss, split, delimiter)) splits.push_back(split);

        return splits;
    }

    static nlohmann::json read_json(std::string path) {
        std::ifstream stream(path);

        if (!stream.is_open() || stream.fail()) throw strerror(errno);

        nlohmann::json token = nlohmann::json::parse(stream);

        stream.close();

        return token;
    }
}

#endif //CADMIUM_TOOLS_HPP