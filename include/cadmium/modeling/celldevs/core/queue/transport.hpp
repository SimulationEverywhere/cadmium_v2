/**
 * Cell-DEVS output queues ruled by the transport delay function.
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

#ifndef CADMIUM_MODELING_CELLDEVS_CORE_QUEUE_TRANSPORT_HPP_
#define CADMIUM_MODELING_CELLDEVS_CORE_QUEUE_TRANSPORT_HPP_

#include <limits>
#include <memory>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>
#include "queue.hpp"

namespace cadmium::celldevs {
	template <typename S>
	struct OutputQueue;

	/**
	 * @brief Cell-DEVS output queue and delay functions.
	 * @tparam S the type used for representing a cell state.
	 */
	template <typename S>
	class TransportOutputQueue: public OutputQueue<S> {
	 private:
		std::shared_ptr<const S> nullPtr = nullptr;  //!< Just a reference for detecting empty queues.
		std::priority_queue<double, std::vector<double>, std::greater<>> timeline;  //!< Queue with times with scheduled events.
		std::unordered_map<double, std::shared_ptr<const S>> states;  //!< Unordered map {scheduled time: state to transmit}.
	 public:

		//! Constructor function.
		TransportOutputQueue(): OutputQueue<S>(), timeline(), states() {}

		/**
		 * Adds a new state to the output queue, and schedules its propagation at a given time.
		 * @param state state to be transmitted by the cell.
		 * @param when clock time when this state must be transmitted.
		 */
		[[maybe_unused]] void addToQueue(S state, double when) override {
			if (states.find(when) == states.end()) {
				timeline.push(when);
			}
			states.insert_or_assign(when, std::make_shared<const S>(std::move(state)));
		}

		//! @return clock time for the next scheduled output.
		[[maybe_unused]] [[nodiscard]] double nextTime() const override {
			return (timeline.empty())? std::numeric_limits<double>::infinity() : timeline.top();
		}

		//! @return next cell state to be transmitted.
		[[maybe_unused]] const std::shared_ptr<const S>& nextState() const override {
			return (timeline.empty())? nullPtr : states.at(timeline.top());
		};

		//! Removes from buffer the next scheduled state transmission.
		void pop() override {
			if (!timeline.empty()) {
				states.erase(timeline.top());
				timeline.pop();
			}
		}
	};
} // namespace cadmium::celldevs

#endif // CADMIUM_MODELING_CELLDEVS_CORE_QUEUE_TRANSPORT_HPP_
