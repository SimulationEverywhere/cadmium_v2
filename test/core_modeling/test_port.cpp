#define BOOST_TEST_MODULE PortTests
#include <boost/test/unit_test.hpp>
#include <cadmium/core/modeling/component.hpp>

using namespace cadmium;

bool invalidPortTypeException(const CadmiumModelException& ex) {
    BOOST_CHECK_EQUAL(ex.what(), std::string("invalid port type"));
    return true;
}

BOOST_AUTO_TEST_CASE(PortTest)
{
    auto port1 = std::make_shared<_Port<int>>("port1");
    BOOST_CHECK(port1->empty());
    BOOST_CHECK_EQUAL(port1->getParent(), nullptr);

    port1->addMessage(0);
    BOOST_CHECK(!port1->empty());
    BOOST_CHECK_EQUAL(1, port1->size());
    BOOST_CHECK_EQUAL(0, port1->getBag().at(0));
    port1->addMessage((int) 1.9);
    BOOST_CHECK(!port1->empty());
    BOOST_CHECK_EQUAL(2, port1->size());
    BOOST_CHECK_EQUAL(0, port1->getBag().at(0));
    BOOST_CHECK_EQUAL(1, port1->getBag().at(1));

    BOOST_CHECK_EQUAL("0", port1->logMessage(0));
    BOOST_CHECK_EQUAL("1", port1->logMessage(1));

    auto port2 = std::make_shared<_Port<double>>("port2");
    BOOST_CHECK(port2->empty());
    BOOST_CHECK_EQUAL(port2->getParent(), nullptr);
    auto port2Interface = std::dynamic_pointer_cast<PortInterface>(port2);
    BOOST_CHECK_NE(port2Interface, nullptr);
    BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<_Port<int>>(port2Interface), nullptr);
    BOOST_CHECK(!port1->compatible(port2));
    BOOST_CHECK(!port2Interface->compatible(port1));
    BOOST_CHECK(port2Interface->compatible(port2));

    auto port3 = port1->newCompatiblePort("port3");
    BOOST_CHECK(port3->empty());
    BOOST_CHECK_EQUAL(port3->getParent(), nullptr);
    BOOST_CHECK(port3->compatible(port1));
    port3->propagate(port1);
    BOOST_CHECK(!port1->empty());
    BOOST_CHECK_EQUAL(2, port3->size());
    auto port3Casted = std::dynamic_pointer_cast<_Port<int>>(port3);
    BOOST_CHECK_EQUAL(0, port3Casted->getBag().at(0));
    BOOST_CHECK_EQUAL(1, port3Casted->getBag().at(1));

    port1->clear();
    BOOST_CHECK(port1->empty());
    BOOST_CHECK(!port3->empty());
    port3Casted->clear();
    BOOST_CHECK(port3->empty());
}
