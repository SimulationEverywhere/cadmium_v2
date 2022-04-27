#include <string>
#include <vector>
#include "devstone_atomic.hpp"
#include "devstone_coupled_homod.hpp"

DEVStoneCoupledHOmod::DEVStoneCoupledHOmod(int width, int depth, int intDelay, int extDelay): DEVStoneCoupled(width, depth, intDelay, extDelay) {
	addInPort<int>("in2");
	if (depth > 1)  {
		childCoupled = std::make_shared<DEVStoneCoupledHOmod>(width, depth - 1, intDelay, extDelay);
		addComponent(childCoupled);
		addCoupling(getInPort("in"), childCoupled->getInPort("in"));
		addCoupling(childCoupled->getOutPort("out"), getOutPort("out"));

		std::vector<std::shared_ptr<DEVStoneAtomic>> prevLayer, currentLayer;
		// First layer of atomic models:
		for (int i = 1; i < width; ++i) {
			auto atomic = std::make_shared<DEVStoneAtomic>("atomic(1," + std::to_string(i) + ")-" +std::to_string(depth), intDelay, extDelay);
			addComponent(atomic);
			addCoupling(getInPort("in2"), atomic->getInPort("in"));
			Coupled::addCoupling(atomic->getOutPort("out"), childCoupled->getInPort("in2"));
			prevLayer.push_back(atomic);
		}
		// Second layer of atomic models:
		for (int i = 1; i < width; ++i) {
			auto atomic = std::make_shared<DEVStoneAtomic>("atomic(2," + std::to_string(i) + ")-" +std::to_string(depth), intDelay, extDelay);
			addComponent(atomic);
			if (i == 1) {
				addCoupling(getInPort("in2"), atomic->getInPort("in"));
			}
			for (const auto& prevAtomic: prevLayer) {
				addCoupling(atomic->getOutPort("out"), prevAtomic->getInPort("in"));
			}
			currentLayer.push_back(atomic);
		}
		// Rest of the tree
		prevLayer = currentLayer;
		currentLayer = std::vector<std::shared_ptr<DEVStoneAtomic>>();
		for (int layer = 3; layer <= width; ++layer) {
			for (int i = 1; i < prevLayer.size(); ++i) {
				auto atomic = std::make_shared<DEVStoneAtomic>("atomic(" + std::to_string(layer) + "," + std::to_string(i) + ")-" +std::to_string(depth), intDelay, extDelay);
				addComponent(atomic);
				if (i == 1) {
					addCoupling(getInPort("in2"), atomic->getInPort("in"));
				}
				addCoupling(atomic->getOutPort("out"), prevLayer.at(i)->getInPort("in"));
				currentLayer.push_back(atomic);
			}
			prevLayer = currentLayer;
			currentLayer = std::vector<std::shared_ptr<DEVStoneAtomic>>();
		}
	}
}
