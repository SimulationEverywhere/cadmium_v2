/**
 * Cell-DEVS grid scenario properties.
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

#ifndef CADMIUM_CELLDEVS_GRID_SCENARIO_HPP_
#define CADMIUM_CELLDEVS_GRID_SCENARIO_HPP_

#include <algorithm>
#include <cmath>
#include <numeric>
#include <utility>
#include <vector>
#include "utility.hpp"
#include "../../../exception.hpp"

namespace cadmium::celldevs {
	/**
	 * @brief Auxiliary struct for grid-like Cadmium Cell-DEVS models.
	 *
	 * This struct contains useful functions for these scenarios (e.g., computing distances between cells).
	 */
	struct GridScenario {
		const coordinates shape;   //!< shape of the scenarios (i.e., how many cells are in the scenario by dimension).
		const coordinates origin;  //!< coordinates of the origin cell. In 2D scenarios, it corresponds to the upper-left cell.
		const bool wrapped;        //!< if true, the scenario is wrapped.

		/**
		 * Grid scenario constructor function.
		 * @param shape shape of the scenario. It must have at least one dimension, and its values must be greater than 0.
		 * @param origin coordinates of the origin cell. The size must be equal to the size of the shape.
		 * @param wrapped it determines whether or not the grid Cell-DEVS scenario is wrapped.
		 */
		GridScenario(const coordinates& shape, const coordinates& origin, bool wrapped): shape(shape), origin(origin), wrapped(wrapped) {
			if (shape.empty()) {
				throw CadmiumModelException("invalid scenario shape");
			}
			if (std::any_of(shape.begin(), shape.end(), [](int a) { return a < 1; })) {
				throw CadmiumModelException("invalid scenario shape");
			}
			if (shape.size() != origin.size()) {
				throw CadmiumModelException("shape-origin dimension mismatch");
			}
		}

		/**
		 * It checks if a given cell is inside the scenario.
		 * @param cell coordinates of the cell under study.
		 * @return true if cell coordinates are within the scenario.
		 */
		[[nodiscard]] bool cellInScenario(const coordinates& cell) const {
			return cell.size() == shape.size() && std::all_of(cell.begin(), cell.end(), [this, i = 0](int a) mutable {
				a = a - origin[i];
				return a >= 0 && a < shape[i++];
			});
		}

		/**
		 * It checks if a given distance vector makes sense in the scenario.
		 * @param distance distance vector under study.
		 * @return true if the distance vector may be valid in the scenario.
		 */
		[[nodiscard]] bool validDistance(const coordinates& distance) const {
			return distance.size() == shape.size() && std::all_of(distance.begin(), distance.end(), [this, i = 0](int a) mutable {
				return std::abs(a) < shape[i++];
			});
		}

		/**
		 * It computes the distance vector that interconnects two cells.
		 * @param cellFrom coordinates of the origin cell.
		 * @param cellTo coordinates of the destination cell.
		 * @return a distance vector that describes how to go from cellFrom to cellTo.
		 */
		[[nodiscard]] coordinates distanceVector(const coordinates& cellFrom, const coordinates& cellTo) const {
			if (!cellInScenario(cellFrom) || !cellInScenario(cellTo)) {
				throw CadmiumModelException("Cell does not belong to scenario");
			}
			coordinates distance;
			for (int i = 0; i < shape.size(); ++i) {
				auto d = cellTo[i] - cellFrom[i];
				if (wrapped && abs(d) > shape[i] / 2) {
					d -= (int) copysign(shape[i], d);
				}
				distance.push_back(d);
			}
			if (!validDistance(distance)) {
				throw CadmiumModelException("Invalid distance vector");
			}
			return distance;
		}

		/**
		 * From an origin cell and a distance vector, it computes the destination cell.
		 * @param cellFrom coordinates of the origin cell.
		 * @param distance distance vector between the origin and destination cells.
		 * @return coordinates of the destination cell.
		 */
		[[maybe_unused]] [[nodiscard]] coordinates cellTo(const coordinates& cellFrom, const coordinates& distance) const {
			if (!cellInScenario(cellFrom)) {
				throw CadmiumModelException("Cell does not belong to scenario");
			}
			if (!validDistance(distance)) {
				throw CadmiumModelException("Invalid distance vector");
			}
			coordinates cellTo;
			for (int i = 0; i < shape.size(); ++i) {
				auto v = cellFrom[i] + distance[i];
				if (wrapped) {
					v = (v + shape[i]) % shape[i];
				}
				cellTo.push_back(v);
			}
			if (!cellInScenario(cellTo)) {
				throw CadmiumModelException("Cell does not belong to scenario");
			}
			return cellTo;
		}

		/**
		 * From a distance vector and a destination cell, it computes the origin cell.
		 * @param distance distance vector between the origin and destination cells.
		 * @param cellTo coordinates of the destination cell.
		 * @return coordinates of the origin cell.
		 */
		[[maybe_unused]] [[nodiscard]] coordinates cellFrom(const coordinates& distance, const coordinates& cellTo) const {
			if (!validDistance(distance)) {
				throw CadmiumModelException("Invalid distance vector");
			}
			if (!cellInScenario(cellTo)) {
				throw CadmiumModelException("Cell does not belong to scenario");
			}
			coordinates cellFrom;
			for (int i = 0; i < shape.size(); ++i) {
				auto v = cellTo[i] - distance[i];
				if (wrapped) {
					v = (v + shape[i]) % shape[i];
				}
				cellFrom.push_back(v);
			}
			if (!cellInScenario(cellFrom)) {
				throw CadmiumModelException("Cell does not belong to scenario");
			}
			return cellFrom;
		}

		/**
		 * It computes the Minkowski distance between two cells (see https://en.wikipedia.org/wiki/Minkowski_distance)
		 * @param p order to be applied when computing the Minkowski distance. It must be greater than 0.
		 * @param cellFrom coordinates of the origin cell.
		 * @param cellTo coordinates of the destination cell.
		 * @return value of the Minkowski distance between the provided cells.
		 */
		[[nodiscard]] inline double minkowskiDistance(int p, const coordinates& cellFrom, const coordinates& cellTo) const {
			return minkowskiDistance(p, distanceVector(cellFrom, cellTo));
		}

		/**
		 * It computes the Manhattan distance between two cells (see https://en.wikipedia.org/wiki/Taxicab_geometry)
		 * @param cellFrom coordinates of the origin cell.
		 * @param cellTo coordinates of the destination cell.
		 * @return value of the Manhattan distance between the provided cells.
		 */
		[[nodiscard]] inline int manhattanDistance(const coordinates& cellFrom, const coordinates& cellTo) const {
			return manhattanDistance(distanceVector(cellFrom, cellTo));
		}

		/**
		 * It computes the Chebyshev distance between two cells (see https://en.wikipedia.org/wiki/Chebyshev_distance)
		 * @param cellFrom coordinates of the origin cell.
		 * @param cellTo coordinates of the destination cell.
		 * @return value of the Chebyshev distance between the provided cells.
		 */
		[[nodiscard]] inline int chebyshevDistance(const coordinates& cellFrom, const coordinates& cellTo) const {
			return chebyshevDistance(distanceVector(cellFrom, cellTo));
		}

		/**
		 * It computes the Minkowski distance from a distance vector.
		 * @param p order to be applied when computing the Minkowski distance. It must be greater than 0.
		 * @param distance distance vector between the origin and destination cells.
		 * @return value of the Minkowski distance corresponding to the provided distance vector.
		 */
		[[nodiscard]] static double minkowskiDistance(int p, const coordinates& distance) {
			if (p < 1) {
				throw CadmiumModelException("p must be greater than 0");
			}
			auto x = std::accumulate(distance.begin(), distance.end(), 0., [p](double sum, int v) { return sum + std::pow(std::abs(v), p); });
			return std::pow(x, 1 / p);
		}

		/**
		 * It computes the Manhattan distance from a distance vector.
		 * @param distance distance vector between the origin and destination cells.
		 * @return value of the Manhattan distance corresponding to the provided distance vector.
		 */
		[[nodiscard]] static inline int manhattanDistance(const coordinates& distance) {
			return std::accumulate(distance.begin(), distance.end(), 0, [](int sum, int v) { return sum + std::abs(v); });
		}

		/**
		 * It computes the Chebyshev distance from a distance vector.
		 * @param distance distance vector between the origin and destination cells.
		 * @return value of the Chebyshev distance corresponding to the provided distance vector.
		 */
		[[nodiscard]] static inline int chebyshevDistance(const coordinates& distance) {
			return *std::max_element(distance.begin(), distance.end(), [](int a, int b) { return std::abs(a) < std::abs(b); });
		}

		/**
		 * It returns a vector with all the distance vectors that describe a Moore neighborhood (see https://en.wikipedia.org/wiki/Moore_neighborhood).
		 * @param range desired range for the resulting Moore neighborhood. It must be greater than 0.
		 * @return vector of distance vectors that conform the Moore neighborhood.
		 */
		[[nodiscard]] std::vector<coordinates> mooreNeighborhood(int range) const {
			std::vector<coordinates> neighborhood;
			for (const auto& distance: mooreScenario(range)) {
				if (validDistance(distance)) {
					neighborhood.push_back(distance);
				}
			}
			return neighborhood;
		}

		/**
		 * It returns a vector with all the distance vectors that describe a von Neumann neighborhood (see https://en.wikipedia.org/wiki/Von_Neumann_neighborhood).
		 * @param range desired range for the resulting Moore neighborhood. It must be greater than 0.
		 * @return vector of distance vectors that conform the von Neumann neighborhood.
		 */
		[[nodiscard]] std::vector<coordinates> vonNeumannNeighborhood(int range) const {
			std::vector<coordinates> neighborhood;
			for (const auto& distance: mooreScenario(range)) {
				if (validDistance(distance) && manhattanDistance(distance) <= range) {
					neighborhood.push_back(distance);
				}
			}
			return neighborhood;
		}

		/**
		 * It returns a vector with all the distance vectors which Minkowski distance is less than or equal to a given maximum distance.
		 * @param p order to be applied when computing the Minkowski distance. It must be greater than 0.
		 * @param maxDistance maximum Minkowski distance for the distance vectors.
		 * @return vector of distance vectors that conform the Minkowski neighborhood.
		 */
		[[nodiscard]] std::vector<coordinates> minkowskiNeighborhood(int p, double maxDistance) const {
			std::vector<coordinates> neighborhood;
			for (const auto& distance: mooreScenario((int) maxDistance)) {
				if (validDistance(distance) && minkowskiDistance(p, distance) <= maxDistance) {
					neighborhood.push_back(distance);
				}
			}
			return neighborhood;
		}

		/**
		 * It generates an unbiased grid scenario that mimics a Moore neighborhood.
		 * The coordinates of each cell in the scenario can be considered as a distance vector.
		 * @param range range of the Moore neighborhood. It must be greater than 0.
		 * @return Moore scenario of the desired range.
		 */
		[[nodiscard]] GridScenario mooreScenario(int range) const {
			if (range < 1) {
				throw CadmiumModelException("range must be greater than 0");
			}
			auto nShape = coordinates(shape.size(), 2 * range + 1);
			auto nOrigin = coordinates(shape.size(), -range);
			return {nShape, nOrigin, false};
		}

		//! Auxiliary class for iterating over all the cells within the grid scenario.
		class Iterator {
			const GridScenario *scenario;  //!< Pointer to the corresponding grid scenario.
			coordinates cell;         //!< latest cell coordinate computed by the iterator.

			/**
			 * It computes the coordinates of the next cell to be iterated.
			 * Once the iterator is done with all the cells in the scenario, it sets cell to an empty vector.
			 * @param d dimension being explored. Usually, it is set to 0 (i.e., the first dimension).
			 */
			void next(int d) {
				if (d < 0 || d >= cell.size()) {
					cell = {};
				} else if (cell[d] - scenario->origin[d] < scenario->shape[d] - 1) {
					cell[d]++;
				} else {
					cell[d] = scenario->origin[d];
					next(d + 1);
				}
			}

		 public:
			/**
			 * Iterator constructor.
			 * @param scenario pointer to the scenario being iterated.
			 * @param cell first cell coordinate (usually, the scenario origin cell).
			 */
			Iterator(const GridScenario *scenario, coordinates cell): scenario(scenario), cell(std::move(cell)) {
				if (!(this->cell.empty() || scenario->cellInScenario(this->cell))) {
					throw CadmiumModelException("Invalid iterator");
				}
			}

			//! Two iterators are different if the point to different scenarios or if the current cell is different.
			bool operator!=(const Iterator& b) const {
				return scenario != b.scenario || cell != b.cell;
			}

			//! when updating the iterator, we just call the next method and return the same iterator.
			Iterator& operator++() {
				next(0);
				return *this;
			}

			//! Iterators return the coordinates of the current cell.
			const coordinates& operator*() const {
				return cell;
			}
		};

		//! @return an iterator that initially points to the origin cell of the scenario.
		Iterator begin() {
			return {this, origin};
		}

		//! @return a consumed iterator (i.e., it initially points to an empty vector).
		Iterator end() {
			return {this, {}};
		}
	};
}

#endif //CADMIUM_CELLDEVS_GRID_SCENARIO_HPP_
