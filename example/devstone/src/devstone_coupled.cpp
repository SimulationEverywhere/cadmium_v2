#include <string>
#include "devstone_atomic.hpp"
#include "devstone_coupled.hpp"
#include "devstone_coupled_li.hpp"
#include "devstone_coupled_hi.hpp"
#include "devstone_coupled_ho.hpp"
#include "devstone_coupled_homod.hpp"

namespace cadmium::example::devstone {
	DEVStoneCoupled::DEVStoneCoupled(int width, int depth, int intDelay, int extDelay) : Coupled(
		"coupled" + std::to_string(depth)), childCoupled() {
		if (depth < 1 || width < 1) {
			throw std::bad_exception();  // TODO custom exceptions
		}
		addInPort<int>("in");
		addOutPort<int>("out");
		if (depth == 1) {
			auto atomic = addComponent<DEVStoneAtomic>("innerAtomic", intDelay, extDelay);
			addCoupling(getInPort("in"), atomic->in);
			addCoupling(atomic->out, getOutPort("out"));
		}
	}

	std::shared_ptr<DEVStoneCoupled> DEVStoneCoupled::newDEVStoneCoupled(const std::string& type, int width, int depth, int intDelay, int extDelay) {
		if (type == "LI") {
			return std::make_shared<DEVStoneCoupledLI>(width, depth, intDelay, extDelay);
		}
		else if (type == "HI") {
			return std::make_shared<DEVStoneCoupledHI>(width, depth, intDelay, extDelay);
		}
		else if (type == "HO") {
			return std::make_shared<DEVStoneCoupledHO>(width, depth, intDelay, extDelay);
		}
		else if (type == "HOmod") {
			return std::make_shared<DEVStoneCoupledHOmod>(width, depth, intDelay, extDelay);
		}
		else {
			throw std::bad_exception();  // TODO custom exceptions
		}
	}

	unsigned long DEVStoneCoupled::nAtomics() const {
		auto res = components.size();
		if (childCoupled != nullptr) {
			res += childCoupled->nAtomics() - 1;
		}
		return res;
	}

	unsigned long DEVStoneCoupled::nEICs() const {
        auto res = 0;
        for (const auto& [portTo, portsFrom]: EIC) {
            res += portsFrom.size();
        }
		if (childCoupled != nullptr) {
			res += childCoupled->nEICs();
		}
		return res;
	}

	unsigned long DEVStoneCoupled::nICs() const {
        auto res = 0;
        for (const auto& [portTo, portsFrom]: IC) {
            res += portsFrom.size();
        }
		if (childCoupled != nullptr) {
			res += childCoupled->nICs();
		}
		return res;
	}

	unsigned long DEVStoneCoupled::nEOCs() const {
        auto res = 0;
        for (const auto& [portTo, portsFrom]: EOC) {
            res += portsFrom.size();
        }
		if (childCoupled != nullptr) {
			res += childCoupled->nEOCs();
		}
		return res;
	}

	unsigned long DEVStoneCoupled::nTransitions() const {
		auto res = (childCoupled == nullptr) ? 0 : childCoupled->nTransitions();
		for (const auto& [childId, child]: components) {
			auto atomic = std::dynamic_pointer_cast<DEVStoneAtomic>(child);
			if (atomic != nullptr) {
				res += atomic->nTransitions();
			}
		}
		return res;
	}
}  //namespace cadmium::example::devstone
