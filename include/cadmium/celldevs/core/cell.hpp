/**
 * Abstract implementation of a cell model according to the Cell-DEVS formalism
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

#ifndef _CADMIUM_CELLDEVS_CORE_CELL_HPP_
#define _CADMIUM_CELLDEVS_CORE_CELL_HPP_

#include <sstream>
#include <memory>
#include <unordered_map>
#include <utility>
#include "../../core/modeling/atomic.hpp"
#include "msg.hpp"
#include "queue/queue.hpp"

namespace cadmium::celldevs {

	/**
	 * Abstract DEVS atomic model for defining cells in asymmetric Cell-DEVS scenarios.
	 * @tparam C the type used for representing a cell ID.
	 * @tparam S the type used for representing a cell state.
	 * @tparam V the type used for representing a neighboring cell's vicinities.
	 */
	template <typename C, typename S, typename V>
	class cell: public AbstractAtomic {
	 protected:
		const C id;                                                              /// Cell ID
		S state;                                                                 /// Cell state
		const std::unordered_map<C, V> neighborhood;                             /// Cell neighborhood. It is defined as tuples {neighboring cell ID, vicinity factor of neighboring cell over the cell}
		std::unordered_map<C, std::shared_ptr<S>> neighborsState;                /// Last neighbors state known by the cell.
		const std::unique_ptr<OutputQueue<S>> outputQueue;                       /// Cell output queue ruled by a given delay type function.
		double clock;                                                            /// Simulation clock (i.e. current time during a simulation)
		double sigma;                                                            /// Time remaining until next internal state transition
		const std::shared_ptr<Port<CellStateMessage<C, S>>> inputNeighborhood;   /// Cell input port. It receives new neighboring cells' state.
		const std::shared_ptr<Port<CellStateMessage<C, S>>> outputNeighborhood;  /// cell output port. It outputs cell state changes.

	 public:
		/**
		 * Creates a new cell with neighbors which vicinities is explicitly specified.
		 * @param id ID of the cell to be created.
		 * @param initialState initial state of the cell.
		 * @param neighborhood unordered map which keys are neighboring cells' IDs and values correspond to the vicinity factors.
		 * @param delayType ID of the delay type function that rules the output queue of the cell.
		 */
		cell(const C id, S initialState, const std::unordered_map<C, V>& neighborhood, const std::string& delayType):
		  AbstractAtomic(id), id(std::move(id)), state(initialState), neighborhood(neighborhood), neighborsState(),
		  outputQueue(outputQueue->newOutputQueue(delayType)), clock(), sigma(),
		  inputNeighborhood(std::make_shared<cadmium::Port<CellStateMessage<C, S>>>("neighborhoodInput")),
		  outputNeighborhood(std::make_shared<cadmium::Port<CellStateMessage<C, S>>>("neighborhoodOutput")) {
			addInPort(inputNeighborhood);
			addOutPort(outputNeighborhood);
			outputQueue->addToQueue(initialState, clock);
		}

		/**
		 * Local computation function. It computes the new state of the cell.
		 * @param state current state of the cell.
		 * @param neighborhood neighborhood of the cell (unordered map {neighbor cell ID: vicinity of neighbor cell over the cell}).
		 * @param neighborsState states of the neighboring cells (unordered map {neighbor cell ID: neighbor cell state}).
		 * @param x set of input messages received by the cell when the local computation function was triggered.
		 * @return new state of the cell.
		 */
		virtual S localComputation(S state, const std::unordered_map<C, V>& neighborhood,
			const std::unordered_map<C, std::shared_ptr<S>>& neighborsState, const PortSet& x) const = 0;

		/**
		 * Output delay function. It determines the time to wait before outputting a message with the new cell state.
		 * @param state  new cell state.
		 * @return simulation time to wait before outputting a message with the new cell state.
		 */
		virtual double outputDelay(const S& state) const = 0;

		/// The internal transition function cleans the output queue and updates the clock and sigma.
		void internalTransition() override {
			outputQueue->pop();
			clock += sigma;
			sigma = outputQueue->next_timeout() - clock;
		}

		/**
		 * The external transition function updates the clock and sigma.
		 * Then, it refreshes the neighbors' state and computes the cell's next state.
		 * If the new cell state is different to the current state, it schedules a new message using the output queue.
		 * @param e elapsed time from the last event.
		 */
		void externalTransition(double e) override {
			clock += e;
			sigma -= e;
			for (auto const& msg: inputNeighborhood->getBag()) {
				if (neighborhood.find(msg.cellId) != neighborhood.end()) {
					neighborsState[msg.cellId] = msg.state;
				}
			}
			auto nextState = localComputation(state, neighborhood, neighborsState, interface->inPorts);
			if (nextState != state) {
				outputQueue->addToQueue(nextState, clock + outputDelay(nextState));
				sigma = outputQueue->nextTime() - clock;
			}
			state = nextState;
		}

		/// The time advance function always corresponds to the value of sigma.
		double timeAdvance() const override {
			return sigma;
		}

		/// The output function outputs the next state scheduled in the queue through the outputNeighborhood port.
		void output() override {
			auto nextState = outputQueue->nextState();
			if (nextState != nullptr) {
				outputNeighborhood->addMessage(CellStateMessage<C, S>(id, nextState));
			}
		}

		/**
		 * It logs the cell's current state to the provided logger.
		 * @param logger pointer to the simulation logger.
		 * @param time simulation time when this function was triggered.
		 * @param modelId unique number that corresponds to the cell model.
		 */
		void logState(std::shared_ptr<Logger>& logger, double time, long modelId) const override {
			std::stringstream ss;
			ss << state;
			logger->logState(time, modelId, getId(), ss.str());
		}
	};
} //namespace cadmium::celldevs

#endif //_CADMIUM_CELLDEVS_CORE_CELL_HPP_
