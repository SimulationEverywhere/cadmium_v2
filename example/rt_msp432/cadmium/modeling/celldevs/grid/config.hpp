/**
 * Structs for assisting the definition of grid cell configuration parameters.
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

#ifndef CADMIUM_MODELING_CELLDEVS_GRID_CONFIG_HPP_
#define CADMIUM_MODELING_CELLDEVS_GRID_CONFIG_HPP_

#include <memory>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <utility>
#include <vector>
#include "scenario.hpp"
#include "utility.hpp"
#include "../core/config.hpp"
#include "../../../exception.hpp"

namespace cadmium::celldevs {
	/**
	 * @brief Grid cell configuration structure.
	 * @tparam S type used to represent cell states.
	 * @tparam V type used to represent vicinities between cells.
	 */
	template<typename S, typename V>
	struct GridCellConfig : public CellConfig<coordinates, S, V> {
		using CellConfig<coordinates, S, V>::rawNeighborhood;
		std::vector<coordinates> cellMap;  //!< Vector of cells with this configuration. By default, it is empty.
		const std::shared_ptr<const GridScenario> scenario;  //!< Pointer to the grid Cell-DEVS scenario.
		std::unordered_map<coordinates, NeighborData<S, V>> absolute;  //!< Pre-processed neighborhood (only absolute neighbors).
		std::unordered_map<coordinates, NeighborData<S, V>> relative;  //!< Pre-processed neighborhood (only relative neighbors).

		/**
		 * Creates a new cell configuration structure from a JSON object.
		 * @param configId ID of the cell configuration structure.
		 * @param configParams JSON object containing all the cell configuration parameters.
		 * @param scenario  pointer to the grid Cell-DEVS scenario.
		 */
		GridCellConfig(std::string configId, const nlohmann::json& configParams, std::shared_ptr<const GridScenario>  scenario):
			CellConfig<coordinates, S, V>(configId, configParams), cellMap(), scenario(std::move(scenario)), absolute(), relative() {
			if (!CellConfig<coordinates, S, V>::isDefault() && configParams.contains("cell_map")) {
				cellMap = configParams["cell_map"].get<std::vector<coordinates>>();
			}
			processNeighborhood();
		}

		/**
		 * It reads the JSON file and pre-process the corresponding neighborhood.
		 * The resulting neighborhood is divided into "absolute" (i.e., the same neighborhood for all the cells),
		 * and "relative" (i.e., defined by distance vectors, each cell will have a different resulting neighborhood).
		 */
		void processNeighborhood() {
			for (const nlohmann::json& rawNeighbors: rawNeighborhood) {
				auto type = rawNeighbors.at("type").get<std::string>();
				auto vicinity = (rawNeighbors.contains("vicinity")) ? rawNeighbors["vicinity"].get<V>() : V();
				if (type == "absolute") {
					auto neighbors = rawNeighbors.at("neighbors").get<std::vector<coordinates>>();
					for (const auto& neighbor: neighbors) {
						if (scenario->cellInScenario(neighbor)) {
							absolute[neighbor] = NeighborData<S, V>(vicinity);
						} else {
							throw CadmiumModelException("Invalid absolute neighbor");
						}
					}
				} else {
					std::vector<coordinates> distances;
					if (type == "relative") {
						distances = rawNeighbors.at("neighbors").get<std::vector<coordinates>>();
					}
					else if (type == "moore") {
						auto range = (rawNeighbors.contains("range")) ? rawNeighbors["range"].get<int>() : 1;
						distances = scenario->mooreNeighborhood(range);
					}
					else if (type == "chebyshev" || type == "chessboard") {
						auto range = (rawNeighbors.contains("range")) ? (int)rawNeighbors["range"].get<double>() : 1;
						distances = scenario->mooreNeighborhood(std::floor(range));
					}
					else if (type == "von_neumann") {
						auto range = (rawNeighbors.contains("range")) ? rawNeighbors["range"].get<int>() : 1;
						distances = scenario->vonNeumannNeighborhood(range);
					}
					else if (type == "manhattan" || type == "taxicab" || type == "snake") {
						auto range = (rawNeighbors.contains("range")) ? (int)rawNeighbors["range"].get<double>() : 1;
						distances = scenario->vonNeumannNeighborhood(range);
					}
					else if (type == "minkowski" || type == "euclidean") {
						auto p = (type == "euclidean") ? 2 : rawNeighbors.at("p").get<int>();
						auto range = (rawNeighbors.contains("range")) ? rawNeighbors["range"].get<double>() : 1.;
						distances = scenario->minkowskiNeighborhood(p, range);
					}
					else {
						throw CadmiumModelException("Unknown neighborhood type");
					}
					for (const auto& distance: distances) {
						if (scenario->validDistance(distance)) {
							relative[distance] = NeighborData<S, V>(vicinity);
						} else {
							throw CadmiumModelException("Invalid distance vector");
						}
					}
				}
			}
		}

		/**
		 * It builds the neighborhood set for a given cell.
		 * @param cellId ID of the target cell that will own the resulting neighborhood set.
		 * @return unordered map {neighboring cell ID: vicinity factor of the neighboring cell over target cell}.
		 */
		std::unordered_map<coordinates, NeighborData<S, V>> buildNeighborhood(const coordinates& cellId) const override {
			std::unordered_map<coordinates, NeighborData<S, V>> neighborhood;
			for (const auto& [distance, vicinity]: relative) {
				try {
					neighborhood[scenario->cellTo(cellId, distance)] = vicinity;
				} catch (CadmiumModelException&) {
					continue;
				}
			}
			for (const auto& [neighbor, vicinity]: absolute) {
				neighborhood[neighbor] = vicinity;
			}
			return neighborhood;
		}
	};
}

#endif //CADMIUM_MODELING_CELLDEVS_GRID_CONFIG_HPP_
