/**
 * Abstract implementation of a cell for asymmetric Cell-DEVS scenarios.
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

#ifndef CADMIUM_MODELING_CELLDEVS_ASYMM_CELL_HPP_
#define CADMIUM_MODELING_CELLDEVS_ASYMM_CELL_HPP_

#include <string>
#include "../core/cell.hpp"
#include "config.hpp"

namespace cadmium::celldevs {
	/**
	 * @brief Abstract implementation of an asymmetric cell model.
	 * @tparam S type used to represent cell states.
	 * @tparam V type used to represent vicinities between cells.
	 */
	template <typename S, typename V>
	class AsymmCell: public Cell<std::string, S, V> {
	 public:
		/**
		 * Creates a new cell for an Asymmetric Cell-DEVS model.
		 * @param id ID of the cell to be created.
		 * @param config configuration parameters for creating the cell.
		 */
		AsymmCell(const std::string& id, const std::shared_ptr<const AsymmCellConfig<S, V>>& config): Cell<std::string, S, V>(id, config) {}
	};
} // namespace cadmium::celldevs

#endif // CADMIUM_MODELING_CELLDEVS_ASYMM_CELL_HPP_
