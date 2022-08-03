#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include "iestream_coupled.hpp"

using namespace cadmium::example::iestream;

int main(int argc, char *argv[]) {
	// First, we parse the arguments
	std::ifstream file;

	if (argc < 2) { // Check that file is included
        std::cerr << "ERROR: not enough arguments" << std::endl;
        std::cerr << "    Usage:" << std::endl;
        std::cerr << "    > main_iestream INPUTFILE" << std::endl;
        return -1;
    }

	const char* filePath = argv[1];
    
	file.open(filePath);
    if(!file.is_open()) { // Check file can be opened
        std::cerr << "ERROR: file can not be opened. Check file path." << std::endl;
        return -1;
    }

	auto model = std::make_shared<iestream_coupled>("IEStreamCoupled", filePath);
	auto rootCoordinator = cadmium::RootCoordinator(model);
	auto logger = std::make_shared<cadmium::CSVLogger>("log_iestream.csv", ";");
	rootCoordinator.setLogger(logger);
	rootCoordinator.start();
	rootCoordinator.simulate(std::numeric_limits<double>::infinity());
	rootCoordinator.stop();
	return 0;
}
