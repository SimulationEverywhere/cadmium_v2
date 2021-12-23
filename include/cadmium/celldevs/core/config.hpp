/**
 * Struct for processing cell configuration parameters from a JSON file.
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

#ifndef _CADMIUM_CELLDEVS_CORE_CONFIG_HPP_
#define _CADMIUM_CELLDEVS_CORE_CONFIG_HPP_

#include "nlohmann/json.hpp"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "cadmium/celldevs/utils.hpp"

namespace cadmium::celldevs {
	/**
	 * Cell configuration structure.
	 * @tparam C type used to represent cell IDs.
	 * @tparam S type used to represent cell states.
	 * @tparam V type used to represent vicinities between cells.
	 */
	template<typename C, typename S, typename V>
	struct CellConfig {
		std::string configId;                                   /// ID of the cell configuration.
		std::string cellModel;                                  /// ID of the cell model.
		std::string delayType;                                  /// ID of the cell configuration.
		S state;                                                /// Initial state of the cell.
		nlohmann::json rawNeighborhood;                         /// JSON file with information regarding neighborhoods
		nlohmann::json cellConfig;                              /// JSON file with additional configuration parameters.
		std::vector<C> cellMap;                                 ///
		std::vector<std::pair<std::string, std::string>> EIC;  ///
		std::vector<std::pair<std::string, std::string>> EOC;  ///

		CellConfig() = default;
		virtual ~CellConfig() = default;

		CellConfig(const std::string& configId, const nlohmann::json& configParams): configId(configId), EIC(), EOC() {
			cellModel = (configParams.contains("model"))? configParams["model"].get<std::string>() : "default";
			delayType = (configParams.contains("delay"))? configParams["delay"].get<std::string>() : "inertial";
			state = (configParams.contains("state"))? configParams["state"].get<S>() : S();
			rawNeighborhood = (configParams.contains("neighborhood"))? configParams["neighborhood"] : nlohmann::json::array();
			cellConfig = (configParams.contains("config"))? configParams["config"] : nlohmann::json();
			cellMap = (!isDefault() && configParams.contains("cell_map")) ? configParams["cell_map"].get<std::vector<C>>() : std::vector<C>();
			if (configParams.contains("eic")) {
				EIC = configParams["eic"].get<std::vector<std::pair<std::string, std::string>>>();
			}
			if (configParams.contains("eoc")) {
				EOC = configParams["eoc"].get<std::vector<std::pair<std::string, std::string>>>();
			}
		}

		inline bool isDefault() const {
			return configId == "default";
		}

		virtual std::unordered_map<C, V> buildNeighborhood(const C& cellId) const = 0;
	};
}  // namespace cadmium::celldevs

#endif //_CADMIUM_CELLDEVS_CORE_CONFIG_HPP_
