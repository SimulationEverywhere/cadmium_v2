/**
 * Everything related to cell state messages sent in Cell-DEVS models
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
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
