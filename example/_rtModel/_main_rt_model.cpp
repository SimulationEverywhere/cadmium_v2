#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include "covidSupervisorySystem.hpp"

using namespace cadmium::example::covidSupervisorySystem;

int main(int argc, char *argv[]) {
	// First, we parse the arguments
	std::ifstream file;

	
	auto model = std::make_shared<covidSupervisorySystem>("covidSupervisorySystem");
	auto rootCoordinator = cadmium::RootCoordinator(model);
	auto logger = std::make_shared<cadmium::CSVLogger>("log_covidSupervisorySystem.csv", ",");
	rootCoordinator.setLogger(logger);
	rootCoordinator.start();
	rootCoordinator.simulate(std::numeric_limits<double>::infinity());
	rootCoordinator.stop();
	return 0;
}
