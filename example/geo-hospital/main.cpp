#include "./utils/tools.hpp"
#include "./coupled/gis_emergencies.hpp"

#include <cadmium/core/cadmium.hpp>
#include <cadmium-viz/logger/web_viewer.hpp>
#include <cadmium-cbm/factory.hpp>

// TODO: Had to mess up the CMakeList.txt, it was complaining about multiple main functions
int main() {
    cadmium::cbm::Factory::initialize();

    nlohmann::json hospitals = tools::read_json("./data/hospitals.geojson");
    nlohmann::json areas = tools::read_json("./data/emergency_areas.geojson");

    // TODO: Is there a way to remove the make_shared?
    auto model = std::make_shared<gis_emergencies>("top", hospitals, areas);
    auto logger = std::make_shared<cadmium::WebViewerLogger>(model, "./output/");

    int comp_size = model->components.size();
    int coup_size = model->IC.size();

    cadmium::Simulate(logger, model, 100.0);

    return 0;
}