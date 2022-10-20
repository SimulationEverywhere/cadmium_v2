#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <iostream>
#include <limits>
#include <string>
#include "efp.hpp"

using namespace cadmium::example::gpt;

int main(int argc, char *argv[]) {
    // First, we parse the arguments
    if (argc < 4) {
        std::cerr << "ERROR: not enough arguments" << std::endl;
        std::cerr << "    Usage:" << std::endl;
        std::cerr << "    > main_efp JOB_GENERATION_PERIOD JOB_PROCESSING_TIME OBSERVATION_TIME [flatten]" << std::endl;
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
        std::cerr << "ERROR: JOB_PROCESSING_TIME is less than 0 (" << processingTime << ")." << std::endl;
        return -1;
    }
    double obsTime = std::stod(argv[3]);
    if (obsTime < 0) {
        std::cerr << "ERROR: OBSERVATION_TIME is less than 0 (" << obsTime << ")." << std::endl;
        return -1;
    }
    bool flatten = false;
    if (argc > 4) {
        std::string arg = argv[4];
        if (arg == "flatten") {
            std::cout << "INFO: The model will be flattened before the simulation." << std::endl;
            flatten = true;
        }
        else {
            std::cerr << "WARNING: unknown argument (" << arg << "). It will be ignored." << std::endl;
        }
    }
    auto paramsProcessed = std::chrono::high_resolution_clock::now();

    // Then, we create the model
    auto model = std::make_shared<EFP>("efp", jobPeriod, processingTime, obsTime);
    if (flatten) {
        model->flatten();
    }

    // Next, we create the coordinator and the logger
    auto rootCoordinator = cadmium::RootCoordinator(model);
    auto logger = std::make_shared<cadmium::CSVLogger>("log_efp.csv", ";");
    rootCoordinator.setLogger(logger);

    // And finally, we run the simulation
    rootCoordinator.start();
    rootCoordinator.simulate(std::numeric_limits<double>::infinity());
    rootCoordinator.stop();

    return 0;
}
