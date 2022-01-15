#include <cadmium/core/cadmium.hpp>
#include <cadmium-cbm/factory.hpp>
#include <cadmium-cbm/tools.hpp>
#include <cadmium-cbm/model_mapping.hpp>
#include <cadmium-cbm/coupler.hpp>
#include <cadmium-viz/logger/web_viewer.hpp>

// TODO: Had to mess up the CMakeList.txt, it was complaining about multiple main functions
int main(int argc, char ** argv){
    if (argc < 3) {
        std::cout << "Program used with wrong parameters. The program must be invoked as follow:";
        std::cout << argv[0] << " path to the model mapping file " << std::endl;
        std::cout << argv[1] << " path to the output folder " << std::endl;
        std::cout << argv[2] << " duration of the simulation, either as a number of iterations or time interval. If left empty, the simulation will run until it passivates. " << std::endl;
        return 1;
    }

    cadmium::cbm::Factory::initialize();

    auto json = cadmium::cbm::tools::read_json(argv[1]);
    auto mm_top = std::make_shared<cadmium::cbm::AutoCoupled>(json);
    auto model = cadmium::cbm::coupler::BuildTop(mm_top);
    auto logger = std::make_shared<cadmium::WebViewerLogger>(model, argv[2]);

    int comp_size = model->components.size();
    int coup_size = model->IC.size();
    // TODO: CHECK WHAT HAPPENS WHEN BAD PORTS ARE PROVIDED IN JSON
    // TODO: How to pass length of sim (arguments ?)

    cadmium::Simulate(logger, model, 100.0);

    return 0;
}