#ifndef _CADMIUM_EXAMPLE_DEVSTONE_ATOMIC_HPP_
#define _CADMIUM_EXAMPLE_DEVSTONE_ATOMIC_HPP_

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>
#include <memory>

struct DEVStoneState {
	double sigma;
	unsigned long nTransitions;
	DEVStoneState();
};

std::ostream &operator << (std::ostream &os, const DEVStoneState &x);

class DEVStoneAtomic: public cadmium::Atomic<DEVStoneState> {
 private:
	const std::shared_ptr<cadmium::Port<int>> in, out;
	const int intDelay, extDelay;
 public:
	DEVStoneAtomic(const std::string& id, int intDelay, int extDelay);
	[[nodiscard]] unsigned long nTransitions() const;
	void internalTransition(DEVStoneState& s) const override;
	void externalTransition(DEVStoneState& s, double e, const cadmium::PortSet& x) const override;
	void output(const DEVStoneState& s, const cadmium::PortSet& y) const override;
	[[nodiscard]] double timeAdvance(const DEVStoneState& s) const override;
};

#endif //_CADMIUM_EXAMPLE_DEVSTONE_ATOMIC_HPP_
