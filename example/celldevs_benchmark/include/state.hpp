#ifndef CADMIUM_EXAMPLE_CELLDEVS_BENCHMARK_STATE_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_BENCHMARK_STATE_HPP_

#include <iostream>
#include <nlohmann/json.hpp>

namespace cadmium::celldevs::example::benchmark {
	//! Synthetic benchmark state.
	struct benchmarkState {
		int digit;
		//! Default constructor function. By default, cells state is 0.
		benchmarkState() : digit(0) {}
	};

	// Required for comparing states and detect any change
	inline bool operator != (const benchmarkState &x, const benchmarkState &y) {
	    return x.digit != y.digit;
	}

	// Required for printing the state of the cell
	std::ostream &operator << (std::ostream &os, const benchmarkState &x) {
	    os << "<" << x.digit <<">";
	    return os;
	}

	//! It parses a JSON file and generates the corresponding benchmark state object.
	[[maybe_unused]] void from_json(const nlohmann::json& j, benchmarkState& s) {
		j.at("digit").get_to(s.digit);
	}

}  // namespace cadmium::celldevs::example::sir

#endif //CADMIUM_EXAMPLE_CELLDEVS_SIR_STATE_HPP_
