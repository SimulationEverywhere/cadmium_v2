#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include "blinkySystem.hpp"

using namespace cadmium::example::blinkySystem;

int main(int argc, char *argv[]) {
	// First, we parse the arguments
	std::ifstream file;

	
	auto model = std::make_shared<blinkySystem>("blinkySystem");
	auto rootCoordinator = cadmium::RootCoordinator(model);
	auto logger = std::make_shared<cadmium::CSVLogger>("log_blinkySystem.csv", ",");
	rootCoordinator.setLogger(logger);
	rootCoordinator.start();
	rootCoordinator.simulate(std::numeric_limits<double>::infinity());
	rootCoordinator.stop();
	return 0;
}
