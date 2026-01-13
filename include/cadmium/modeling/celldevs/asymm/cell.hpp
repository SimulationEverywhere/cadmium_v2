/**
 * Abstract implementation of a cell for asymmetric Cell-DEVS scenarios.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
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
