#ifndef CADMIUM_CBM_FACTORY_HPP_
#define CADMIUM_CBM_FACTORY_HPP_

#include <cadmium/core/modeling/component.hpp>
#include "includes.hpp"

namespace cadmium::cbm {

    class Factory {

    private:
        // NOTE: map of string and pointer to function which returns a component pointer.
        inline static std::map<std::string, std::function<std::shared_ptr<Component>(std::string id, nlohmann::json json)>> map;

        template<typename T>
        static std::shared_ptr<T> createT(const std::string& id, const nlohmann::json& json) {
            return std::make_shared<T>(id, json);
        }

    public:
        template<typename T>
        static void register_component(const std::string& s) {
            map.insert(std::make_pair(s, &createT<T>));
        }

        static void initialize() {
            // TODO: Can this be done elsewhere? ideally, in includes.hpp
            Factory::register_component<hospital>("hospital");
            Factory::register_component<emergency_area>("emergency_area");
            // Factory::register_component<gis_emergencies>("gis_emergencies");
            // TODO: remove type from parameters in atomic and coupled models
            // TODO: register coupled models, use different map
        }

        static bool contains(const std::string& s) {
            return map.find(s) != map.end();
        }

        static std::shared_ptr<cadmium::Component> build(const std::string& s, const std::string& id, const nlohmann::json& json) {
            auto it = map.find(s);

            if (it == map.end()) return nullptr;

            return it->second(id, json);
        }
    };
}

#endif //CADMIUM_CBM_FACTORY_HPP_
