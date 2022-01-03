//
// Created by Román Cárdenas Rodríguez on 3/1/22.
//

#ifndef CADMIUM_EXAMPLE_CELLDEVS_GRID_SIRD_VICINITY_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_GRID_SIRD_VICINITY_HPP_

#include <iostream>
#include <nlohmann/json.hpp>

struct mc {
	float connectivity;     /// Connectivity factor from 0 to 1 (i.e. how easy it is to move from one cell to another)
	float mobility;         /// Mobility factor from 0 to 1 (i.e. percentage of people that go from one cell to another)
	mc() : connectivity(0), mobility(0) {}  // a default constructor is required
	mc(float c, float m) : connectivity(c), mobility(m) {}
};

/**
 * We need to implement the from_json method for the desired cells vicinity struct.
 * Otherwise, Cadmium will not be able to understand the JSON configuration file.
 * @param j Chunk of JSON file that represents a cell state
 * @param v cells vicinity struct to be filled with the configuration shown in the JSON file.
 */
[[maybe_unused]] void from_json(const nlohmann::json& j, mc &v) {
	j.at("connectivity").get_to(v.connectivity);
	j.at("mobility").get_to(v.mobility);
}

#endif //CADMIUM_EXAMPLE_CELLDEVS_GRID_SIRD_VICINITY_HPP_
