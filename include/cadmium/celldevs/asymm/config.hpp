/**
 * Structs for assisting the definition of asymmetric cell configuration parameters.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 */


#ifndef CADMIUM_CELLDEVS_ASYMM_CONFIG_HPP_
#define CADMIUM_CELLDEVS_ASYMM_CONFIG_HPP_

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include "../core/config.hpp"
#include "../core/utility.hpp"

namespace cadmium::celldevs {
	/**
	 * @brief Asymmetric cell configuration parameters
	 * @tparam S type used to represent cell states.
	 * @tparam V type used to represent vicinities between cells.
	 */
	template<typename S, typename V>
	struct AsymmCellConfig : public CellConfig<std::string, S, V> {
		using CellConfig<std::string, S, V>::rawNeighborhood;

		/**
		 * Creates a new asymmetric cell configuration structure from a JSON object.
		 * @param configId ID of the cell configuration structure.
		 * @param configParams JSON object containing all the cell configuration parameters.
		 */
		AsymmCellConfig(const std::string& configId, const nlohmann::json& configParams) : CellConfig<std::string, S, V>(configId, configParams) {}

		/**
		 * It builds a neighborhood set for a given cell in the scenario.
		 * In asymmetric Cell-DEVS, neighborhoods are provided as a map {neighbor ID: vicinity factor}.
		 * @param cellId ID of the cell that will own the neighborhood set.
		 * @return unordered map {neighbor cell ID: neighbor cell data}.
		 */
		std::unordered_map<std::string, NeighborData<S, V>> buildNeighborhood(const std::string& cellId) const override {
			return rawNeighborhood.template get<std::unordered_map<std::string, NeighborData<S, V>>>();
		}
	};
} // namespace cadmium::celldevs

#endif // CADMIUM_CELLDEVS_ASYMM_CONFIG_HPP_
