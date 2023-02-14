/**
 * Structs for assisting in the model data storage and representation.
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

#ifndef CADMIUM_MODELING_CELLDEVS_CORE_UTILITY_HPP_
#define CADMIUM_MODELING_CELLDEVS_CORE_UTILITY_HPP_

#include <memory>
#include <nlohmann/json.hpp>
#include <utility>
#include "../grid/utility.hpp"

namespace cadmium::celldevs {
	/**
	 * @brief Neighbor cell data structure.
	 * @tparam S type used to represent cell states.
	 * @tparam V type used to represent vicinities between cells.
	 */
	template<typename S, typename V>
	struct NeighborData {
		std::shared_ptr<const S> state;  //!< Pointer to the latest known state of the neighboring cell.
		V vicinity;                      //!< Vicinity factor of neighboring cell over the cell that holds the data.

		//! Default constructor function.
		NeighborData() : state(), vicinity() {}

		/**
		 * Constructor function with explicit vicinity factor.
		 * @param vicinity Vicinity factor of neighboring cell over the cell that holds the data.
		 */
		explicit NeighborData(V vicinity) : state(), vicinity(std::move(vicinity)) {}
	};

	/**
	 * Function to deserialize a JSON object as a neighbor data object in the JSON notation.
	 * Note that the neighboring cell state is always a null pointer, as this is a simulation-only parameter.
	 * @tparam S type used to represent cell states.
	 * @tparam V type used to represent vicinities between cells.
	 * @param j JSON object that contains the serialized representation of the object.
	 * @param n object to be deserialized.
	 */
	template <typename S, typename V>
	void from_json(const nlohmann::json& j, NeighborData<S, V>& n) {
		j.get_to(n.vicinity);
	}
} // namespace cadmium::celldevs

#endif // CADMIUM_MODELING_CELLDEVS_CORE_UTILITY_HPP_
