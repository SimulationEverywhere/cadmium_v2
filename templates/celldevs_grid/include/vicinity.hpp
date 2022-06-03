#ifndef CADMIUM_TEMPLATE_CELLDEVS_ASYMM_VICINITY_HPP_
#define CADMIUM_TEMPLATE_CELLDEVS_ASYMM_VICINITY_HPP_

#include "nlohmann/json.hpp"

/// This structure contains all the fields that belong to the vicinity factor.
struct Vicinity {
	/// The vicinity structure must have a default constructor function.
	Vicinity() {}
};

/**
 * You must implement the from_json for your vicinity structure.
 * In this way, Cadmium will be able to parse the JSON configuration file.
 * @param j JSON object representing a cell state.
 * @param v vicinity structure to be filled with the configuration shown in the JSON file.
 */
[[maybe_unused]] void from_json(const nlohmann::json& j, Vicinity& v) {}

#endif //CADMIUM_TEMPLATE_CELLDEVS_VICINITY_HPP_
