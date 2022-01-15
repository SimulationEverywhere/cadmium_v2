#ifndef CADMIUM_EXAMPLE_GIS_EMERGENCIES_HPP
#define CADMIUM_EXAMPLE_GIS_EMERGENCIES_HPP

#include <cadmium/core/modeling/coupled.hpp>

#include "../atomics/hospital.hpp"
#include "../atomics/emergency_area.hpp"

class gis_emergencies: public cadmium::Coupled {
public:
    explicit gis_emergencies(std::string id, nlohmann::json j_hospitals, nlohmann::json j_areas):
            cadmium::Coupled(std::move(id), "gis_emergencies") {
        std::map<std::string, nlohmann::json> m_areas;
        std::map<std::string, nlohmann::json> m_hospitals;


        for (auto& f : j_areas.at("features")) {
            nlohmann::json j = f.at("properties");
            auto geo_id = j.at("dauid").get<std::string>();
            m_areas[geo_id] = j;

            this->addComponent(emergency_area(geo_id, j));
        }

        for (auto& f : j_hospitals.at("features")) {
            nlohmann::json j = f.at("properties");
            auto geo_id = j.at("index").get<std::string>();
            m_hospitals[geo_id] = j;

            this->addComponent(hospital(geo_id, j));
        }

        for (auto& f : j_areas.at("features")) {
            auto area_id = f.at("properties").at("dauid").get<std::string>();
            auto hospitals = f.at("properties").at("hospitals").get<std::string>();
            std::vector<std::string> split = tools::split(tools::trim(hospitals), ',');

            nlohmann::json area = m_areas[area_id].at("dauid").get<std::string>();

            nlohmann::json h1 = m_hospitals[split[0]].at("index").get<std::string>();
            nlohmann::json h2 = m_hospitals[split[1]].at("index").get<std::string>();
            nlohmann::json h3 = m_hospitals[split[2]].at("index").get<std::string>();

            addInternalCoupling(area_id, "out_1", h1, "processor_in");
            addInternalCoupling(area_id, "out_2", h2, "processor_in");
            addInternalCoupling(area_id, "out_3", h3, "processor_in");
            addInternalCoupling(h1, "processor_out", area, "rejected_1");
            addInternalCoupling(h2, "processor_out", area, "rejected_1");
            addInternalCoupling(h3, "processor_out", area, "rejected_1");
        }
    }
};

#endif // CADMIUM_EXAMPLE_GIS_EMERGENCIES_HPP