//
// Created by Bruno St-Aubin on 2022-01-08.
//

#ifndef CADMIUM_CBM_TOOLS_HPP
#define CADMIUM_CBM_TOOLS_HPP

#include <string>
#include <utility>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <utility>
#include <json.hpp>

namespace cadmium::cbm::tools {
    static std::string join_strings(std::vector<std::string> strings, char separator) {
        auto fn = [separator](std::string &ss, std::string &s) {
            if (ss.empty()) return s;

            return (ss.back() == separator) ? ss + s : ss + separator + s;
        };

        return std::accumulate(std::begin(strings), std::end(strings), std::string(), fn);
    }

    static std::string path(std::vector<std::string> strings) {
        return join_strings(std::move(strings), '/');
    }

    static void read_file(const std::string& path, const std::function<void(std::ifstream& stream)>& fn) {
        std::ifstream stream(path);

        // TODO: ERROR HANDLING
        if (!stream.is_open() || stream.fail()) throw std::bad_exception();

        fn(stream);
        stream.close();
    }

    static nlohmann::json read_json(const std::string& path) {
        nlohmann::json json;

        read_file(path, [&json](std::ifstream& stream) {
            json = nlohmann::json::parse(stream);
        });

        return json;
    }
}

#endif //CADMIUM_CBM_TOOLS_HPP
