/**
 * Cell configuration structure.
 * Copyright (C) 2021  Román Cárdenas Rodríguez
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

#ifndef CADMIUM_MODELING_CELLDEVS_CORE_CONFIG_HPP_
#define CADMIUM_MODELING_CELLDEVS_CORE_CONFIG_HPP_

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "utility.hpp"

namespace cadmium::celldevs {
	/**
	 * @brief Cell configuration structure.
	 * @tparam C type used to represent cell IDs.
	 * @tparam S type used to represent cell states.
	 * @tparam V type used to represent vicinities between cells.
	 */
	template<typename C, typename S, typename V>
	struct CellConfig {
		const std::string configId;                            //!< ID of the cell configuration.
		std::string cellModel;                                 //!< ID of the cell model. By default, it is set to "default".
		std::string delayType;                                 //!< ID of the delay type function used by the cell. By default, it is set to "inertial".
		S state;                                               //!< Initial state of the cell. By default, it is set to the default S value.
		nlohmann::json rawNeighborhood;                        //!< JSON file with information regarding neighborhoods. By default, it is set to an empty JSON object.
		nlohmann::json rawCellConfig;                          //!< JSON file with additional configuration parameters. By default, it is set to an empty JSON object.
		std::vector<std::pair<std::string, std::string>> EIC;  //!< pairs <port from, port to> that describe how to connect the outside world with the input of the cells.
		std::vector<std::string> EOC;                          //!< pairs <port from, port to> that describe how to connect the output of the cells with the outside world.

		virtual ~CellConfig() = default;

		/**
		 * It builds a neighborhood set for a given cell in the scenario.
		 * @param cellId ID of the cell that will own the neighborhood set.
		 * @return unordered map {neighbor cell ID: neighbor cell data}.
		 */
		virtual std::unordered_map<C, NeighborData<S, V>> buildNeighborhood(const C& cellId) const = 0;

		/**
		 * Creates a new cell configuration structure from a JSON object.
		 * @param configId ID of the cell configuration structure.
		 * @param configParams JSON object containing all the cell configuration parameters.
		 */
		CellConfig(std::string  configId, const nlohmann::json& configParams): configId(std::move(configId)), EIC(), EOC() {
			cellModel = (configParams.contains("model"))? configParams["model"].get<std::string>() : "default";
			delayType = (configParams.contains("delay"))? configParams["delay"].get<std::string>() : "inertial";
			state = (configParams.contains("state"))? configParams["state"].get<S>() : S();
			rawNeighborhood = (configParams.contains("neighborhood"))? configParams["neighborhood"] : nlohmann::json();
			rawCellConfig = (configParams.contains("config")) ? configParams["config"] : nlohmann::json();
			if (configParams.contains("eic")) {
				configParams["eic"].get_to(EIC);
			}
			if (configParams.contains("eoc")) {
				configParams["eoc"].get_to(EOC);
			}
		}

		//! @return true if this configuration corresponds to the default configuration.
		[[nodiscard]] inline bool isDefault() const {
			return configId == "default";
		}
	};
}  // namespace cadmium::celldevs

#endif // CADMIUM_MODELING_CELLDEVS_CORE_CONFIG_HPP_
