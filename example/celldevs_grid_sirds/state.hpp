//
// Created by Román Cárdenas Rodríguez on 3/1/22.
//

#ifndef CADMIUM_EXAMPLE_CELLDEVS_GRID_SIRD_STATE_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_GRID_SIRD_STATE_HPP_

#include <iostream>
#include <nlohmann/json.hpp>

struct sird {
	unsigned int population;    /// Number of individuals that live in the cell
	double susceptible;          /// Percentage (from 0 to 1) of people that are susceptible to the disease
	double infected;             /// Percentage (from 0 to 1) of people that are infected
	double recovered;            /// Percentage (from 0 to 1) of people that already recovered from the disease
	double deceased;             /// Percentage (from 0 to 1) of people that deceased due to the disease
	sird() : population(0), susceptible(1), infected(0), recovered(0), deceased(0) {}  // a default constructor is required
	sird(unsigned int pop, double s, double i, double r, double d) : population(pop), susceptible(s), infected(i), recovered(r), deceased(d) {}
};

/**
 * We need to implement the != operator for the desired cell state struct.
 * Otherwise, Cadmium will not be able to detect a state change and work properly
 * @param x first state struct to compare
 * @param y second state struct to compare
 * @return true if x and y contain different data
 */
inline bool operator != (const sird &x, const sird &y) {
	return x.population != y.population ||
		x.susceptible != y.susceptible || x.infected != y.infected ||
		x.recovered != y.recovered || x.deceased != y.deceased;
}

/**
 * We need to implement the << operator for the desired cell state struct.
 * Otherwise, Cadmium will not be able to print the cell state in the output log file
 * @param os output stream (usually, the log file)
 * @param x state struct to print
 * @return the output stream with the cell state already printed
 */
std::ostream &operator << (std::ostream &os, const sird &x) {
	os << "<" << x.population << "," << x.susceptible << "," << x.infected << "," << x.recovered << "," << x.deceased << ">";
	return os;
}

/**
 * We need to implement the from_json method for the desired cell state struct.
 * Otherwise, Cadmium will not be able to understand the JSON configuration file.
 * @param j Chunk of JSON file that represents a cell state
 * @param s cell state struct to be filled with the configuration shown in the JSON file.
 */
[[maybe_unused]] void from_json(const nlohmann::json& j, sird &s) {
	j.at("population").get_to(s.population);
	j.at("susceptible").get_to(s.susceptible);
	j.at("infected").get_to(s.infected);
	j.at("recovered").get_to(s.recovered);
	j.at("deceased").get_to(s.deceased);
}

#endif //CADMIUM_EXAMPLE_CELLDEVS_GRID_SIRD_STATE_HPP_
