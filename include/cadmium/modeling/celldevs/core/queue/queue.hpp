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

#ifndef CADMIUM_MODELING_CELLDEVS_CORE_QUEUE_HPP_
#define CADMIUM_MODELING_CELLDEVS_CORE_QUEUE_HPP_

#include <limits>
#include <memory>
#include <utility>
#include "inertial.hpp"
#include "transport.hpp"
#include "hybrid.hpp"
#include "../../../../exception.hpp"

namespace cadmium::celldevs {
	/**
	 * @brief Cell-DEVS output queue.
	 *
	 * Interface for implementing Cell-DEVS output queues ruled by a delay type function.
	 * @tparam S the type used for representing a cell state.
	 */
	template <typename S>
	struct OutputQueue {
		//! Virtual destructor function.
		virtual ~OutputQueue() = default;

		/**
		 * Adds a new state to the output queue and schedules its propagation at a given time.
		 * @param state state to be transmitted by the cell.
		 * @param when clock time when this state must be transmitted.
		 */
		virtual void addToQueue(S state, double when) = 0;

		//! @return clock time for the next scheduled output.
		[[nodiscard]] virtual double nextTime() const = 0;

		//! @return next cell state to be transmitted.
		virtual const std::shared_ptr<const S>& nextState() const = 0;

		//! Removes from buffer the next scheduled state transmission.
		virtual void pop() = 0;

		/**
		 * It returns a pointer to a new output queue ruled by the selected delay type function.
		 * @param delayType delay type function ID. So far, it allows "inertial", "transport", or "hybrid" IDs.
		 * @return unique pointer pointing to a new output queue structure. If delay type is not found, it raises
		 * @throw Exception if delay type function ID is unknown (i.e., it is not "inertial", "transport", nor "hybrid").
		 */
		static std::unique_ptr<OutputQueue<S>> newOutputQueue(std::string const &delayType) {
			if (delayType == "inertial") {
				return std::make_unique<InertialOutputQueue<S>>();
			} else if (delayType == "transport") {
				return std::make_unique<TransportOutputQueue<S>>();
			} else if (delayType == "hybrid") {
				return std::make_unique<HybridOutputQueue<S>>();
			} else {
				throw CadmiumModelException("delay type function not implemented");
			}
		}
	};
} // namespace cadmium::celldevs

#endif // CADMIUM_MODELING_CELLDEVS_CORE_QUEUE_HPP_
