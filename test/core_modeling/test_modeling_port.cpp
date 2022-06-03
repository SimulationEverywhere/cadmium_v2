#define BOOST_TEST_MODULE PortTests
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(FailTest)
{
    BOOST_CHECK_EQUAL(5, 5);
}
