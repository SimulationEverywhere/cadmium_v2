/**
 * Abstract implementations of a DEVS atomic model.
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

#ifndef _CADMIUM_CORE_MODELING_ATOMIC_HPP_
#define _CADMIUM_CORE_MODELING_ATOMIC_HPP_

#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "component.hpp"
#include "port.hpp"
#include "../modeling/atomic.hpp"


namespace cadmium {

	/**
	 * Interface for DEVS atomic models. This abstract class does not consider atomic models' state,
	 * so Cadmium can treat atomic models with different state types as if they were of the same class.
	 */
    class AtomicInterface: public Component {
     public:
        explicit AtomicInterface(const std::string& id) : Component(id) {}

		/// Virtual method for the atomic model's internal transition function.
        virtual void internalTransition() = 0;

		/**
		 * Virtual method for the atomic model's external transition function.
		 * @param e time elapsed since the last state transition of the model.
		 */
        virtual void externalTransition(double e) = 0;

		/**
		 * Virtual method for the atomic model's confluent transition function.
		 * @param e time elapsed since the last state transition of the model.
		 */
        virtual void confluentTransition(double e) {
			this->internalTransition();
			this->externalTransition(0.);
		}

		/// Virtual method for the atomic model's output function.
        virtual void output() = 0;

		/**
		 * Virtual method for the atomic model's time advance function.
		 * @return time to wait until next internal transition.
		 */
        [[nodiscard]] virtual double timeAdvance() const = 0;

		/** TODO
		 * Virtual method to log the atomic models's current state.
		 * @param logger pointer to the simulation logger.
		 * @param time simulation time when this function was triggered.
		 * @param modelId unique number that corresponds to the DEVS atomic model.
		 *
		 */
		virtual std::string logState() const = 0;
    };

	/**
	 * DEVS atomic model. The Atomic class is closer to the DEVS formalism than the AbstractAtomic class.
	 * @tparam S the type used for representing a cell state.
	 */
    template <typename S>
    class Atomic: public AtomicInterface {
     protected:
        S state;
     public:
        explicit Atomic(const std::string& id, S initialState) : AtomicInterface(id), state(initialState) {}

        virtual void internalTransition(S& s) const = 0;
        virtual void externalTransition(S& s, double e, const PortSet& x) const = 0;
        virtual void output(const S& s, const PortSet& y) const = 0;
        virtual double timeAdvance(const S& s) const = 0;
        virtual void confluentTransition(S& s, double e, const PortSet& x) const {
            this->internalTransition(s);
            this->externalTransition(s, 0., x);
        }

        void internalTransition() override {
            this->internalTransition(state);
        }

        void externalTransition(double e) override {
            this->externalTransition(state, e, interface->inPorts);
        }

        void confluentTransition(double e) override {
            this->confluentTransition(state, e, interface->inPorts);
        }

        void output() override {
            this->output(state, interface->outPorts);
        }

        [[nodiscard]] double timeAdvance() const override {
            return this->timeAdvance(state);
        }

		std::string logState() const override {
			std::stringstream ss;
			ss << state;
			return ss.str();
			// logger->logState(time, modelId, getId(), ss.str());
		}
    };
}

#endif //_CADMIUM_CORE_MODELING_ATOMIC_HPP_
