#ifndef _CADMIUM_SIMULATION_ABS_SIMULATOR_HPP_
#define _CADMIUM_SIMULATION_ABS_SIMULATOR_HPP_

#include <cadmium/core/modeling/component.hpp>
#include <limits>
#include <memory>

namespace cadmium {
    struct AbstractSimulator {
        double timeLast, timeNext;
        explicit AbstractSimulator(double time): timeLast(time), timeNext(std::numeric_limits<double>::infinity()) {};
        virtual ~AbstractSimulator() = default;

        virtual std::shared_ptr<Component> getComponent() = 0;
        virtual void collection(double time) = 0;
        virtual void transition(double time) = 0;
        virtual void clear() {
            getComponent()->clearPorts();
        };
    };
}

#endif //_CADMIUM_SIMULATION_ABS_SIMULATOR_HPP_
