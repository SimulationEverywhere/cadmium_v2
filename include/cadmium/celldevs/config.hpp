/**
 * <one line to give the program's name and a brief idea of what it does.>
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

#ifndef _CADMIUM_CELLDEVS_CONFIG_HPP_
#define _CADMIUM_CELLDEVS_CONFIG_HPP_

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "utils.hpp"

namespace cadmium::celldevs {
	/**
	 * Cell configuration structure.
	 * @tparam C type used to represent cell IDs.
	 * @tparam S type used to represent cell states.
	 * @tparam V type used to represent vicinities between cells.
	 */
	template<typename C, typename S, typename V>
	struct Config {
		std::string delay;                      /// ID of the delay buffer of the cell.
		std::string cell_type;                  /// Cell model type.
		S state;                                /// Initial state of the cell.
		std::unordered_map<C, V> neighborhood;  /// Unordered map {neighbor_cell_position: vicinity}.
		nlohmann::json config;                   /// JSON file with additional configuration parameters.

		Config() = default;
		~Config() = default;

		Config(const std::string delay, const std::string cell_type, const S &state,
			const std::unordered_map<C, V> &neighborhood, cadmium::json config) :
			delay(delay), cell_type(cell_type), state(state), neighborhood(neighborhood),
			config(std::move(config)) {}
	};
}  // namespace cadmium::celldevs

#endif //_CADMIUM_CELLDEVS_CONFIG_HPP_
