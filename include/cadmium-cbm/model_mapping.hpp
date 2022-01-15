#ifndef WEB_CBM_WORKFLOW_HPP_
#define WEB_CBM_WORKFLOW_HPP_

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <json.hpp>

namespace cadmium::cbm {
    class Validator {
    public:
        static bool IsSourced(const nlohmann::json& json) {
            return json.contains("model_type") && json.contains("source");
        }

        static bool IsAutoCoupled(const nlohmann::json& json) {
            return json.contains("id") && json.contains("model_type") && json.contains("components");
        }

        static bool IsSourceDirect(const nlohmann::json& json) {
            return json.at("type").get<std::string>() == "direct";
        }

        static bool IsSourceGeoJson(const nlohmann::json& json) {
            return json.at("type").get<std::string>() == "geojson";
        }

        static bool IsSourceJson(const nlohmann::json& json) {
            return json.at("type").get<std::string>() == "json";
        }

        static bool IsSourceCsv(const nlohmann::json& json) {
            return json.at("type").get<std::string>() == "csv";
        }
    };

    class Source {
    public:
        std::string type;

        explicit Source(nlohmann::json json) {
            type = json.at("type").get<std::string>();
        }

        virtual ~Source() = default;

        virtual std::vector<nlohmann::json> Read() = 0;
    };

    class SourceDirect: public Source {
    public:
        nlohmann::json data;

        explicit SourceDirect(const nlohmann::json& json): Source(json) {
            data = json.at("data");
        }

        virtual ~SourceDirect() = default;

        std::vector<nlohmann::json> Read() override {
            std::vector<nlohmann::json> items;
            for (auto& j : data) items.push_back(j);
            return items;
        };
    };

    class SourceData: public Source {
    public:
        std::string file;

        explicit SourceData(const nlohmann::json& json): Source(json) {
            file = json.at("file").get<std::string>();
        }

        virtual ~SourceData() = default;

        virtual std::vector<nlohmann::json> ReadFile(std::ifstream& stream) = 0;

        std::vector<nlohmann::json> Read() override {
            std::ifstream stream(file);

            if (!stream.is_open() || stream.fail()) throw std::bad_exception(); // TODO custom exceptions

            std::vector<nlohmann::json> items = ReadFile(stream);
            stream.close();

            return items;
        }
    };

    class SourceGeoJson: public SourceData {
    public:
        std::map<std::string, std::string> mapping;

        explicit SourceGeoJson(const nlohmann::json& json): SourceData(json) {
            mapping = json.at("mapping").get<std::map<std::string, std::string>>();
        }

        virtual ~SourceGeoJson() = default;

        std::vector<nlohmann::json> ReadFile(std::ifstream& stream) override {
            nlohmann::json json = nlohmann::json::parse(stream);

            if (json.is_null() || !json.contains("features") || !json["features"].is_array()) {
                throw std::bad_exception(); // TODO custom exceptions
            }

            std::vector<nlohmann::json> items;

            for (auto& f : json["features"]) {
                nlohmann::json item;
                for (auto const& kv : mapping) item[kv.first] = f["properties"][kv.second];
                items.push_back(item);
            }

            return items;
        };
    };

    class SourceJson: public SourceData {
    public:
        explicit SourceJson(const nlohmann::json& json): SourceData(json) { }

        virtual ~SourceJson() = default;

        std::vector<nlohmann::json> ReadFile(std::ifstream& stream) override {
            nlohmann::json json = nlohmann::json::parse(stream);

            if (json.is_null() || !json.is_array()) {
                throw std::bad_exception(); // TODO custom exceptions
            }

            std::vector<nlohmann::json> items;
            for (auto& f : json) items.push_back(f);

            return items;
        };
    };

    class SourceCsv: public SourceData {
    public:
        explicit SourceCsv(const nlohmann::json& json): SourceData(json) { }

        virtual ~SourceCsv() = default;

        std::vector<nlohmann::json> ReadFile(std::ifstream& stream) override {
            // TODO Not implemented
            throw std::bad_exception(); // TODO custom exceptions
        };
    };

    class Coupling {
    public:
        std::string from_model;
        std::string from_port;
        std::string to_model;
        std::string to_port;
        std::vector<std::vector<std::string>> links;

        explicit Coupling(const nlohmann::json& json) {
            from_model = json.at("from_model").get<std::string>();
            from_port = json.at("from_port").get<std::string>();
            to_model = json.at("to_model").get<std::string>();
            to_port = json.at("to_port").get<std::string>();
            links = json.at("links").get<std::vector<std::vector<std::string>>>();
        }

        virtual ~Coupling() = default;
    };

    // Base class for both Sourced and Automated coupled models.
    class Component {
    public:
        std::string model_type;

        Component() = default;

        explicit Component(const nlohmann::json& json) {
            model_type = json.at("model_type").get<std::string>();
        }

        virtual ~Component() = default;
    };

    class Sourced: public Component {
    public:
        std::shared_ptr<Source> source;

        Sourced() = default;

        explicit Sourced(const nlohmann::json& json): Component(json) {
            std::string type = json.at("source").at("type");

            if (Validator::IsSourceDirect(json.at("source"))) source = std::make_shared<SourceDirect>(json.at("source"));
            else if (Validator::IsSourceGeoJson(json.at("source"))) source = std::make_shared<SourceGeoJson>(json.at("source"));
            else if (Validator::IsSourceJson(json.at("source"))) source = std::make_shared<SourceJson>(json.at("source"));
            else if (Validator::IsSourceCsv(json.at("source"))) source = std::make_shared<SourceCsv>(json.at("source"));
            else throw std::bad_exception(); // TODO custom exceptions
        }

        virtual ~Sourced() = default;
    };

    class AutoCoupled: public Component {
    public:
        std::string id;
        std::vector<std::shared_ptr<Component>> components;
        std::vector<std::shared_ptr<Coupling>> couplings;

        AutoCoupled() = default;

        explicit AutoCoupled(const nlohmann::json& json): Component(json) {
            id = json.at("id").get<std::string>();

            for (auto& j : json.at("components")) {
                if (Validator::IsSourced(j)) components.push_back(std::make_shared<Sourced>(j));
                else if (Validator::IsAutoCoupled(j)) components.push_back(std::make_shared<AutoCoupled>(j));
            }

            for (auto& j : json.at("couplings")) couplings.push_back(std::make_shared<Coupling>(j));
        }

        virtual ~AutoCoupled() = default;
    };
}

#endif //WEB_CBM_WORKFLOW_HPP_