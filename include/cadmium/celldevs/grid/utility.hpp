/**
 * Utility stuff for vectors. These are required for grid-based Cell-DEVS scenarios.
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

#ifndef CADMIUM_CELLDEVS_GRID_UTILILITY_HPP_
#define CADMIUM_CELLDEVS_GRID_UTILILITY_HPP_

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace cadmium::celldevs {
	using coordinates = std::vector<int>;  //!< Type alias for referring to cell coordinates

	/**
	 * Auxiliary function for printing cell coordinates.
	 * @param os output stream.
	 * @param v coordinates to be printed.
	 * @return output stream containing the printed values of the coordinates.
	*/
	std::ostream &operator<<(std::ostream &os, const coordinates & v) {
		os << "(";
		std::string separator;
		for (const auto &x : v) {
			os << separator << x;
			separator = ",";
		}
		os << ")";
		return os;
	}
}  //namespace cadmium::celldevs

/**
 * @brief Auxiliary hash container for vectors.
 *
 * It allows us to use vectors as keys in hash maps.
 * @tparam T vector content type.
 */
template <typename T>
struct std::hash<std::vector<T>> {
	/**
	 * Hashing function for vectors.
	 * This function is based on https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
	 * @param vec vector to be used by the hashing function.
	 * @return hash resulting from the vector.
	 */
	std::size_t operator()(const std::vector<T>& vec) const {
		std::size_t seed = vec.size();
		for(const auto &i: vec) {
			seed ^= hash<T>()(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

#endif //CADMIUM_CELLDEVS_GRID_UTILILITY_HPP_
