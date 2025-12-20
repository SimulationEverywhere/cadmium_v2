/**
 * Abstract simulator.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 */


#ifndef CADMIUM_CORE_SIMULATION_ABS_SIMULATOR_HPP_
#define CADMIUM_CORE_SIMULATION_ABS_SIMULATOR_HPP_

#include <limits>
#include <memory>
#include "../logger/logger.hpp"
#include "../modeling/component.hpp"

namespace cadmium {
	//! Abstract simulator class.
    class AbstractSimulator {
	 protected:
		long modelId;     //!< Model identification number.
		double timeLast;  //!< Last simulation time.
        double timeNext;  //!< Next simulation time.
	 public:
		/**
		 * Constructor function.
		 * @param time initial simulation time.
		 */
		AbstractSimulator(double time): modelId(), timeLast(time), timeNext(std::numeric_limits<double>::infinity()) {}

		//! default destructor function.
		virtual ~AbstractSimulator() = default;

		//! @return last simulation time.
		[[nodiscard]] double getTimeLast() const {
			return timeLast;
		}

		//! @return next simulation time.
		[[nodiscard]] double getTimeNext() const {
			return timeNext;
		}

		//! @return pointer to the component corresponding to the abstract simulator.
		[[nodiscard]] virtual std::shared_ptr<Component> getComponent() const = 0;

		/**
		 * Sets the model number ID.
		 * @param next corresponding model ID number.
		 * @return the next model ID number.
		 */
		virtual long setModelId(long next) = 0;

		/**
		 * Sets a logger (atomic states and output messages).
		 * @param log pointer to the logger.
		 */
		virtual void setLogger(const std::shared_ptr<Logger>& log) = 0;

		/**
		 * It performs all the tasks needed before the simulation.
		 * @param time initial simulation time.
		 */
		virtual void start(double time) = 0;

		/**
		 * It performs all the tasks needed after the simulation.
		 * @param time last simulation time.
		 */
		virtual void stop(double time) = 0;

		/**
		 * It executes the model collection function.
		 * @param time simulation time.
		 */
		virtual void collection(double time) = 0;

		/**
		 * It executes the model transition function.
		 * @param time
		 */
		virtual void transition(double time) = 0;

		//! it clears the input and output ports of the model.
		virtual void clear() = 0;
    };
}

#endif //CADMIUM_CORE_SIMULATION_ABS_SIMULATOR_HPP_
