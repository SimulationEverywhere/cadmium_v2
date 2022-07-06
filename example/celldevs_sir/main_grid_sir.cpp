#include "cadmium/core/logger/csv.hpp"
#include "cadmium/core/simulation/root_coordinator.hpp"
#include <fstream>
#include <string>
#include "grid/coupled.hpp"

using namespace cadmium::celldevs::example::sir;

int main(int argc, char ** argv) {
	if (argc < 2) {
		std::cout << "Program used with wrong parameters. The program must be invoked as follows:";
		std::cout << argv[0] << " SCENARIO_CONFIG.json [MAX_SIMULATION_TIME (default: 500)]" << std::endl;
		return -1;
	}
	std::string configFilePath = argv[1];
	double simTime = (argc > 2)? atof(argv[2]) : 500;

	auto model = std::make_shared<GridSIRCoupled>("sir", configFilePath);
	model->buildModel();
	auto rootCoordinator = cadmium::RootCoordinator(model);
	auto logger = std::make_shared<cadmium::CSVLogger>("grid_log.csv", ";");
	rootCoordinator.setLogger(logger);
	rootCoordinator.start();
	rootCoordinator.simulate(simTime);
	rootCoordinator.stop();
}
