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

#ifndef CADMIUM_MODELING_CELLDEVS_CORE_CELL_HPP_
#define CADMIUM_MODELING_CELLDEVS_CORE_CELL_HPP_

#include <sstream>
#include <memory>
#include <unordered_map>
#include <utility>
#include "../../devs/atomic.hpp"
#include "config.hpp"
#include "msg.hpp"
#include "queue/queue.hpp"
#include "utility.hpp"

namespace cadmium::celldevs {
	/**
	 * @brief Abstract DEVS atomic model for defining cells in Cell-DEVS scenarios.
	 * @tparam C the type used for representing a cell ID.
	 * @tparam S the type used for representing a cell state.
	 * @tparam V the type used for representing a neighboring cell's vicinities.
	 */
	template <typename C, typename S, typename V>
	class Cell: public AtomicInterface {
	 protected:
		const C id;                                                           //!< Cell ID
		const std::shared_ptr<const CellConfig<C, S, V>> cellConfig;	      //!< Cell configuration parameters.
		S state;                                                              //!< Cell state.
		std::unordered_map<C, NeighborData<S, V>> neighborhood;               //!< Cell neighborhood set.
		const std::unique_ptr<OutputQueue<S>> outputQueue;                    //!< Cell output queue ruled by a given delay type function.
		double clock;                                                         //!< Simulation clock (i.e. current time during a simulation).
		double sigma;                                                         //!< Time remaining until next internal state transition.
		BigPort<CellStateMessage<C, S>> inputNeighborhood;   //!< Cell input port. It receives new neighboring cells' state.
		BigPort<CellStateMessage<C, S>> outputNeighborhood;  //!< cell output port. It outputs cell state changes.
	 public:
		/**
		 * Creates a new cell for a Cell-DEVS model.
		 * It adds the input and output ports used by cells to communicate with each other.
		 * It also schedules a new message to be sent at t = 0 (i.e., at the beginning of the simulation).
		 * @param id ID of the cell to be created.
		 * @param config configuration parameters for creating the cell.
		 */
		Cell(const C& id, const std::shared_ptr<const CellConfig<C, S, V>>& cellConfig):
		  AtomicInterface(cellId(id)), id(id), cellConfig(cellConfig), state(cellConfig->state), neighborhood(cellConfig->buildNeighborhood(id)),
		  outputQueue(OutputQueue<S>::newOutputQueue(cellConfig->delayType)), clock(), sigma() {
			inputNeighborhood = addInBigPort<CellStateMessage<C, S>>("inputNeighborhood");
			outputNeighborhood = addOutBigPort<CellStateMessage<C, S>>("outputNeighborhood");
			outputQueue->addToQueue(state, clock);
		}

		/**
		 * Local computation function. It computes the new state of the cell.
		 * @param state copy of the current state of the cell.
		 * @param neighborhood neighborhood set of the cell (unordered map {neighbor cell ID: neighbor cell data}).
		 * @return new state of the cell.
		 */
		virtual S localComputation(S state, const std::unordered_map<C, NeighborData<S, V>>& neighborhood) const = 0;

		/**
		 * Output delay function. It determines the time to wait before outputting a message with the new cell state.
		 * @param state  new cell state.
		 * @return simulation time to wait before outputting a message with the new cell state.
		 */
		virtual double outputDelay(const S& state) const = 0;

		/**
		 * Returns a string representation of a cell.
		 * @param id ID of a cell.
		 * @return string representation of the cell ID.
		 */
		static std::string cellId(const C& id) {
			std::stringstream ss;
			ss << id;
			return ss.str();
		}

		//! @return constant reference to cell configuration parameters.
		const std::shared_ptr<const CellConfig<C, S, V>>& getCellConfig() const {
			return cellConfig;
		}

		//! @return constant reference to cell neighborhood set.
		const std::unordered_map<C, NeighborData<S, V>>& getNeighborhood() const {
			return neighborhood;
		}

		//! The internal transition function cleans the output queue and updates the clock and sigma.
		void internalTransition() override {
			outputQueue->pop();
			clock += sigma;
			sigma = outputQueue->nextTime() - clock;
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
				neighborhood.at(msg->cellId).state = msg->state;
			}
			auto nextState = localComputation(state, neighborhood);
			if (nextState != state) {
				outputQueue->addToQueue(nextState, clock + outputDelay(nextState));
				sigma = outputQueue->nextTime() - clock;
			}
			state = nextState;
		}

		//! The time advance function always corresponds to the value of sigma.
		[[nodiscard]] double timeAdvance() const override {
			return sigma;
		}

		//! The output function outputs the next state scheduled in the queue through the outputNeighborhood port.
		void output() override {
			auto nextState = outputQueue->nextState();
			if (nextState != nullptr) {
				outputNeighborhood->addMessage(CellStateMessage<C, S>(id, nextState));
			}
		}

		/**
		 * It logs the cell's current state.
		 * @return string representing the current cell state.
		 */
		[[nodiscard]] std::string logState() const override {
			std::stringstream ss;
			ss << state;
			return ss.str();
		}
	};
} // namespace cadmium::celldevs

#endif // CADMIUM_MODELING_CELLDEVS_CORE_CELL_HPP_
