#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include "efp.hpp"

using namespace cadmium::example::gpt;

int main(int argc, char *argv[]) {
	// First, we parse the arguments
	if (argc < 4) {
		std::cerr << "ERROR: not enough arguments" << std::endl;
		std::cerr << "    Usage:" << std::endl;
		std::cerr << "    > main_efp JOB_GENERATION_PERIOD JOB_PROCESSING_TIME OBSERVATION_TIME" << std::endl;
		std::cerr << "        (JOB_GENERATION_PERIOD, JOB_PROCESSING_TIME, and OBSERVATION_TIME must be greater than or equal to 0)" << std::endl;
		return -1;
	}
	int jobPeriod = std::stoi(argv[1]);
	if (jobPeriod < 0) {
		std::cerr << "ERROR: JOB_GENERATION_PERIOD is less than 0 (" << jobPeriod << ")" << std::endl;
		return -1;
	}
	int processingTime = std::stoi(argv[2]);
	if (processingTime < 0) {
		std::cerr << "ERROR: JOB_PROCESSING_TIME is less than 0 (" << processingTime << ")" << std::endl;
		return -1;
	}
	double obsTime = std::stod(argv[3]);
	if (obsTime < 0) {
		std::cerr << "ERROR: OBSERVATION_TIME is less than 0 (" << obsTime << ")" << std::endl;
		return -1;
	}
	auto model = std::make_shared<EFP>("efp", jobPeriod, processingTime, obsTime);
	model->flatten();

	auto components = model->getComponents();
/*
	for (auto it = components.begin(); it < components.end(); it++) {
		std::cout << "ID:" << (*it)->getId() << std::endl;
	}
*/

	auto couplings = model->getICs();
/*
	for (auto it = couplings.begin(); it < couplings.end(); it++) {
		std::cout << "PORT FROM:" << std::get<(0)>(*it)->getId() << std::endl;
		std::cout << "PORT TO:" << std::get<(1)>(*it)->getId() << std::endl;
		std::cout << std::get<(0)>(*it)->getParent()->getId() << std::endl;
		std::cout << std::get<(1)>(*it)->getParent()->getId() << std::endl;
	}
*/
	auto rootCoordinator = cadmium::RootCoordinator(model);
	auto logger = std::make_shared<cadmium::CSVLogger>("log_efp.csv", ";");
	rootCoordinator.setLogger(logger);
	rootCoordinator.start();
	rootCoordinator.simulate(std::numeric_limits<double>::infinity());
	rootCoordinator.stop();
	return 0;
}