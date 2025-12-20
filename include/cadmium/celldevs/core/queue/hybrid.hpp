/**
 * Cell-DEVS output queues ruled by the hybrid delay function.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 */


#ifndef CADMIUM_CELLDEVS_CORE_QUEUE_HYBRID_HPP_
#define CADMIUM_CELLDEVS_CORE_QUEUE_HYBRID_HPP_

#include <limits>
#include <deque>
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
	class HybridOutputQueue: public OutputQueue<S> {
	 private:
		std::shared_ptr<const S> nullPtr = nullptr;  //!< Just a reference for detecting empty queues.
		std::deque<std::pair<double, std::shared_ptr<const S>>> states;  //!< Double-ended queue with pairs <time, state>
	 public:
		//! Constructor function.
		HybridOutputQueue(): OutputQueue<S>(), states() {}

		/**
		 * Adds a new state to the output queue, and schedules its propagation at a given time.
		 * @param state state to be transmitted by the cell.
		 * @param when clock time when this state must be transmitted.
		 */
		[[maybe_unused]] void addToQueue(S state, double when) override {
			while (!states.empty() && states.back().first >= when) {
				states.pop_back();
			}
			states.push_back({when, std::make_shared<const S>(std::move(state))});
		}

		//! @return clock time for the next scheduled output.
		[[maybe_unused]] [[nodiscard]] double nextTime() const override {
			return (states.empty())? std::numeric_limits<double>::infinity() : states.front().first;
		}

		//! @return next cell state to be transmitted.
		[[maybe_unused]] const std::shared_ptr<const S>& nextState() const override {
			return (states.empty())? nullPtr : states.front().second;
		};

		//! Removes from buffer the next scheduled state transmission.
		void pop() override {
			if (!states.empty()) {
				states.pop_front();
			}
		}
	};
} // namespace cadmium::celldevs

#endif // CADMIUM_CELLDEVS_CORE_QUEUE_HYBRID_HPP_
