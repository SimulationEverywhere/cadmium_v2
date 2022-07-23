#ifndef CADMIUM_EXAMPLE_CELLDEVS_SIR_STATE_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_SIR_STATE_HPP_

#include <iostream>
#include <nlohmann/json.hpp>

namespace cadmium::celldevs::example::sir {
	//! Susceptible-Infected-Recovered state.
	struct SIRState {
		int p;     //!< Cell population.
		double s;  //!< Ratio of susceptible people (from 0 to 1).
		double i;  //!< Ratio of infected people (from 0 to 1).
		double r;  //!< Ratio of recovered people (from 0 to 1).

		//! Default constructor function. By default, cells are unoccupied and all the population is considered susceptible.
		SIRState() : p(0), s(1), i(0), r(0) {
		}
	};

	//! It returns true if x and y are different.
	inline bool operator!=(const SIRState& x, const SIRState& y) {
		return x.p != y.p || x.s != y.s || x.i != y.i || x.r != y.r;
	}

	//! It prints a SIR state in an output stream.
	std::ostream& operator<<(std::ostream& os, const SIRState& x) {
		os << "<" << x.p << "," << x.s << "," << x.i << "," << x.r << ">";
		return os;
	}

	//! It parses a JSON file and generates the corresponding SIR state object.
	[[maybe_unused]] void from_json(const nlohmann::json& j, SIRState& s) {
		j.at("p").get_to(s.p);
		j.at("s").get_to(s.s);
		j.at("i").get_to(s.i);
		j.at("r").get_to(s.r);
	}
}  // namespace cadmium::celldevs::example::sir

#endif //CADMIUM_EXAMPLE_CELLDEVS_SIR_STATE_HPP_
