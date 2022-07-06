#include <string>
#include <vector>
#include "devstone_atomic.hpp"
#include "devstone_coupled_homod.hpp"

namespace cadmium::example::devstone {
	DEVStoneCoupledHOmod::DEVStoneCoupledHOmod(int width, int depth, int intDelay, int extDelay)
		: DEVStoneCoupled(width, depth, intDelay, extDelay) {
		addInPort<int>("in2");
		if (depth > 1) {
			childCoupled = addComponent<DEVStoneCoupledHOmod>(width, depth - 1, intDelay, extDelay);
			addCoupling(getInPort("in"), childCoupled->getInPort("in"));
			addCoupling(childCoupled->getOutPort("out"), getOutPort("out"));

			std::vector<std::shared_ptr<DEVStoneAtomic>> prevLayer, currentLayer;
			// First layer of atomic models:
			for (int i = 1; i < width; ++i) {
				auto atomic = addComponent<DEVStoneAtomic>("atomic(1," + std::to_string(i) + ")-" + std::to_string(depth), intDelay, extDelay);
				addCoupling(getInPort("in2"), atomic->in);
				addCoupling(atomic->out, childCoupled->getInPort("in2"));
				prevLayer.push_back(atomic);
			}
			// Second layer of atomic models:
			for (int i = 1; i < width; ++i) {
				auto atomic = addComponent<DEVStoneAtomic>("atomic(2," + std::to_string(i) + ")-" + std::to_string(depth), intDelay, extDelay);
				if (i == 1) {
					addCoupling(getInPort("in2"), atomic->in);
				}
				for (const auto& prevAtomic: prevLayer) {
					addCoupling(atomic->out, prevAtomic->in);
				}
				currentLayer.push_back(atomic);
			}
			// Rest of the tree
			prevLayer = currentLayer;
			currentLayer = std::vector<std::shared_ptr<DEVStoneAtomic>>();
			for (int layer = 3; layer <= width; ++layer) {
				for (int i = 1; i < prevLayer.size(); ++i) {
					auto atomic = addComponent<DEVStoneAtomic>("atomic(" + std::to_string(layer) + "," + std::to_string(i) + ")-" + std::to_string(depth), intDelay, extDelay);
					if (i == 1) {
						addCoupling(getInPort("in2"), atomic->in);
					}
					addCoupling(atomic->out, prevLayer.at(i)->in);
					currentLayer.push_back(atomic);
				}
				prevLayer = currentLayer;
				currentLayer = std::vector<std::shared_ptr<DEVStoneAtomic>>();
			}
		}
	}
}  // namespace cadmium::example::devstone
