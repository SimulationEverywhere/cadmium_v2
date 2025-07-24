/**
 * Abstract implementation of a Imprecise DEVS atomic model.
 * Copyright (C) 2025	Sasisekhar Mangalam Govind
 * ARSLab	-	Carleton University
 * 
 * Copyright (C) 2021	Román Cárdenas Rodríguez
 * ARSLab	-	Carleton University
 * GreenLSI	-	Polytechnic University of Madrid
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

#ifndef CADMIUM_MODELING_IMPRECISE_DEVS_ATOMIC_HPP_
#define CADMIUM_MODELING_IMPRECISE_DEVS_ATOMIC_HPP_

#include <memory>
#ifndef NO_LOGGING
	#include <sstream>
#endif
#include "../devs/atomic.hpp"
#include <concepts>

namespace cadmium {
	enum class C {
		OPTIONAL,
		MANDATORY
	};
	inline std::ostream &operator << (std::ostream &os, const C& x) {
		os << ((x == C::OPTIONAL)? "OPTIONAL" : "MANDATORY");
		return os;
	}

	template <typename S>
	concept HasComputationEnum = requires (S s) { 
		requires std::same_as<decltype(s.computation), C>;
	};

	/**
	 * @brief Imprecise DEVS atomic model.
	 *
	 * The Atomic class is closer to the DEVS formalism than the AtomicInterface class.
	 * @tparam S the data type used for representing a cell state.
	 */
    template <typename S>
    class IAtomic: public AtomicInterface {
		static_assert(HasComputationEnum<S>, 
			"S must have a 'computation' member of type C enum.");
     protected:
        S state;  //! Atomic model state.
     public:
		/**
		 * Constructor function.
		 * @param id ID of the atomic model.
		 * @param initialState initial atomic model state.
		 */
        explicit IAtomic(const std::string& id, S initialState) : AtomicInterface(id), state(std::move(initialState)) {}

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
		 * Virtual method for the deadline.
		 * @param s reference to the current atomic model state. You can READ the atomic model state here.
		 * @return time; definition of deadline not decided yet.
		 */
        virtual double deadline(const S& s) const = 0;

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

		[[nodiscard]] double deadline() const {
            return this->deadline(state);
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

#endif //CADMIUM_MODELING_IMPRECISE_DEVS_ATOMIC_HPP_
