/**
 * Structs for assisting the definition of cell configuration parameters.
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

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "cadmium/celldevs/grid/utility.hpp"

namespace cadmium::celldevs {

	/**
	 * Neighbor cell data structure.
	 * @tparam S type used to represent cell states.
	 * @tparam V type used to represent vicinities between cells.
	 */
	template <typename S, typename V>
	struct NeighborData {
		std::shared_ptr<S> state;  /// Pointer to the latest known state of the neighboring cell.
		V vicinity;				   /// Vicinity factor of neighboring cell over the cell that holds the data.

		NeighborData(): state(), vicinity() {}
		explicit NeighborData(V vicinity): state(), vicinity(std::move(vicinity)) {}

		/**
		 * Function to deserialize a JSON object as a neighbor data object in the JSON notation.
		 * Note that the neighboring cell state is always a null pointer, as this is a simulation-only parameter.
		 * @param j JSON object that contains the serialized representation of the object.
		 * @param n object to be deserialized.
		 */
		friend void from_json(const nlohmann::json& j, NeighborData<S, V>& n) {
			j.get_to(n.vicinity);
		}
	};

	/**
	 * Cell configuration structure.
	 * @tparam C type used to represent cell IDs.
	 * @tparam S type used to represent cell states.
	 * @tparam V type used to represent vicinities between cells.
	 */
	template<typename C, typename S, typename V>
	struct CellConfig {
		const std::string configId;                            /// ID of the cell configuration.
		std::string cellModel;                                 /// ID of the cell model. By default, it is set to "default".
		std::string delayType;                                 /// ID of the delay type function used by the cell. By default, it is set to "inertial".
		S state;                                               /// Initial state of the cell. By default, it is set to the default S value.
		nlohmann::json rawNeighborhood;                        /// JSON file with information regarding neighborhoods. By default, it is set to an empty JSON object.
		nlohmann::json cellConfig;                             /// JSON file with additional configuration parameters. By default, it is set to an empty JSON object.
		// TODO mover cellMap a solo grid.
		std::vector<C> cellMap;                                /// Vector with the IDs of those cells which follow this configuration. By default, it is set to an empty vector.
		std::vector<std::pair<std::string, std::string>> EIC;  /// pairs <port from, port to> that describe how to connect the outside world with the input of the cells.
		std::vector<std::pair<std::string, std::string>> EOC;  /// pairs <port from, port to> that describe how to connect the output of the cells with the outside world.

		virtual ~CellConfig() = default;

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
			cellConfig = (configParams.contains("config"))? configParams["config"] : nlohmann::json();
			cellMap = (!isDefault() && configParams.contains("cell_map")) ? configParams["cell_map"].get<std::vector<C>>() : std::vector<C>();
			if (configParams.contains("eic")) {
				EIC = configParams["eic"].get<std::vector<std::pair<std::string, std::string>>>();
			}
			if (configParams.contains("eoc")) {
				EOC = configParams["eoc"].get<std::vector<std::pair<std::string, std::string>>>();
			}
		}

		/// @return true if this configuration corresponds to the default configuration.
		[[nodiscard]] inline bool isDefault() const {
			return configId == "default";
		}

		/**
		 * It builds a neighborhood set for a given cell in the scenario.
		 * @param cellId ID of the cell that will own the neighborhood set.
		 * @return unordered map {neighbor cell ID: neighbor cell data}.
		 */
		virtual std::unordered_map<C, NeighborData<S, V>> buildNeighborhood(const C& cellId) const = 0;
	};
}  // namespace cadmium::celldevs

#endif //_CADMIUM_CELLDEVS_CORE_CONFIG_HPP_
