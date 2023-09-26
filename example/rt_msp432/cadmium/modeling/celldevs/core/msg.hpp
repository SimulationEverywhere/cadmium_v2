/**
 * Everything related to cell state messages sent in Cell-DEVS models
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

#ifndef CADMIUM_MODELING_CELLDEVS_CORE_MSG_HPP_
#define CADMIUM_MODELING_CELLDEVS_CORE_MSG_HPP_

#include <iostream>
#include <memory>

namespace cadmium::celldevs {
	/**
	 * @brief Cell state message struct.
	 * @tparam C the type used for representing a cell ID.
	 * @tparam S the type used for representing a cell state.
	 */
	template <typename C, typename S>
	struct CellStateMessage {
		C cellId;	                     //! ID of the cell that generated the message.
		std::shared_ptr<const S> state;	 //! pointer to a copy of the cell state when the message was created.

		/**
		 * Constructor function
		 * @param cellId ID of the cell that sends the message.
		 * @param state State shared by the sending cell.
		 */
		CellStateMessage(C cellId, std::shared_ptr<const S> state): cellId(cellId), state(state) {}
	};

	/**
	 * Operator for printing cell state messages in streams. It only outputs the reported cell state.
	 * @tparam C the type used for representing a cell ID.
	 * @tparam S the type used for representing a cell state.
	 * @param os output stream.
	 * @param msg cell message containing the state to be printed.
	 * @return the output stream with the cell state already printed.
	 */
	template <typename C, typename S>
	std::ostream &operator<<(std::ostream &os, const CellStateMessage<C, S> &msg) {
		os << *msg.state;
		return os;
	}
} // namespace cadmium::celldevs

#endif // CADMIUM_MODELING_CELLDEVS_CORE_MSG_HPP_
