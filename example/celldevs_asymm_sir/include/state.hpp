#ifndef CADMIUM_EXAMPLE_CELLDEVS_ASYMM_SIR_STATE_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_ASYMM_SIR_STATE_HPP_

#include <iostream>
#include <nlohmann/json.hpp>

struct SIR {
	int p;     // population
	double s;  // susceptible ratio
	double i;  // infected ratio
	double r;  // recovered ratio
	SIR(): p(0), s(1), i(0), r(0) {}
};

inline bool operator != (const SIR& x, const SIR& y) {
	return x.p != y.p || x.s != y.s || x.i != y.i || x.r != y.r;
}

std::ostream &operator << (std::ostream& os, const SIR& x) {
	os << "<" << x.p << "," << x.s << "," << x.i << "," << x.r << ">";
	return os;
}

[[maybe_unused]] void from_json(const nlohmann::json& j, SIR& s) {
	j.at("p").get_to(s.p);
	j.at("s").get_to(s.s);
	j.at("i").get_to(s.i);
	j.at("r").get_to(s.r);
}

#endif //CADMIUM_EXAMPLE_CELLDEVS_ASYMM_SIR_STATE_HPP_
