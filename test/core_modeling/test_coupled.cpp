#define BOOST_TEST_MODULE CoupledTests
#include <boost/test/unit_test.hpp>
#include <cadmium/core/modeling/atomic.hpp>
#include <cadmium/core/modeling/coupled.hpp>

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

struct DummyIntAtomic: public Atomic<DummyState> {
	std::shared_ptr<Port<int>> outPort;
	explicit DummyIntAtomic(const std::string& id): Atomic<DummyState>(id, DummyState()) {
		addInPort(std::make_shared<Port<int>>("inPort"));
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

	void externalTransition(DummyState& s, double e, const PortSet& x) const override {
		s.clock += e;
		s.sigma = ++s.nExternals;
		for (const auto& inPort: x.getPorts()) {
			s.nInputs += (int) inPort->size();
		}
	}

	void output(const DummyState& s, const PortSet& y) const override {
		y.addMessage("outPort", s.nInternals);
	}

	[[nodiscard]] double timeAdvance(const DummyState& s) const override {
		return s.sigma;
	}
};

struct DummyDoubleAtomic: public Atomic<DummyState> {
	std::shared_ptr<Port<double>> outPort;
	explicit DummyDoubleAtomic(const std::string& id): Atomic<DummyState>(id, DummyState()) {
		addInPort(std::make_shared<Port<double>>("inPort"));
		outPort = addOutPort<double>("outPort");
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

	void externalTransition(DummyState& s, double e, const PortSet& x) const override {
		s.clock += e;
		s.sigma = ++s.nExternals;
		for (const auto& inPort: x.getPorts()) {
			s.nInputs += (int) inPort->size();
		}
	}

	void output(const DummyState& s, const PortSet& y) const override {
		y.addMessage("outPort", (double) s.nInternals);
	}

	[[nodiscard]] double timeAdvance(const DummyState& s) const override {
		return s.sigma;
	}
};

bool portBelongsOtherException(const CadmiumModelException& ex) {
	BOOST_CHECK_EQUAL(ex.what(), std::string("port already belongs to other component"));
	return true;
}

bool componentIdDefinedException(const CadmiumModelException& ex) {
	BOOST_CHECK_EQUAL(ex.what(), std::string("component ID already defined"));
	return true;
}

bool invalidPortTypeException(const CadmiumModelException& ex) {
	BOOST_CHECK_EQUAL(ex.what(), std::string("invalid port type"));
	return true;
}

bool noPortParentException(const CadmiumModelException& ex) {
	BOOST_CHECK_EQUAL(ex.what(), std::string("port does not belong to any model"));
	return true;
}

BOOST_AUTO_TEST_CASE(CoupledTest)
{
	auto coupled = Coupled("coupled");
	auto inPort = coupled.addInPort<int>("inPort");
	auto outPort = coupled.addOutPort<double>("outPort");

	BOOST_CHECK(coupled.inEmpty());
	BOOST_CHECK(coupled.outEmpty());
	BOOST_CHECK(coupled.containsInPort(inPort));
	BOOST_CHECK(coupled.containsOutPort(outPort));
	BOOST_CHECK_EQUAL(nullptr, coupled.getParent());
	BOOST_CHECK_EQUAL(&coupled, inPort->getParent());
	BOOST_CHECK_EQUAL(&coupled, outPort->getParent());
	BOOST_CHECK_EQUAL(coupled.getOutPort("outPort"), outPort);

	auto dummyInt1 = std::make_shared<DummyIntAtomic>("dummyInt1");
	auto dummyInt2 = std::make_shared<DummyIntAtomic>("dummyInt2");
	auto dummyDouble1 = std::make_shared<DummyDoubleAtomic>("dummyDouble1");
	auto dummyDouble2 = std::make_shared<DummyDoubleAtomic>("dummyDouble2");
	BOOST_CHECK_EQUAL(nullptr, dummyInt1->getParent());
	BOOST_CHECK_EQUAL(nullptr, dummyInt2->getParent());
	BOOST_CHECK_EQUAL(nullptr, dummyDouble1->getParent());
	BOOST_CHECK_EQUAL(nullptr, dummyDouble2->getParent());
	coupled.addComponent(dummyInt1);
	coupled.addComponent(dummyInt2);
	coupled.addComponent(dummyDouble1);
	coupled.addComponent(dummyDouble2);
	BOOST_CHECK_EQUAL(&coupled, dummyInt1->getParent());
	BOOST_CHECK_EQUAL(&coupled, dummyInt2->getParent());
	BOOST_CHECK_EQUAL(&coupled, dummyDouble1->getParent());
	BOOST_CHECK_EQUAL(&coupled, dummyDouble2->getParent());
	BOOST_CHECK_EXCEPTION(coupled.addComponent(dummyInt1), CadmiumModelException, componentIdDefinedException);
	BOOST_CHECK_EXCEPTION(coupled.addInPort(dummyInt1->outPort), CadmiumModelException, portBelongsOtherException);

	auto hangingPort = Port<int>::newPort("hangingPort");
	auto hangingDummyInt = DummyIntAtomic("hangingDummyInt");
	BOOST_CHECK_EXCEPTION(coupled.addCoupling(hangingPort, dummyDouble1->outPort), CadmiumModelException, invalidPortTypeException);
	BOOST_CHECK_EXCEPTION(coupled.addCoupling(hangingPort, dummyInt1->outPort), CadmiumModelException, noPortParentException);
	// TODO more test
}
