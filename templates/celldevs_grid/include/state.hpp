#ifndef CADMIUM_TEMPLATE_CELLDEVS_ASYMM_STATE_HPP_
#define CADMIUM_TEMPLATE_CELLDEVS_ASYMM_STATE_HPP_

#include <iostream>
#include "nlohmann/json.hpp"

/// This structure contains all the fields that belong to the cell state.
struct State {
	/// The state structure must have a default constructor function.
	State() {}
};

/**
 * You must implement the inequality operator for your cell state structure.
 * @param x first cell state.
 * @param y second cell state.
 * @return true if the first cell state is not equal to the second cell state.
 */
inline bool operator != (const State& x, const State& y) {
	return true;
}

/**
 * You must implement the insertion operator for your cell state structure.
 * @param os output stream.
 * @param s cell state.
 * @return output stream with the inserted cell state.
 */
std::ostream &operator << (std::ostream& os, const State& s) {
	return os;
}

/**
 * You must implement the from_json for your cell state structure.
 * In this way, Cadmium will be able to parse the JSON configuration file.
 * @param j JSON object representing a cell state.
 * @param s cell state structure to be filled with the configuration shown in the JSON file.
 */
[[maybe_unused]] void from_json(const nlohmann::json& j, State& s) {}

#endif //CADMIUM_TEMPLATE_CELLDEVS_ASYMM_HPP_
