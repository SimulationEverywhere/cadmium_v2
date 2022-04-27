#include <string>
#include "devstone_atomic.hpp"
#include "devstone_coupled_hi.hpp"

DEVStoneCoupledHI::DEVStoneCoupledHI(int width, int depth, int intDelay, int extDelay): DEVStoneCoupled(width, depth, intDelay, extDelay) {
	if (depth > 1)  {
		childCoupled = std::make_shared<DEVStoneCoupledHI>(width, depth - 1, intDelay, extDelay);
		addComponent(childCoupled);
		addCoupling(getInPort("in"), childCoupled->getInPort("in"));
		addCoupling(childCoupled->getOutPort("out"), getOutPort("out"));
		std::shared_ptr<DEVStoneAtomic> prevAtomic = nullptr;
		for (auto i = 1; i < width; ++i) {
			auto atomic = std::make_shared<DEVStoneAtomic>("atomic" + std::to_string(i) + "-" + std::to_string(depth), intDelay, extDelay);
			addComponent(atomic);
			addCoupling(getInPort("in"), atomic->getInPort("in"));
			if (prevAtomic != nullptr) {
				addCoupling(prevAtomic->getOutPort("out"), atomic->getInPort("in"));
			}
			prevAtomic = atomic;
		}
	}
}
