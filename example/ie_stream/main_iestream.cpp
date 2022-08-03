#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include "iestreamTest.hpp"


using namespace cadmium::example::iestream;

int main(int argc, char *argv[]) {
	// First, we parse the arguments

	// Then, we create the model and start the simulation
	auto model = std::make_shared<iestreamTest>("iestreamTest");
	//auto model = std::make_shared<ABP>("network");
	auto rootCoordinator = cadmium::RootCoordinator(model);
	auto logger = std::make_shared<cadmium::CSVLogger>("log_iestream.csv", ",");
	rootCoordinator.setLogger(logger);
	rootCoordinator.start();
	rootCoordinator.simulate(std::numeric_limits<double>::infinity());
	rootCoordinator.stop();
	return 0;
}
