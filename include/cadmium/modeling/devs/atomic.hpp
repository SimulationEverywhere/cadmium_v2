/**
 * Abstract implementation of a DEVS atomic model.
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

#ifndef CADMIUM_MODELING_DEVS_ATOMIC_HPP_
#define CADMIUM_MODELING_DEVS_ATOMIC_HPP_

#include <memory>
#ifndef NO_LOGGING
	#include <sstream>
#endif
#include <string>
#include <utility>
#include <vector>
#include "component.hpp"
#include "port.hpp"

namespace cadmium {
	/**
	 * @brief Interface for DEVS atomic models.
	 *
	 * This abstract class does not consider atomic models' state,
	 * so Cadmium can treat atomic models with different state types as if they were of the same class.
	 */
    class AtomicInterface: public Component {
     public:
		/**
		 * Constructor function.
		 * @param id ID of the atomic model.
		 */
        explicit AtomicInterface(const std::string& id): Component(id) {}

		//! Virtual method for the atomic model's internal transition function.
        virtual void internalTransition() = 0;

		/**
		 * Virtual method for the atomic model's external transition function.
		 * @param e time elapsed since the last state transition of the model.
		 */
        virtual void externalTransition(double e) = 0;

		/**
		 * Virtual method for the atomic model's confluent transition function.
		 * By default, it first triggers the internal transition function and then the external with e = 0.
		 * @param e time elapsed since the last state transition of the model.
		 */
        virtual void confluentTransition(double e) {
			this->internalTransition();
			this->externalTransition(0.);
		}

		//! Virtual method for the atomic model's output function.
        virtual void output() = 0;

		/**
		 * Virtual method for the atomic model's time advance function.
		 * @return time to wait until next internal transition.
		 */
        [[nodiscard]] virtual double timeAdvance() const = 0;

	#ifndef NO_LOGGING
		/**
		 * Virtual method to log the atomic model's current state.
		 * @return string representing the current state of the atomic model.
		 */
		[[nodiscard]] virtual std::string logState() const = 0;
	#endif
    };

	/**
	 * @brief DEVS atomic model.
	 *
	 * The Atomic class is closer to the DEVS formalism than the AtomicInterface class.
	 * @tparam S the data type used for representing a cell state.
	 */
    template <typename S>
    class Atomic: public AtomicInterface {
     protected:
        S state;  //! Atomic model state.
     public:
		/**
		 * Constructor function.
		 * @param id ID of the atomic model.
		 * @param initialState initial atomic model state.
		 */
        explicit Atomic(const std::string& id, S initialState) : AtomicInterface(id), state(std::move(initialState)) {}

		/**
		 * Virtual method for the atomic model internal transition function.
		 * @param s reference to the current atomic model state. You MUST MODIFY the atomic model state here.
		 */
        virtual void internalTransition(S& s) const = 0;

		/**
		 * Virtual method for the atomic model external transition function.
		 * @param s reference to the current atomic model state. You MUST MODIFY the atomic model state here.
		 * @param e time elapsed since the last state transition function was triggered.
		 * @param x reference to the atomic model input port set. You can READ input messages here.
		 */
        virtual void externalTransition(S& s, double e) const = 0;

		/**
		 * Virtual method for the atomic model output function.
		 * @param s reference to the current atomic model state. You can READ the atomic model state here.
		 * @param y reference to the atomic model output port set. You MUST ADD output messages here.
		 */
        virtual void output(const S& s) const = 0;

		/**
		 * Virtual method for the time advance function.
		 * @param s reference to the current atomic model state. You can READ the atomic model state here.
		 * @return time to wait for the next internal transition function.
		 */
        virtual double timeAdvance(const S& s) const = 0;

		/**
		 * Virtual method for the confluent transition function.
		 * @param s reference to the current atomic model state. You MUST MODIFY the atomic model state here.
		 * @param e time elapsed since the last state transition function was triggered.
		 * @param x reference to the atomic model input port set. You can READ input messages here.
		 */
        virtual void confluentTransition(S& s, double e) const {
            this->internalTransition(s);
            this->externalTransition(s, 0.);
        }

        void internalTransition() override {
            this->internalTransition(state);
        }

        void externalTransition(double e) override {
            this->externalTransition(state, e);
        }

        void confluentTransition(double e) override {
            this->confluentTransition(state, e);
        }

        void output() override {
            this->output(state);
        }

        [[nodiscard]] double timeAdvance() const override {
            return this->timeAdvance(state);
        }

	#ifndef NO_LOGGING
		//! @return a string representation of the model state. S must implement the insertion (<<) operator.
		[[nodiscard]] std::string logState() const override {
			std::stringstream ss;
			ss << state;
			return ss.str();
		}
	#endif
    };
}

#endif //CADMIUM_MODELING_DEVS_ATOMIC_HPP_
