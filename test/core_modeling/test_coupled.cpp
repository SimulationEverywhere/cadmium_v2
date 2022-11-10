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
	Port<int> inPort, outPort;
	explicit DummyIntAtomic(const std::string& id): Atomic<DummyState>(id, DummyState()) {
		inPort = addInPort<int>("inPort");
		outPort = addOutPort<int>("outPort");
	}
	using Atomic<DummyState>::internalTransition;
	using Atomic<DummyState>::externalTransition;
	using Atomic<DummyState>::confluentTransition;
	using Atomic<DummyState>::output;
	using Atomic<DummyState>::timeAdvance;

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

struct DummyDoubleAtomic: public Atomic<DummyState> {
	Port<double> inPort, outPort;
	explicit DummyDoubleAtomic(const std::string& id): Atomic<DummyState>(id, DummyState()) {
		inPort = addInPort<double>("inPort");
		outPort = addOutPort<double>("outPort");
	}
	using Atomic<DummyState>::internalTransition;
	using Atomic<DummyState>::externalTransition;
	using Atomic<DummyState>::confluentTransition;
	using Atomic<DummyState>::output;
	using Atomic<DummyState>::timeAdvance;

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

bool componentNotFoundException(const CadmiumModelException& ex) {
	BOOST_CHECK_EQUAL(ex.what(), std::string("component not found"));
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

bool invalidDestinationException(const CadmiumModelException& ex) {
	BOOST_CHECK_EQUAL(ex.what(), std::string("invalid destination port"));
	return true;
}

bool invalidOriginException(const CadmiumModelException& ex) {
	BOOST_CHECK_EQUAL(ex.what(), std::string("invalid origin port"));
	return true;
}

bool duplicateCouplingException(const CadmiumModelException& ex) {
	BOOST_CHECK_EQUAL(ex.what(), std::string("duplicate coupling"));
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
	BOOST_CHECK_EQUAL(nullptr, dummyInt1->getParent());
	BOOST_CHECK_EQUAL(nullptr, dummyInt2->getParent());
	BOOST_CHECK_EQUAL(nullptr, dummyDouble1->getParent());
	coupled.addComponent(dummyInt1);
	coupled.addComponent(dummyInt2);
	coupled.addComponent(dummyDouble1);
	auto dummyDouble2 = coupled.addComponent<DummyDoubleAtomic>("dummyDouble2");
	BOOST_CHECK_EQUAL(&coupled, dummyInt1->getParent());
	BOOST_CHECK_EQUAL(&coupled, dummyInt2->getParent());
	BOOST_CHECK_EQUAL(&coupled, dummyDouble1->getParent());
	BOOST_CHECK_EQUAL(&coupled, dummyDouble2->getParent());
	BOOST_CHECK_EXCEPTION(coupled.addComponent(dummyInt1), CadmiumModelException, componentIdDefinedException);
	BOOST_CHECK_EXCEPTION(coupled.addInPort(dummyInt1->outPort), CadmiumModelException, portBelongsOtherException);

	auto hangingDummyInt = DummyIntAtomic("hangingDummyInt");
	BOOST_CHECK_EQUAL(coupled.getComponent("dummyInt1"), dummyInt1);
	BOOST_CHECK_EQUAL(coupled.getComponent("dummyInt2"), dummyInt2);
	BOOST_CHECK_EQUAL(coupled.getComponent("dummyDouble1"), dummyDouble1);
	BOOST_CHECK_EQUAL(coupled.getComponent("dummyDouble2"), dummyDouble2);
	BOOST_CHECK_EXCEPTION((void) coupled.getComponent("hangingDummyInt"), CadmiumModelException, componentNotFoundException);

	auto hangingPort = std::make_shared<_Port<int>>("hangingPort");
	BOOST_CHECK_EXCEPTION(coupled.addCoupling(hangingPort, dummyDouble1->outPort), CadmiumModelException, invalidPortTypeException);
	BOOST_CHECK_EXCEPTION(coupled.addCoupling(hangingPort, dummyInt1->outPort), CadmiumModelException, noPortParentException);
	BOOST_CHECK_EXCEPTION(coupled.addCoupling(inPort, hangingDummyInt.outPort), CadmiumModelException, invalidDestinationException);
	BOOST_CHECK_EXCEPTION(coupled.addCoupling(dummyInt1->outPort, hangingDummyInt.inPort), CadmiumModelException, invalidDestinationException);
	BOOST_CHECK_EXCEPTION(coupled.addCoupling(hangingDummyInt.outPort, dummyInt1->outPort), CadmiumModelException, invalidOriginException);

	coupled.addCoupling(inPort, dummyInt1->inPort);
	coupled.addCoupling(dummyInt1->outPort, dummyInt2->inPort);
	coupled.addCoupling(dummyDouble2->outPort, outPort);
	auto& eic = coupled.getEICs();
	auto& ic = coupled.getICs();
	auto& eoc = coupled.getEOCs();
	BOOST_CHECK_EQUAL(1, eic.size());
	BOOST_CHECK(coupled.containsCoupling(eic, inPort, dummyInt1->inPort));
	BOOST_CHECK_EQUAL(1, ic.size());
	BOOST_CHECK(coupled.containsCoupling(ic, dummyInt1->outPort, dummyInt2->inPort));
	BOOST_CHECK_EQUAL(1, eoc.size());
	BOOST_CHECK(coupled.containsCoupling(eoc, dummyDouble2->outPort, outPort));

	BOOST_CHECK_EXCEPTION(coupled.addEIC("inPort", "dummyInt1", "inPort"), CadmiumModelException, duplicateCouplingException);
	BOOST_CHECK_EXCEPTION(coupled.addIC("dummyInt1", "outPort", "dummyInt2", "inPort"), CadmiumModelException, duplicateCouplingException);
	BOOST_CHECK_EXCEPTION(coupled.addEOC("dummyDouble2", "outPort", "outPort"), CadmiumModelException, duplicateCouplingException);

	coupled.addIC("dummyDouble1", "outPort", "dummyDouble2", "inPort");
	BOOST_CHECK_EQUAL(1, eic.size());
	BOOST_CHECK(coupled.containsCoupling(eic, inPort, dummyInt1->inPort));
	BOOST_CHECK_EQUAL(2, ic.size());
	BOOST_CHECK(coupled.containsCoupling(ic, dummyInt1->outPort, dummyInt2->inPort));
	BOOST_CHECK(coupled.containsCoupling(ic, dummyDouble1->outPort, dummyDouble2->inPort));
	BOOST_CHECK_EQUAL(1, eoc.size());
	BOOST_CHECK(coupled.containsCoupling(eoc, dummyDouble2->outPort, outPort));
}
