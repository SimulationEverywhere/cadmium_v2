#include <cadmium/modeling/celldevs/asymm/coupled.hpp>
#include <cadmium/simulation/logger/csv.hpp>
#include <cadmium/simulation/root_coordinator.hpp>
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
    auto paramsProcessed = std::chrono::high_resolution_clock::now();

	auto model = std::make_shared<AsymmCellDEVSCoupled<SIRState, double>>("sir", addAsymmCell, configFilePath);
	model->buildModel();
    auto modelGenerated = std::chrono::high_resolution_clock::now();
    std::cout << "Model creation time: " << std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>( modelGenerated - paramsProcessed).count() << " seconds" << std::endl;

    modelGenerated = std::chrono::high_resolution_clock::now();
	auto rootCoordinator = cadmium::RootCoordinator(model);
	rootCoordinator.setLogger<cadmium::CSVLogger>("log_asymm_sir.csv", ";");
	rootCoordinator.start();
    auto engineStarted = std::chrono::high_resolution_clock::now();
    std::cout << "Engine creation time: " << std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(engineStarted - modelGenerated).count() << " seconds" << std::endl;

    engineStarted = std::chrono::high_resolution_clock::now();
    rootCoordinator.simulate(simTime);
    auto simulationDone =  std::chrono::high_resolution_clock::now();
    std::cout << "Simulation time: " << std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(simulationDone - engineStarted).count() << " seconds" << std::endl;
	rootCoordinator.stop();
}
