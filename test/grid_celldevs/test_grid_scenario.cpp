#define BOOST_TEST_MODULE GridScenarioTests
#include <boost/test/unit_test.hpp>
#include <cadmium/celldevs/grid/scenario.hpp>

using namespace cadmium::celldevs;

BOOST_AUTO_TEST_CASE(basic_scenario) {
	BOOST_CHECK_THROW(GridScenario({}, {}, false), std::bad_exception);
	BOOST_CHECK_THROW(GridScenario({-1, 4}, {0, 0}, false), std::bad_exception);
	BOOST_CHECK_THROW(GridScenario({0, 4}, {0, 0}, false), std::bad_exception);
	BOOST_CHECK_THROW(GridScenario({1, 4}, {0, 0, 0}, false), std::bad_exception);
	BOOST_CHECK_THROW(GridScenario({1, 4}, {0}, false), std::bad_exception);

	BOOST_CHECK_NO_THROW(GridScenario({1}, {1}, false));
	BOOST_CHECK_NO_THROW(GridScenario({1}, {-1}, false));
	BOOST_CHECK_NO_THROW(GridScenario({1}, {-1}, true));
	BOOST_CHECK_NO_THROW(GridScenario({1, 2, 4}, {1, 0, -4}, false));
	BOOST_CHECK_NO_THROW(GridScenario({1, 2, 4}, {1, 0, -4}, true));
}

BOOST_AUTO_TEST_CASE(cell_in_scenario) {
	auto scenario = GridScenario({5, 5}, {-2, -2}, false);
	for (const auto& cell: scenario) {
		std::cout << cell << "\n";
	}
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
