/**
 * Abstract Model of a cell for grid Cell-DEVS scenarios.
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

#ifndef CADMIUM_MODELING_CELLDEVS_GRID_CELL_HPP_
#define CADMIUM_MODELING_CELLDEVS_GRID_CELL_HPP_

#include <memory>
#include <vector>
#include "../core/cell.hpp"
#include "config.hpp"
#include "scenario.hpp"
#include "utility.hpp"

namespace cadmium::celldevs {
	/**
	 * @brief Abstract base class for cells in grid Cell-DEVS scenarios.
	 * @tparam S type used to represent cell states.
	 * @tparam V type used to represent vicinities between cells.
	 */
	template <typename S, typename V>
	class GridCell: public Cell<coordinates, S, V> {
		using Cell<coordinates, S, V>::id;                   //!< Cell ID (i.e., cell position in the grid).
		const std::shared_ptr<const GridScenario> scenario;  //!< pointer to current Cell-DEVS scenario.
	 public:
		/**
		 * Creates a new cell for a grid Cell-DEVS model.
		 * @param id ID of the cell to be created (i.e., cell position in the grid).
		 * @param config configuration parameters for creating the grid cell.
		 */
		GridCell(const coordinates& id, const std::shared_ptr<const GridCellConfig<S, V>>& config):
			Cell<coordinates, S, V>(id, config), scenario(config->scenario) {}
		/**
		 * It computes the distance vector from the current cell to the destination cell provided.
		 * This method may throw an exception if the destination cell is not part of the current Cell-DEVS scenario.
		 * @param cellTo coordinates of the destination cell.
		 * @return distance vector from current cell to destination cell.
		 */
		[[maybe_unused]] [[nodiscard]] inline coordinates distanceVectorTo(const coordinates& cellTo) const {
			return scenario->distanceVector(id, cellTo);
		}

		/**
		 * It computes the distance vector from the the origin cell provided to the current cell.
		 * This method may throw an exception if the origin cell is not part of the current Cell-DEVS scenario.
		 * @param cellFrom coordinates of the origin cell.
		 * @return distance vector from origin cell to current cell.
		 */
		[[maybe_unused]] [[nodiscard]] inline coordinates distanceVectorFrom(const coordinates& cellFrom) const {
			return scenario->distanceVector(cellFrom, id);
		}

		/**
		 * It computes the destination cell resulting from adding a distance vector to the current cell.
		 * This method may throw an exception if the distance vector and/or the resulting cell are invalid.
		 * @param distance distance vector.
		 * @return coordinates of the destination cell.
		 */
		[[maybe_unused]] [[nodiscard]] inline coordinates cellTo(const coordinates& distance) const {
			return scenario->cellTo(id, distance);
		}

		/**
		 * It computes the origin cell resulting from subtracting a distance vector from the current cell.
		 * This method may throw an exception if the distance vector and/or the resulting cell are invalid.
		 * @param distance distance vector.
		 * @return coordinates of the origin cell.
		 */
		[[maybe_unused]] [[nodiscard]] inline coordinates cellFrom(const coordinates& distance) const {
			return scenario->cellFrom(distance, id);
		}

		/**
		 * It computes the Minkowski distance between current cell and the provided neighboring cell.
		 * @param p order to be applied when computing the Minkowski distance. It must be greater than 0.
		 * @param neighbor coordinates of the neighboring cell under study.
		 * @return Minkowski distance.
		 */
		[[nodiscard]] inline double minkowskiDistance(int p, const coordinates& neighbor) const {
			return scenario->minkowskiDistance(p, id, neighbor);
		}

		/**
		 * It computes the Euclidean distance between current cell and the provided neighboring cell.
		 * @param neighbor coordinates of the neighboring cell under study.
		 * @return Euclidean distance.
		 */
		[[maybe_unused]] [[nodiscard]] inline double euclideanDistance(const coordinates& neighbor) const {
			return minkowskiDistance(2, neighbor);
		}

		/**
		 * It computes the Manhattan distance between current cell and the provided neighboring cell.
		 * @param neighbor coordinates of the neighboring cell under study.
		 * @return Manhattan distance.
		 */
		[[maybe_unused]] [[nodiscard]] inline int manhattanDistance(const coordinates& neighbor) const {
			return scenario->manhattanDistance(id, neighbor);
		}

		/**
		 * It computes the Chebyshev distance between current cell and the provided neighboring cell.
		 * @param neighbor coordinates of the neighboring cell under study.
		 * @return Chebyshev distance.
		 */
		[[maybe_unused]] [[nodiscard]] inline int chebyshevDistance(const coordinates& neighbor) const {
			return scenario->chebyshevDistance(id, neighbor);
		}
	};
}

#endif //CADMIUM_MODELING_CELLDEVS_GRID_CELL_HPP_
