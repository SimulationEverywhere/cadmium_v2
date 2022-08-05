#include <string>
#include "devstone.hpp"
#include "devstone_coupled.hpp"
#include "devstone_generator.hpp"

namespace cadmium::example::devstone {
    DEVStone::DEVStone(const std::string& type, int width, int depth, int intDelay, int extDelay) : Coupled(
        "coupled" + std::to_string(depth)), devstone(DEVStoneCoupled::newDEVStoneCoupled(type, width, depth, intDelay, extDelay)) {
        addComponent(devstone);
        auto generator = addComponent<DEVStoneGenerator>();
        for (const auto& [portId, port]: devstone->getInPorts()) {
            addCoupling(generator->out, port);
        }
    }

    unsigned long DEVStone::nAtomics() const {
        return devstone->nAtomics();
    }

    unsigned long DEVStone::nEICs() const {
        return devstone->nEICs();
    }

    unsigned long DEVStone::nICs() const {
        return devstone->nICs();
    }

    unsigned long DEVStone::nEOCs() const {
        return devstone->nEOCs();
    }

    unsigned long DEVStone::nTransitions() const {
        return devstone->nTransitions();
    }
}  //namespace cadmium::example::devstone
