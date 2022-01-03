/**
 * TODO
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

#ifndef _CADMIUM_EXAMPLE_CELLDEVS_ASYMM_GREATER_CELLS_DEFAULT_HPP_
#define _CADMIUM_EXAMPLE_CELLDEVS_ASYMM_GREATER_CELLS_DEFAULT_HPP_

#include "cadmium/celldevs/asymm/cell.hpp"

class DefaultCell: public cadmium::celldevs::AsymmCell<int, int> {
 public:
	DefaultCell(const std::string& id, const std::shared_ptr<cadmium::celldevs::AsymmCellConfig<int, int>>& config):
		cadmium::celldevs::AsymmCell<int, int>(id, config){}

	/**
	 * Local computation function. The new state of the cell will correspond to the greater state of any neighboring cell.
	 * @param state current state of the cell.
	 * @param neighborhood neighborhood of the cell (unordered map {neighbor cell ID: vicinity of neighbor cell over the cell}).
	 * @param neighborsState states of the neighboring cells (unordered map {neighbor cell ID: neighbor cell state}).
	 * @param x set of input messages received by the cell when the local computation function was triggered.
	 * @return new state of the cell.
	 */
	[[nodiscard]] int localComputation(int state, const std::unordered_map<std::string, cadmium::celldevs::NeighborData<int, int>>& neighborhood, const cadmium::PortSet& x) const override {
		for (const auto& [neighborId, neighborData]: neighborhood) {
			auto neighborState = *neighborData.state;
			state = (neighborState > state)? neighborState : state;
		}
		return state;
	}

	/**
	 * Output delay function. Any state change will be outputted after waiting 1 unit of time.
	 * @param state  new cell state.
	 * @return simulation time to wait before outputting a message with the new cell state.
	 */
	[[nodiscard]] double outputDelay(const int& state) const override {
		return 1.;
	}
};

#endif //_CADMIUM_EXAMPLE_CELLDEVS_ASYMM_GREATER_CELLS_DEFAULT_HPP_
