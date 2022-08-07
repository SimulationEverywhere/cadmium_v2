#include <string>
#include "devstone.hpp"
#include "devstone_coupled.hpp"
#include "devstone_generator.hpp"

namespace cadmium::example::devstone {
    DEVStone::DEVStone(const std::string& type, int width, int depth, int intDelay, int extDelay) : Coupled(
        "coupled" + std::to_string(depth)), devstone(DEVStoneCoupled::newDEVStoneCoupled(type, width, depth, intDelay, extDelay)) {
        addComponent(devstone);
        auto generator = addComponent<DEVStoneGenerator>();
        for (const auto& port: devstone->getInPorts()) {
            addCoupling(generator->out, port);
        }
    }

    int DEVStone::nAtomics() const {
        return devstone->nAtomics();
    }

    int DEVStone::nEICs() const {
        return devstone->nEICs();
    }

    int DEVStone::nICs() const {
        return devstone->nICs();
    }

    int DEVStone::nEOCs() const {
        return devstone->nEOCs();
    }

    int DEVStone::nInternals() const {
        return devstone->nInternals();
    }

    int DEVStone::nExternals() const {
        return devstone->nExternals();
    }

    int DEVStone::nEvents() const {
        return devstone->nEvents();
    }
}  //namespace cadmium::example::devstone
