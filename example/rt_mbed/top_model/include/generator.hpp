#ifndef _GENERATOR_HPP__
#define _GENERATOR_HPP__

#include <cadmium/modeling/devs/atomic.hpp>
#include <iostream>
#include <cstdlib>

namespace cadmium::blinkySystem {
	//! Class for representing the Generator DEVS model state.struct GeneratorState {
	struct GeneratorState {
		double sigma;
		bool val;
		//! Generator state constructor.
		GeneratorState(): sigma(0), val(0)  {}
	};
	/**
	 * Insertion operator for GeneratorState objects. It only displays the value of sigma.
	 * @param out output stream.
	 * @param s state to be represented in the output stream.
	 * @return output stream with sigma already inserted.
	 */
	std::ostream& operator<<(std::ostream &out, const GeneratorState& state) {
		out << "Status:, " << state.val; // state to string
		return out;
	}

	//! Atomic DEVS model of a Generator.
	class Generator : public Atomic<GeneratorState> {
	 private:
		
	 public:
		Port<bool> out;  
		float a, b;

		/**
		 * Constructor function.
		 * @param id ID of the new Generator model object.
		 */
		Generator(const std::string& id): Atomic<GeneratorState>(id, GeneratorState()) {
			out = addOutPort<bool>("out");
			a = 10; b = 20;
			state.val = 0;
			srand((unsigned) time(NULL));
			state.sigma = a + (float)rand()/RAND_MAX * (b-a); // sigma takes random values between 1 and 20
//			printf("[generator] init function\n");
		}

		/**
		 * It updates the GeneratorState::sigma.
		 * @param state reference to the current state of the model.
		 */
		void internalTransition(GeneratorState& state) const override {
			state.sigma = a + (float)rand()/RAND_MAX * (b-a); // sigma takes random values between 1 and 20 
//			printf("[generator] internal transition function\n");
		}

		/**
		 * Updates GeneratorState::state.
		 * @param state reference to the current model state.
		 * @param e time elapsed since the last state transition function was triggered.
		 * @param x reference to the model input port set.
		 */
		void externalTransition(GeneratorState& state, double e) const override {
			state.sigma = std::numeric_limits<double>::infinity();
//			printf("[generator] external transition function\n");
		}

		/**
		 * It outputs a 0 value to the out port.
		 * @param state reference to the current model state.
		 * @param y reference to the atomic model output port set.
		 */
		void output(const GeneratorState& state) const override {
			out->addMessage(state.val);
//			printf("[generator] output function\n");
		}

		/**
		 * It returns the value of GeneratorState::sigma.
		 * @param state reference to the current model state.
		 * @return the sigma value.
		 */
		[[nodiscard]] double timeAdvance(const GeneratorState& state) const override {
			return state.sigma;
		}
	};
}  //namespace cadmium::blinkySystem

#endif //_GENERATOR_HPP__
