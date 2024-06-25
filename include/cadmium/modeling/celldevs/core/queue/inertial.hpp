/**
 * Cell-DEVS output queues ruled by the inertial delay function.
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

#ifndef CADMIUM_MODELING_CELLDEVS_CORE_QUEUE_INERTIAL_HPP_
#define CADMIUM_MODELING_CELLDEVS_CORE_QUEUE_INERTIAL_HPP_

#include <limits>
#include <memory>
#include <utility>
#include "queue.hpp"

namespace cadmium::celldevs {
	template <typename S>
	struct OutputQueue;

	/**
	 * @brief Cell-DEVS output queue and delay functions.
	 * @tparam S the type used for representing a cell state.
	 */
	template <typename S>
	class InertialOutputQueue: public OutputQueue<S> {
	 private:
		std::shared_ptr<const S> lastState;  //!< Pointer to last copy of cell state.
		double next;                         //!< Simulation time at which the cell must output its state.
	 public:
		//! Constructor function. Last state is nullptr and next time is infinity
		InertialOutputQueue(): OutputQueue<S>(), lastState(), next(std::numeric_limits<double>::infinity()) {}

		/**
		 * Adds a new state to the output queue, and schedules its propagation at a given time.
		 * @param state copy of the new cell state.
		 * @param when clock time when this state must be transmitted.
		 */
		[[maybe_unused]] void addToQueue(S state, double when) override {
			lastState = std::make_shared<const S>(std::move(state));
			next = when;
		}

		//! @return clock time for the next scheduled output.
		[[maybe_unused]] [[nodiscard]] double nextTime() const override {
			return next;
		}

		//! @return next cell state to be transmitted.
		[[maybe_unused]] const std::shared_ptr<const S>& nextState() const override {
			return lastState;
		};

		//! Removes from buffer the next scheduled state transmission.
		void pop() override {
			lastState = nullptr;
			next = std::numeric_limits<double>::infinity();
		}
	};
} // namespace cadmium::celldevs

#endif // CADMIUM_MODELING_CELLDEVS_CORE_QUEUE_INERTIAL_HPP_
