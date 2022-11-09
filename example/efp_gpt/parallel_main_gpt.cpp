#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/parallel_root_coordinator.hpp>
#include <limits>
#include "gpt.hpp"

using namespace cadmium::example::gpt;

int main(int argc, char *argv[]) {
    // First, we parse the arguments
    if (argc < 4) {
        std::cerr << "ERROR: not enough arguments" << std::endl;
        std::cerr << "    Usage:" << std::endl;
        std::cerr << "    > main_gpt GENERATION_PERIOD PROCESSING_TIME OBSERVATION_TIME" << std::endl;
        std::cerr << "        (GENERATION_PERIOD, PROCESSING_TIME, and OBSERVATION_TIME must be greater than or equal to 0)" << std::endl;
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

    // Then, we create the model and start the simulation
    auto model = std::make_shared<GPT>("gpt", jobPeriod, processingTime, obsTime);
    auto rootCoordinator = cadmium::ParallelRootCoordinator(model);
    auto logger = std::make_shared<cadmium::CSVLogger>("log_gpt.csv", ";");
    rootCoordinator.setLogger(logger);
    rootCoordinator.start();
    rootCoordinator.simulate(std::numeric_limits<double>::infinity());
    rootCoordinator.stop();
    return 0;
}
