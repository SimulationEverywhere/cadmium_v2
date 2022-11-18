#define BOOST_TEST_MODULE DEVStoneTests
#include <boost/test/unit_test.hpp>
#include <string>
#include "../../example/devstone/include/devstone.hpp"

#define STEP 5
#define MAX_WIDTH 50
#define MAX_DEPTH 50

using namespace cadmium::example::devstone;

int expectedAtomics(const std::string& type, int width, int depth) {
	auto wFactor = width - 1;
	if (type == "HOmod") {
		wFactor += (width - 1) * width / 2;
	}
	return wFactor * (depth - 1) + 1;
}

int expectedEICs(const std::string& type, int width, int depth) {
	auto wFactor = width;
	if (type == "HO") {
		wFactor = width + 1;
	} else if (type == "HOmod") {
		wFactor = 2 * (width - 1) + 1;
	}
	return wFactor * (depth - 1) + 1;
}

int expectedICs(const std::string& type, int width, int depth) {
	auto wFactor = 0;
	if (width > 2 && (type == "HI" || type == "HO")) {
		wFactor = (width - 2);
	} else if (type == "HOmod") {
		wFactor = (width - 1) * (width - 1) + (width - 1) * width / 2;
	}
	return wFactor * (depth - 1);
}

int expectedEOCs(const std::string& type, int width, int depth) {
	return (type == "HO")? width * (depth - 1) + 1 : depth;
}

int expectedInternals(const std::string& type, int width, int depth) {
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

int expectedExternals(const std::string& type, int width, int depth) {
    return expectedInternals(type, width, depth);
}

int expectedEvents(const std::string& type, int width, int depth) {
    if (type != "HOmod") {
        return expectedInternals(type, width, depth);
    }
    auto n = 1;
    if (width > 1 && depth > 1) {
        n += 2 * (width - 1);
        auto aux = 0;
        for (int i = 2; i < depth; ++i) {
            aux += 1 + (i - 1) * (width - 1);
        }
        n += aux * 2 * (width - 1) * (width - 1);
        n += (aux + 1) * ((width - 1) * (width - 1) + (width - 2) * (width - 1) / 2);
    }
    return n;
}

cadmium::RootCoordinator createEngine(const std::shared_ptr<DEVStone>& devstone) {
	auto rootCoordinator = cadmium::RootCoordinator(devstone);
	rootCoordinator.start();
	return rootCoordinator;
}

BOOST_AUTO_TEST_CASE(DEVStoneLI)
{
	for (int w = 1; w <= MAX_WIDTH; w += STEP) {
		for (int d = 1; d <= MAX_DEPTH; d += STEP) {
			auto coupled = std::make_shared<DEVStone>("LI", w, d, 0, 0);
            BOOST_CHECK_EQUAL(coupled->nAtomics(), expectedAtomics("LI", w, d));
			BOOST_CHECK_EQUAL(coupled->nEICs(), expectedEICs("LI", w, d));
			BOOST_CHECK_EQUAL(coupled->nICs(), expectedICs("LI", w, d));
			BOOST_CHECK_EQUAL(coupled->nEOCs(), expectedEOCs("LI", w, d));
			auto coordinator = createEngine(coupled);
            coordinator.simulate(std::numeric_limits<double>::infinity());
			BOOST_CHECK_EQUAL(coupled->nInternals(), expectedInternals("LI", w, d));
            BOOST_CHECK_EQUAL(coupled->nExternals(), expectedExternals("LI", w, d));
            BOOST_CHECK_EQUAL(coupled->nEvents(), expectedEvents("LI", w, d));
		}
	}
}

BOOST_AUTO_TEST_CASE(DEVStoneHI)
{
	for (int w = 1; w <= MAX_WIDTH; w += STEP) {
		for (int d = 1; d <= MAX_DEPTH; d += STEP) {
			auto coupled = std::make_shared<DEVStone>("HI", w, d, 0, 0);
            BOOST_CHECK_EQUAL(coupled->nAtomics(), expectedAtomics("HI", w, d));
			BOOST_CHECK_EQUAL(coupled->nEICs(), expectedEICs("HI", w, d));
			BOOST_CHECK_EQUAL(coupled->nICs(), expectedICs("HI", w, d));
			BOOST_CHECK_EQUAL(coupled->nEOCs(), expectedEOCs("HI", w, d));
			auto coordinator = createEngine(coupled);
            coordinator.simulate(std::numeric_limits<double>::infinity());
			BOOST_CHECK_EQUAL(coupled->nInternals(), expectedInternals("HI", w, d));
            BOOST_CHECK_EQUAL(coupled->nExternals(), expectedExternals("HI", w, d));
            BOOST_CHECK_EQUAL(coupled->nEvents(), expectedEvents("HI", w, d));
		}
	}
}

BOOST_AUTO_TEST_CASE(DEVStoneHO)
{
	for (int w = 1; w <= MAX_WIDTH; w += STEP) {
		for (int d = 1; d <= MAX_DEPTH; d += STEP) {
			auto coupled = std::make_shared<DEVStone>("HO", w, d, 0, 0);
            BOOST_CHECK_EQUAL(coupled->nAtomics(), expectedAtomics("HO", w, d));
			BOOST_CHECK_EQUAL(coupled->nEICs(), expectedEICs("HO", w, d));
			BOOST_CHECK_EQUAL(coupled->nICs(), expectedICs("HO", w, d));
			BOOST_CHECK_EQUAL(coupled->nEOCs(), expectedEOCs("HO", w, d));
			auto coordinator = createEngine(coupled);
            coordinator.simulate(std::numeric_limits<double>::infinity());
			BOOST_CHECK_EQUAL(coupled->nInternals(), expectedInternals("HO", w, d));
            BOOST_CHECK_EQUAL(coupled->nExternals(), expectedExternals("HO", w, d));
            BOOST_CHECK_EQUAL(coupled->nEvents(), expectedEvents("HO", w, d));
		}
	}
}

BOOST_AUTO_TEST_CASE(DEVStoneHOmod)
{
	for (int w = 1; w <= MAX_WIDTH / 5; w += STEP / 5) {
		for (int d = 1; d <= MAX_DEPTH / 5; d += STEP / 5) {
			auto coupled = std::make_shared<DEVStone>("HOmod", w, d, 0, 0);
            BOOST_CHECK_EQUAL(coupled->nAtomics(), expectedAtomics("HOmod", w, d));
			BOOST_CHECK_EQUAL(coupled->nEICs(), expectedEICs("HOmod", w, d));
			BOOST_CHECK_EQUAL(coupled->nICs(), expectedICs("HOmod", w, d));
			BOOST_CHECK_EQUAL(coupled->nEOCs(), expectedEOCs("HOmod", w, d));
			auto coordinator = createEngine(coupled);
            coordinator.simulate(std::numeric_limits<double>::infinity());
			BOOST_CHECK_EQUAL(coupled->nInternals(), expectedInternals("HOmod", w, d));
            BOOST_CHECK_EQUAL(coupled->nExternals(), expectedExternals("HOmod", w, d));
            BOOST_CHECK_EQUAL(coupled->nEvents(), expectedEvents("HOmod", w, d));
		}
	}
}
