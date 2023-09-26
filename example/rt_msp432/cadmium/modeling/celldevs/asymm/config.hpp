/**
 * Structs for assisting the definition of asymmetric cell configuration parameters.
 * Copyright (C) 2022  Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CADMIUM_MODELING_CELLDEVS_ASYMM_CONFIG_HPP_
#define CADMIUM_MODELING_CELLDEVS_ASYMM_CONFIG_HPP_

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

#endif // CADMIUM_MODELING_CELLDEVS_ASYMM_CONFIG_HPP_
