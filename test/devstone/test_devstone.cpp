#define BOOST_TEST_MODULE DEVStoneTests
#include <boost/test/unit_test.hpp>
#include <string>
#include "../../example/devstone/include/devstone_coupled.hpp"

#define STEP 5
#define MAX_WIDTH 50
#define MAX_DEPTH 50

using namespace cadmium::example::devstone;

unsigned long expectedAtomics(const std::string& type, int width, int depth) {
	auto wFactor = width - 1;
	if (type == "HOmod") {
		wFactor += (width - 1) * width / 2;
	}
	return wFactor * (depth - 1) + 1;
}

unsigned long expectedEICs(const std::string& type, int width, int depth) {
	auto wFactor = width;
	if (type == "HO") {
		wFactor = width + 1;
	} else if (type == "HOmod") {
		wFactor = 2 * (width - 1) + 1;
	}
	return wFactor * (depth - 1) + 1;
}

unsigned long expectedICs(const std::string& type, int width, int depth) {
	auto wFactor = 0;
	if (width > 2 && (type == "HI" || type == "HO")) {
		wFactor = (width - 2);
	} else if (type == "HOmod") {
		wFactor = (width - 1) * (width - 1) + (width - 1) * width / 2;
	}
	return wFactor * (depth - 1);
}

unsigned long expectedEOCs(const std::string& type, int width, int depth) {
	return (type == "HO")? width * (depth - 1) + 1 : depth;
}

unsigned long expectedTransitions(const std::string& type, int width, int depth) {
	if (type == "LI") {
		return (width - 1) * (depth - 1) + 1;
	} else if (type == "HI" || type == "HO") {
		return (width - 1) * width / 2 * (depth - 1) + 1;
	}
	auto n = 1;
	for (int d = 1; d < depth; ++d) {
		n += (1 + (d - 1) * (width - 1)) * (width - 1) * width / 2 + (width - 1) * (width + (d - 1) * (width - 1));
	}
	return n;
}

cadmium::RootCoordinator createEngine(const std::shared_ptr<DEVStoneCoupled>& devstone) {
	auto rootCoordinator = cadmium::RootCoordinator(devstone);
	rootCoordinator.start();
	return rootCoordinator;
}

[[maybe_unused]] void runSimulation(cadmium::RootCoordinator& rootCoordinator) {
	for (const auto& inPort: rootCoordinator.getTopCoordinator()->getComponent()->getInPorts()) {
		rootCoordinator.getTopCoordinator()->inject(0, std::dynamic_pointer_cast<cadmium::_Port<int>>(inPort), -1);
	}
	rootCoordinator.simulate(std::numeric_limits<double>::infinity());
}

BOOST_AUTO_TEST_CASE(DEVStoneLI)
{
	for (int w = 1; w <= MAX_WIDTH; w += STEP) {
		for (int d = 1; d <= MAX_DEPTH; d += STEP) {
			auto coupled = DEVStoneCoupled::newDEVStoneCoupled("LI", w, d, 0, 0);
			BOOST_CHECK_EQUAL(coupled->nEICs(), expectedEICs("LI", w, d));
			BOOST_CHECK_EQUAL(coupled->nICs(), expectedICs("LI", w, d));
			BOOST_CHECK_EQUAL(coupled->nEOCs(), expectedEOCs("LI", w, d));
			auto coordinator = createEngine(coupled);
			runSimulation(coordinator);
			BOOST_CHECK_EQUAL(coupled->nTransitions(), expectedTransitions("LI", w, d));
		}
	}
}

BOOST_AUTO_TEST_CASE(DEVStoneHI)
{
	for (int w = 1; w <= MAX_WIDTH; w += STEP) {
		for (int d = 1; d <= MAX_DEPTH; d += STEP) {
			auto coupled = DEVStoneCoupled::newDEVStoneCoupled("HI", w, d, 0, 0);
			BOOST_CHECK_EQUAL(coupled->nEICs(), expectedEICs("HI", w, d));
			BOOST_CHECK_EQUAL(coupled->nICs(), expectedICs("HI", w, d));
			BOOST_CHECK_EQUAL(coupled->nEOCs(), expectedEOCs("HI", w, d));
			auto coordinator = createEngine(coupled);
			runSimulation(coordinator);
			BOOST_CHECK_EQUAL(coupled->nTransitions(), expectedTransitions("HI", w, d));
		}
	}
}

BOOST_AUTO_TEST_CASE(DEVStoneHO)
{
	for (int w = 1; w <= MAX_WIDTH; w += STEP) {
		for (int d = 1; d <= MAX_DEPTH; d += STEP) {
			auto coupled = DEVStoneCoupled::newDEVStoneCoupled("HO", w, d, 0, 0);
			BOOST_CHECK_EQUAL(coupled->nEICs(), expectedEICs("HO", w, d));
			BOOST_CHECK_EQUAL(coupled->nICs(), expectedICs("HO", w, d));
			BOOST_CHECK_EQUAL(coupled->nEOCs(), expectedEOCs("HO", w, d));
			auto coordinator = createEngine(coupled);
			runSimulation(coordinator);
			BOOST_CHECK_EQUAL(coupled->nTransitions(), expectedTransitions("HO", w, d));
		}
	}
}

BOOST_AUTO_TEST_CASE(DEVStoneHOmod)
{
	for (int w = 1; w <= MAX_WIDTH / 5; w += STEP / 5) {
		for (int d = 1; d <= MAX_DEPTH / 5; d += STEP / 5) {
			auto coupled = DEVStoneCoupled::newDEVStoneCoupled("HOmod", w, d, 0, 0);
			BOOST_CHECK_EQUAL(coupled->nEICs(), expectedEICs("HOmod", w, d));
			BOOST_CHECK_EQUAL(coupled->nICs(), expectedICs("HOmod", w, d));
			BOOST_CHECK_EQUAL(coupled->nEOCs(), expectedEOCs("HOmod", w, d));
			auto coordinator = createEngine(coupled);
			runSimulation(coordinator);
			BOOST_CHECK_EQUAL(coupled->nTransitions(), expectedTransitions("HOmod", w, d));
		}
	}
}
