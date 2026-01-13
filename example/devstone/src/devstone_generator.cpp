/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022-present Guillermo Trabes
 * ARSLab - Carleton University
 * Copyright (c) 2022-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 */

#include <limits>
#include "devstone_generator.hpp"

namespace cadmium::example::devstone {
    DEVStoneGenerator::DEVStoneGenerator(): Atomic<double>("DEVStoneGenerator", 0.)  {
        out = addOutPort<int>("out");
    }

    void DEVStoneGenerator::internalTransition(double& s) const {
        s = std::numeric_limits<double>::infinity();
    }

    void DEVStoneGenerator::externalTransition(double& s, double e) const {
        s = std::numeric_limits<double>::infinity();
    }

    void DEVStoneGenerator::output(const double& s) const {
        out->addMessage(-1);
    }

    [[nodiscard]] double DEVStoneGenerator::timeAdvance(const double& s) const {
        return s;
    }
}  //namespace cadmium::example::devstone
