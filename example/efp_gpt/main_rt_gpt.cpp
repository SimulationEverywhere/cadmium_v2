/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022-present Guillermo Trabes
 * Copyright (c) 2022-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 */

#include <cadmium/simulation/logger/csv.hpp>
#include <cadmium/simulation/rt_clock/chrono.hpp>
#include <cadmium/simulation/rt_root_coordinator.hpp>
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

    // Let's create the RT clock. In this case, we will use a chrono clock with a maximum jitter of 10 milliseconds
    // note that the max jitter is optional. If left blank, it won't check the delay jitter.
    auto maxJitter = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::milliseconds(10));
    auto clock = cadmium::ChronoClock(maxJitter);

    // For creating RT coordinators, we must forward the model and the desired RT clock
    auto rootCoordinator = cadmium::RealTimeRootCoordinator(model, clock);
    // The rest works as with the other root coordinators.
    rootCoordinator.setLogger<cadmium::CSVLogger>("log_rt_gpt.csv", ";");
    rootCoordinator.start();
    rootCoordinator.simulate(std::numeric_limits<double>::infinity());
    rootCoordinator.stop();
    return 0;
}
