//
// Created by Román Cárdenas Rodríguez on 3/1/22.
//

#ifndef CADMIUM_EXAMPLE_CELLDEVS_GRID_SIRD_CELLS_SIR_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_GRID_SIRD_CELLS_SIR_HPP_

#include <cmath>
#include <nlohmann/json.hpp>
#include <cadmium/celldevs/grid/cell.hpp>
#include <cadmium/celldevs/grid/config.hpp>
#include "../state.hpp"
#include "../vicinity.hpp"


/// Configuration for SIRDS cell model for Cadmium Cell-DEVS
struct SIRDSCellConfig {
	double virulence;    /// in this example, virulence is provided using a configuration structure
	double recovery;     /// in this example, recovery is provided using a configuration structure
	double immunity;     /// in this example, immunity is provided using a configuration structure
	double fatality;     /// in this example, fatality is provided using a configuration structure
};

/**
 * We need to implement the from_json method for the desired cell configuration struct.
 * Otherwise, Cadmium will not be able to understand the JSON configuration file.
 * @param j Chunk of JSON file that represents a cell configuration
 * @param c cell configuration struct to be filled with the configuration shown in the JSON file.
 */
void from_json(const nlohmann::json& j, SIRDSCellConfig& c) {
	j.at("virulence").get_to(c.virulence);
	j.at("recovery").get_to(c.recovery);
	j.at("immunity").get_to(c.immunity);
	j.at("fatality").get_to(c.fatality);
}

/**
 * Basic Susceptible-Infected-Recovered-Deceased model for Cadmium Cell-DEVS
 * SIRDCell inherits the grid_cell class. Cell state uses the sir struct, and vicinities the mc struct.
 */
 class SIRDSCell : public cadmium::celldevs::GridCell<sird, mc> {
 public:
	SIRDSCellConfig sirdsConfig;  /// in this example, virulence is fixed. It is 0.6

	 SIRDSCell(const std::vector<int>& id, const std::shared_ptr<cadmium::celldevs::GridCellConfig<sird, mc>>& config):
		 cadmium::celldevs::GridCell<sird, mc>(id, config) {
		 config->cellConfig.get_to(sirdsConfig);
	 }

	/**
	 * We have to override the local_computation method to specify how the cell state changes according to our model.
	 * Remember: the local_computation function CANNOT change any attribute of the cell object (it is a constant method)
	 *           the local_computation function must return the state that the cell should have according to its current state and the neighbors' latest published state.
	 * IMPORTANT: this function does not set the new state of the cell. It just says which state should have the cell. The Cadmium simulator will change the state when it applies
	 * IMPORTANT: neighbor cells' state ARE JUST COPIES of their latest published state. You cannot change a neighbor cell state.
	 * IMPORTANT: neighbor cells' latest published state MAY NOT BE the neighbor cells' current state.
	 * @return the new state that the cell should have
	 */
	[[nodiscard]] sird localComputation(sird state, const std::unordered_map<std::vector<int>, cadmium::celldevs::NeighborData<sird, mc>>& neighborhood, const cadmium::PortSet& x) const override {
		auto newI = newInfections(state, neighborhood);  // to compute the percentage of new infections, we implement an auxiliary method.
		auto newR = newRecoveries(state);  // to compute the percentage of new recovered people, we implement an auxiliary method
		auto newD = newDeceases(state);
		auto newS = newSusceptibles(state);

		// We just want two decimals in the percentage -> let's round the current outcome:
		state.deceased = std::round((state.deceased + newD) * 100) / 100;
		state.recovered = std::round((state.recovered + newR - newS) * 100) / 100;
		state.infected = std::round((state.infected + newI - newR - newD) * 100) / 100;
		state.susceptible = 1 - state.infected - state.recovered - state.deceased;
		// We return the new state that the cell should have (remember, it is not yet the cell's state)
		return state;
	}

	 /**
	  * Output delay function. Any state change will be outputted after waiting 1 unit of time.
	  * @param state  new cell state.
	  * @return simulation time to wait before outputting a message with the new cell state.
	  */
	 [[nodiscard]] double outputDelay(const sird& state) const override {
		 return 1.;
	 }

	/**
	 * Auxiliary method to compute the percentage of new infections. This method MUST be constant. Otherwise, it won't compile
	 * @param state current state of the cell
	 * @return percentage of new infections
	 */
	[[nodiscard]] double newInfections(const sird& state, const std::unordered_map<std::vector<int>, cadmium::celldevs::NeighborData<sird, mc>>& neighborhood) const {
		double aux = 0;
		for(const auto& [neighborId, neighborData]: neighborhood) {
			auto s = neighborData.state;
			auto v = neighborData.vicinity;
			aux += s->infected * (double) s->population * v.mobility * v.connectivity;
		}
		return std::min(state.susceptible, state.susceptible * sirdsConfig.virulence * aux / (double) state.population);
	}

	/**
	 * Auxiliary method to compute the percentage of new recoveries. This method MUST be constant. Otherwise, it won't compile
	 * @param state current state of the cell
	 * @return percentage of new recoveries
	 */
	[[nodiscard]] double newRecoveries(const sird& state) const {
		return state.infected * sirdsConfig.recovery;
	}

	 /**
	   * Auxiliary method to compute the percentage of new deceases. This method MUST be constant. Otherwise, it won't compile
	   * @param state current state of the cell
	   * @return percentage of new deceases
	   */
	 [[nodiscard]] double newDeceases(const sird& state) const {
		 return state.infected * sirdsConfig.fatality;
	 }

	 /**
	  * Auxiliary method to compute the percentage of new susceptible people. This method MUST be constant. Otherwise, it won't compile
	  * @param state current state of the cell
	  * @return percentage of new susceptible people
	  */
	 [[nodiscard]] double newSusceptibles(const sird& state) const {
		 return state.recovered * (1 - sirdsConfig.immunity);
	 }
};

#endif //CADMIUM_EXAMPLE_CELLDEVS_GRID_SIRD_CELLS_SIR_HPP_
