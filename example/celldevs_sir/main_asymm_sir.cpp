#include <cadmium/celldevs/asymm/coupled.hpp>
#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <fstream>
#include <string>
#include "asymm_sir_cell.hpp"

using namespace cadmium::celldevs;
using namespace cadmium::celldevs::example::sir;

std::shared_ptr<AsymmCell<SIRState, double>> addAsymmCell(const std::string& cellId, const std::shared_ptr<const AsymmCellConfig<SIRState, double>>& cellConfig) {
	auto cellModel = cellConfig->cellModel;
	if (cellModel == "default" || cellModel == "SIR") {
		return std::make_shared<AsymmSIRCell>(cellId, cellConfig);
	} else {
		throw std::bad_typeid();
	}
}

int main(int argc, char ** argv) {
	if (argc < 2) {
		std::cout << "Program used with wrong parameters. The program must be invoked as follows:";
		std::cout << argv[0] << " SCENARIO_CONFIG.json [MAX_SIMULATION_TIME (default: 500)]" << std::endl;
		return -1;
	}
	std::string configFilePath = argv[1];
	double simTime = (argc > 2)? atof(argv[2]) : 500;

	auto model = std::make_shared<AsymmCellDEVSCoupled<SIRState, double>>("sir", addAsymmCell, configFilePath);
	model->buildModel();
	auto rootCoordinator = cadmium::RootCoordinator(model);
	auto logger = std::make_shared<cadmium::CSVLogger>("asymm_log.csv", ";");
	rootCoordinator.setLogger(logger);
	rootCoordinator.start();
	rootCoordinator.simulate(simTime);
	rootCoordinator.stop();
}
