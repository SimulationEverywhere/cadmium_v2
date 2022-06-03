#include <limits>
#include "devstone_atomic.hpp"
#include "dhrystone.hpp"

DEVStoneState::DEVStoneState() : sigma(std::numeric_limits<double>::infinity()), nTransitions() {}

std::ostream &operator << (std::ostream &os, const DEVStoneState &x) {
	os << x.nTransitions;
	return os;
}

DEVStoneAtomic::DEVStoneAtomic(const std::string& id, int intDelay, int extDelay): cadmium::Atomic<DEVStoneState>(id, DEVStoneState()),
  in(std::make_shared<cadmium::Port<int>>("in")), out(cadmium::Port<int>::newPort("out")), intDelay(intDelay), extDelay(extDelay) {
	addInPort(in);
	addOutPort(out);
}

[[maybe_unused]] unsigned long DEVStoneAtomic::nTransitions() const {
	return state.nTransitions;
}

void DEVStoneAtomic::internalTransition(DEVStoneState& s) const {
	runDhrystone(intDelay);
	s.sigma = std::numeric_limits<double>::infinity();
}

void DEVStoneAtomic::externalTransition(DEVStoneState& s, double e, const cadmium::PortSet& x) const {
	runDhrystone(extDelay);
	s.nTransitions += 1;
	s.sigma = 0;
}

void DEVStoneAtomic::output(const DEVStoneState& s, const cadmium::PortSet& y) const {
	out->addMessage(s.nTransitions);
}

double DEVStoneAtomic::timeAdvance(const DEVStoneState& s) const {
	return s.sigma;
}
