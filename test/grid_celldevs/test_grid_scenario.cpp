#define BOOST_TEST_MODULE GridScenarioTests
#include <boost/test/unit_test.hpp>
#include <cadmium/celldevs/grid/scenario.hpp>

using namespace cadmium;
using namespace cadmium::celldevs;

bool invalidShapeException(const CadmiumModelException& ex) {
	BOOST_CHECK_EQUAL(ex.what(), std::string("invalid scenario shape"));
	return true;
}

bool shapeOriginMismatchException(const CadmiumModelException& ex) {
	BOOST_CHECK_EQUAL(ex.what(), std::string("shape-origin dimension mismatch"));
	return true;
}

BOOST_AUTO_TEST_CASE(basic_scenario) {
	BOOST_CHECK_EXCEPTION(GridScenario({}, {}, false), CadmiumModelException, invalidShapeException);
	BOOST_CHECK_EXCEPTION(GridScenario({-1, 4}, {0, 0}, false), CadmiumModelException, invalidShapeException);
	BOOST_CHECK_EXCEPTION(GridScenario({0, 4}, {0, 0}, false), CadmiumModelException, invalidShapeException);
	BOOST_CHECK_EXCEPTION(GridScenario({1, 4}, {0, 0, 0}, false), CadmiumModelException, shapeOriginMismatchException);
	BOOST_CHECK_EXCEPTION(GridScenario({1, 4}, {0}, false), CadmiumModelException, shapeOriginMismatchException);

	BOOST_CHECK_NO_THROW(GridScenario({1}, {1}, false));
	BOOST_CHECK_NO_THROW(GridScenario({1}, {-1}, false));
	BOOST_CHECK_NO_THROW(GridScenario({1}, {-1}, true));
	BOOST_CHECK_NO_THROW(GridScenario({1, 2, 4}, {1, 0, -4}, false));
	BOOST_CHECK_NO_THROW(GridScenario({1, 2, 4}, {1, 0, -4}, true));
}

BOOST_AUTO_TEST_CASE(cell_in_scenario) {
	auto scenario = GridScenario({5, 5}, {-2, -2}, false);

	BOOST_TEST(!scenario.cellInScenario({2}));
	BOOST_TEST(!scenario.cellInScenario({2, 2, 2}));

	BOOST_TEST(!scenario.cellInScenario({2, 3}));
	BOOST_TEST(!scenario.cellInScenario({3, 2}));
	BOOST_TEST(scenario.cellInScenario({1, 2}));
	BOOST_TEST(scenario.cellInScenario({2, 1}));
	BOOST_TEST(scenario.cellInScenario({2, 2}));

	BOOST_TEST(!scenario.cellInScenario({-2, -3}));
	BOOST_TEST(!scenario.cellInScenario({-3, -2}));
	BOOST_TEST(scenario.cellInScenario({-1, -2}));
	BOOST_TEST(scenario.cellInScenario({-2, -1}));
	BOOST_TEST(scenario.cellInScenario({-2, -2}));
}
