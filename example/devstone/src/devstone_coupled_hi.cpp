#include <string>
#include "devstone_atomic.hpp"
#include "devstone_coupled_hi.hpp"

namespace cadmium::example::devstone {
	DEVStoneCoupledHI::DEVStoneCoupledHI(int width, int depth, int intDelay, int extDelay) : DEVStoneCoupled(width, depth, intDelay, extDelay) {
		if (depth > 1) {
			childCoupled = addComponent<DEVStoneCoupledHI>(width, depth - 1, intDelay, extDelay);
			addCoupling(getInPort("in"), childCoupled->getInPort("in"));
			addCoupling(childCoupled->getOutPort("out"), getOutPort("out"));
			std::shared_ptr<DEVStoneAtomic> prevAtomic = nullptr;
			for (auto i = 1; i < width; ++i) {
				auto atomic = addComponent<DEVStoneAtomic>("atomic" + std::to_string(i) + "-" + std::to_string(depth), intDelay, extDelay);
				addCoupling(getInPort("in"), atomic->in);
				if (prevAtomic != nullptr) {
					addCoupling(prevAtomic->out, atomic->in);
				}
				prevAtomic = atomic;
			}
		}
	}
}  //namespace cadmium::example::devstone
