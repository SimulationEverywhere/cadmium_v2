#include <string>
#include "devstone_atomic.hpp"
#include "devstone_coupled_li.hpp"

namespace cadmium::example::devstone {
	DEVStoneCoupledLI::DEVStoneCoupledLI(int width, int depth, int intDelay, int extDelay) : DEVStoneCoupled(width, depth, intDelay, extDelay) {
		if (depth > 1) {
			childCoupled = addComponent<DEVStoneCoupledLI>(width, depth - 1, intDelay, extDelay);
			addCoupling(getInPort("in"), childCoupled->getInPort("in"));
			addCoupling(childCoupled->getOutPort("out"), getOutPort("out"));
			for (auto i = 1; i < width; ++i) {
				auto atomic = addComponent<DEVStoneAtomic>("atomic" + std::to_string(i) + "-" + std::to_string(depth), intDelay, extDelay);
				addCoupling(getInPort("in"), atomic->in);
			}
		}
	}
}  //namespace cadmium::example::devstone
