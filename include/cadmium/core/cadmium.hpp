#ifndef CADMIUM_CADMIUM_HPP
#define CADMIUM_CADMIUM_HPP

#include <utility>

#include "./simulation/coordinator.hpp"
#include "./modeling/coupled.hpp"
#include "../logger/logger.hpp"

namespace cadmium {
    static void Simulate(const std::shared_ptr<Logger>& logger, const std::shared_ptr<Coupled>& model, long nIterations) {
        auto coordinator = Coordinator(model);

        coordinator.setLogger(logger);
        coordinator.start();
        coordinator.simulate(nIterations);
        coordinator.stop();
    }

    static void Simulate(const std::shared_ptr<Logger>& logger, const std::shared_ptr<Coupled>& model, double timeInterval) {
        auto coordinator = Coordinator(model);

        coordinator.setLogger(logger);
        coordinator.start();
        coordinator.simulate(timeInterval);
        coordinator.stop();
    }

    static void Simulate(const std::shared_ptr<Logger>& logger, const std::shared_ptr<Coupled>& model) {
        Simulate(logger, model, std::numeric_limits<double>::infinity());
    }
}

#endif //CADMIUM_CADMIUM_HPP
