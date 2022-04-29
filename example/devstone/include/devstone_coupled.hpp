#ifndef _CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HPP_
#define _CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HPP_

#include <cadmium/core/modeling/coupled.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <memory>

class DEVStoneCoupled: public cadmium::Coupled {
 protected:
	std::shared_ptr<DEVStoneCoupled> childCoupled;
 public:
	DEVStoneCoupled(int width, int depth, int intDelay, int extDelay);
	static std::shared_ptr<DEVStoneCoupled> newDEVStoneCoupled(const std::string& type, int width, int depth, int intDelay, int extDelay);
	[[nodiscard]] static cadmium::RootCoordinator createEngine(const std::shared_ptr<DEVStoneCoupled>& devstone);
	[[maybe_unused]] static void runSimulation(cadmium::RootCoordinator& rootCoordinator);

	[[nodiscard]] unsigned long nAtomics() const;
	[[nodiscard]] unsigned long nICs() const;
	[[nodiscard]] unsigned long nEICs() const;
	[[nodiscard]] unsigned long nEOCs() const;
	[[nodiscard]] unsigned long nTransitions() const;
};

#endif //_CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HPP_
