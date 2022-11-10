#define BOOST_TEST_MODULE AtomicTests
#include <boost/test/unit_test.hpp>
#include <cadmium/core/modeling/atomic.hpp>

using namespace cadmium;

struct DummyState {
	int nInternals, nExternals, nInputs;
	double clock, sigma;
	DummyState(): nInternals(), nExternals(), nInputs(), clock(), sigma() {}
};

std::ostream &operator << (std::ostream& os, const DummyState& x) {
	os << "<" << x.nInternals << "," << x.nExternals << "," << x.nInputs << "," << x.clock << "," << x.sigma << ">";
	return os;
}

struct DummyAtomic: public Atomic<DummyState> {
    Port<int> inPort;
	Port<int> outPort;
	explicit DummyAtomic(const std::string& id): Atomic<DummyState>(id, DummyState()) {
		inPort = addInPort<int>("inPort");
		outPort = addOutPort<int>("outPort");
	}
	using Atomic<DummyState>::internalTransition;
	using Atomic<DummyState>::externalTransition;
	using Atomic<DummyState>::confluentTransition;
	using Atomic<DummyState>::output;
	using Atomic<DummyState>::timeAdvance;

	const DummyState& getState() {
		return state;
	}

	void internalTransition(DummyState& s) const override {
		s.clock += s.sigma;
		s.sigma = ++s.nInternals;
	}

	void externalTransition(DummyState& s, double e) const override {
		s.clock += e;
		s.sigma = ++s.nExternals;
        s.nInputs += (int) inPort->size();
	}

	void output(const DummyState& s) const override {
		outPort->addMessage(s.nInternals);
	}

	[[nodiscard]] double timeAdvance(const DummyState& s) const override {
		return s.sigma;
	}
};

bool portBelongsOtherException(const CadmiumModelException& ex) {
	BOOST_CHECK_EQUAL(ex.what(), std::string("port already belongs to other component"));
	return true;
}

BOOST_AUTO_TEST_CASE(AtomicTest)
{
	auto dummy1 = DummyAtomic("dummy1");
	BOOST_CHECK(dummy1.inEmpty());
	BOOST_CHECK(dummy1.outEmpty());
	auto inPort = std::dynamic_pointer_cast<_Port<int>>(dummy1.getInPort("inPort"));
	auto outPort = dummy1.getOutPort<int>("outPort");
	BOOST_CHECK(dummy1.containsInPort(inPort));
	BOOST_CHECK(dummy1.containsOutPort(outPort));
	BOOST_CHECK_EQUAL(nullptr, dummy1.getParent());
	BOOST_CHECK_EQUAL(&dummy1, inPort->getParent());
	BOOST_CHECK_EQUAL(&dummy1, outPort->getParent());
	BOOST_CHECK_EQUAL(dummy1.outPort, outPort);

	auto dummy2 = DummyAtomic("dummy2");
	BOOST_CHECK_EXCEPTION(dummy2.addInPort(inPort), CadmiumModelException, portBelongsOtherException);

	auto state = dummy1.getState();
	BOOST_CHECK_EQUAL(0, state.nInternals);
	BOOST_CHECK_EQUAL(0, state.nExternals);
	BOOST_CHECK_EQUAL(0, state.nInputs);
	BOOST_CHECK_EQUAL(0., state.clock);
	BOOST_CHECK_EQUAL(0., state.sigma);

	dummy1.output();
	BOOST_CHECK(!dummy1.outEmpty());
	BOOST_CHECK_EQUAL(1, outPort->size());
	BOOST_CHECK_EQUAL(0, outPort->getBag().at(0));
	state = dummy1.getState();
	BOOST_CHECK_EQUAL(0, state.nInternals);
	BOOST_CHECK_EQUAL(0, state.nExternals);
	BOOST_CHECK_EQUAL(0, state.nInputs);
	BOOST_CHECK_EQUAL(0., state.clock);
	BOOST_CHECK_EQUAL(0., state.sigma);
	dummy1.internalTransition();
	state = dummy1.getState();
	BOOST_CHECK_EQUAL(1, state.nInternals);
	BOOST_CHECK_EQUAL(0, state.nExternals);
	BOOST_CHECK_EQUAL(0, state.nInputs);
	BOOST_CHECK_EQUAL(0., state.clock);
	BOOST_CHECK_EQUAL(1., state.sigma);
	dummy1.clearPorts();
	BOOST_CHECK(dummy1.outEmpty());

	dummy1.output();
	BOOST_CHECK(!dummy1.outEmpty());
	BOOST_CHECK_EQUAL(1, outPort->size());
	BOOST_CHECK_EQUAL(1, outPort->getBag().at(0));
	state = dummy1.getState();
	BOOST_CHECK_EQUAL(1, state.nInternals);
	BOOST_CHECK_EQUAL(0, state.nExternals);
	BOOST_CHECK_EQUAL(0, state.nInputs);
	BOOST_CHECK_EQUAL(0., state.clock);
	BOOST_CHECK_EQUAL(1., state.sigma);
	dummy1.internalTransition();
	state = dummy1.getState();
	BOOST_CHECK_EQUAL(2, state.nInternals);
	BOOST_CHECK_EQUAL(0, state.nExternals);
	BOOST_CHECK_EQUAL(0, state.nInputs);
	BOOST_CHECK_EQUAL(1., state.clock);
	BOOST_CHECK_EQUAL(2., state.sigma);
	dummy1.clearPorts();
	BOOST_CHECK(dummy1.outEmpty());

	dummy1.externalTransition(0.);
	state = dummy1.getState();
	BOOST_CHECK_EQUAL(2, state.nInternals);
	BOOST_CHECK_EQUAL(1, state.nExternals);
	BOOST_CHECK_EQUAL(0, state.nInputs);
	BOOST_CHECK_EQUAL(1., state.clock);
	BOOST_CHECK_EQUAL(1., state.sigma);

	inPort->addMessage(0);
	BOOST_CHECK(!dummy1.inEmpty());
	dummy1.externalTransition(0.5);
	state = dummy1.getState();
	BOOST_CHECK_EQUAL(2, state.nInternals);
	BOOST_CHECK_EQUAL(2, state.nExternals);
	BOOST_CHECK_EQUAL(1, state.nInputs);
	BOOST_CHECK_EQUAL(1.5, state.clock);
	BOOST_CHECK_EQUAL(2., state.sigma);

	dummy1.output();
	BOOST_CHECK(!dummy1.outEmpty());
	BOOST_CHECK_EQUAL(1, outPort->size());
	BOOST_CHECK_EQUAL(2, outPort->getBag().at(0));
	inPort->addMessage(1);
	BOOST_CHECK(!dummy1.inEmpty());
	dummy1.confluentTransition(2.);
	state = dummy1.getState();
	BOOST_CHECK_EQUAL(3, state.nInternals);
	BOOST_CHECK_EQUAL(3, state.nExternals);
	BOOST_CHECK_EQUAL(3, state.nInputs);
	BOOST_CHECK_EQUAL(3.5, state.clock);
	BOOST_CHECK_EQUAL(3., state.sigma);
	BOOST_CHECK_EQUAL(dummy1.logState(), "<3,3,3,3.5,3>");
}
