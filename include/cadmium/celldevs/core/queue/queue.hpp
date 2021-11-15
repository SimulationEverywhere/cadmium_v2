/**
 * Abstract definition of Cell-DEVS output queues and delay functions.
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

#ifndef _CADMIUM_CELLDEVS_CORE_QUEUE_HPP_
#define _CADMIUM_CELLDEVS_CORE_QUEUE_HPP_

#include <limits>

namespace cadmium::celldevs {
	/**
	 * Interface for implementing Cell-DEVS output queues and delay functions.
	 * @tparam S the type used for representing a cell state.
	 */
	template <typename S>
	struct OutputQueue {
	 public:
		virtual ~OutputQueue() = default;

		/**
		 * Adds a new state to the output queue, and schedules its propagation at a given time.
		 * @param state state to be transmitted by the cell.
		 * @param when clock time when this state must be transmitted.
		 */
		virtual void addToQueue(S state, double when) = 0;

		///@return clock time for the next scheduled output.
		virtual T nextTime() const = 0;

		/// @return next cell state to be transmitted.
		virtual S nextState() const = 0;

		/// Removes from buffer the next scheduled state transmission.
		virtual void pop() = 0;
	};
}

#endif //_CADMIUM_CELLDEVS_CORE_QUEUE_HPP_
