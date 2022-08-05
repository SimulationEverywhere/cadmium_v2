#include <iostream>
#include <limits>
#include "devstone_atomic.hpp"
#include "dhrystone.hpp"

namespace cadmium::example::devstone {
	DEVStoneAtomicState::DEVStoneAtomicState(): sigma(std::numeric_limits<double>::infinity()), nInternals(), nExternals(), nEvents() {}

	std::ostream &operator << (std::ostream &os, const DEVStoneAtomicState& x) {
		os << x.nInternals << "," << x.nExternals << "," << x.nEvents;
		return os;
	}

	DEVStoneAtomic::DEVStoneAtomic(const std::string& id, int intDelay, int extDelay):
        Atomic<DEVStoneAtomicState>(id, DEVStoneAtomicState()), intDelay(intDelay), extDelay(extDelay) {
		in = addInPort<int>("in");
		out = addOutPort<int>("out");
	}

	[[nodiscard]] int DEVStoneAtomic::nInternals() const {
		return state.nInternals;
	}

    [[nodiscard]] int DEVStoneAtomic::nExternals() const {
        return state.nExternals;
    }

    [[nodiscard]] int DEVStoneAtomic::nEvents() const {
        return state.nEvents;
    }

	void DEVStoneAtomic::internalTransition(DEVStoneAtomicState& s) const {
		runDhrystone(intDelay);
        s.nInternals += 1;
		s.sigma = std::numeric_limits<double>::infinity();
	}

	void DEVStoneAtomic::externalTransition(DEVStoneAtomicState& s, double e) const {
		runDhrystone(extDelay);
		s.nExternals += 1;
        s.nEvents += in->size();
		s.sigma = 0;
	}

	void DEVStoneAtomic::output(const DEVStoneAtomicState& s) const {
		out->addMessage(s.nEvents);
	}

	[[nodiscard]] double DEVStoneAtomic::timeAdvance(const DEVStoneAtomicState& s) const {
		return s.sigma;
	}
}  //namespace cadmium::example::devstone
