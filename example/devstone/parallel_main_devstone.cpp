#include <chrono>
#include <iostream>
#include <string>
#include "include/devstone_coupled.hpp"
#include <cadmium/core/simulation/parallel_root_coordinator.hpp>

using namespace cadmium::example::devstone;

int main(int argc, char *argv[]) {
	// First, we parse the arguments
	if (argc < 4) {
		std::cerr << "ERROR: not enough arguments" << std::endl;
		std::cerr << "    Usage:" << std::endl;
		std::cerr << "    > main_devstone MODEL_TYPE WIDTH DEPTH INTDELAY EXTDELAY" << std::endl;
		std::cerr << "        (MODEL_TYPE must be either LI, HI, HO, or HOmod)" << std::endl;
		std::cerr << "        (WIDTH and DEPTH must be greater than or equal to 1)" << std::endl;
		std::cerr << "        (INTDELAY and EXTDELAY must be greater than or equal to 0 ms)" << std::endl;
		std::cerr << "    Alternative usages:" << std::endl;
		std::cerr << "    > main_devstone MODEL_TYPE WIDTH DEPTH DELAY" << std::endl;
		std::cerr << "        (INTDELAY and EXTDELAY are set to DELAY ms)" << std::endl;
		std::cerr << "    > main_devstone MODEL_TYPE WIDTH DEPTH" << std::endl;
		std::cerr << "        (INTDELAY and EXTDELAY are set to 0 ms)" << std::endl;
		return -1;
	}
	std::string type = argv[1];
	int width = std::stoi(argv[2]);
	int depth = std::stoi(argv[3]);
	int intDelay = 0;
	int extDelay = 0;
	if (argc > 4) {
		intDelay = std::stoi(argv[4]);
		extDelay = (argc == 5) ? intDelay : std::stoi(argv[5]);
	}
	auto paramsProcessed = std::chrono::high_resolution_clock::now();

	// Then, we generate the corresponding DEVStone model and inject the original
	auto coupled = DEVStoneCoupled::newDEVStoneCoupled(type, width, depth, intDelay, extDelay);
	auto modelGenerated = std::chrono::high_resolution_clock::now();
	std::cout << "Model creation time: " << std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>( modelGenerated - paramsProcessed).count() << " seconds" << std::endl;

	// Then, we inject initial events and create and start the simulation engine
	modelGenerated = std::chrono::high_resolution_clock::now();
	auto rootCoordinator = cadmium::ParallelRootCoordinator(coupled);
	rootCoordinator.start();
	auto engineStarted = std::chrono::high_resolution_clock::now();
	std::cout << "Engine creation time: " << std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(engineStarted - modelGenerated).count() << " seconds" << std::endl;

	// Simulation starts
	engineStarted = std::chrono::high_resolution_clock::now();
	rootCoordinator.simulate(std::numeric_limits<double>::infinity());
	auto simulationDone =  std::chrono::high_resolution_clock::now();
	std::cout << "Simulation time: " << std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(simulationDone - engineStarted).count() << " seconds" << std::endl;
	// Once we are done, we stop the simulation engine
	rootCoordinator.stop();

	return 0;
}
