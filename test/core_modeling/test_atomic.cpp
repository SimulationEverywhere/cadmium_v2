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
	os << "<" << x.nInternals << "," << x.nExternals << "," << x.nInputs << "," << x.clock << x.sigma << ">";
	return os;
}

class DummyAtomic: public Atomic<DummyState> {
 public:
	explicit DummyAtomic(const std::string& id): Atomic<DummyState>(id, DummyState()) {
		addInPort<int>("inPort");
		addOutPort<int>("outPort");
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

bool invalidPortTypeException(const CadmiumModelException& ex) {
	BOOST_CHECK_EQUAL(ex.what(), std::string("invalid port type"));
	return true;
}

bool portNotFoundException(const CadmiumModelException& ex) {
	BOOST_CHECK_EQUAL(ex.what(), std::string("port not found"));
	return true;
}

BOOST_AUTO_TEST_CASE(AtomicTest)
{
	auto dummy1 = DummyAtomic("dummy1");
}
