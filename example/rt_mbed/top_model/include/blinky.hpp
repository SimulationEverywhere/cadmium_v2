#ifndef _BLINKY_HPP__
#define _BLINKY_HPP__

#include <cadmium/modeling/devs/atomic.hpp>
#include <iostream>

namespace cadmium::blinkySystem {
	//! Class for representing the Blinky DEVS model state.struct BlinkyState {
	struct BlinkyState {
		double sigma;
		bool lightOn;
		bool fastToggle;
		//! Blinky state constructor.
		BlinkyState(): sigma(0), lightOn(false), fastToggle(false)  {}
	};
	/**
	 * Insertion operator for BlinkyState objects. It displays the value of sigma and lightOn.
	 * @param out output stream.
	 * @param s state to be represented in the output stream.
	 * @return output stream with sigma and lightOn already inserted.
	 */
	std::ostream& operator<<(std::ostream &out, const BlinkyState& state) {
		out << "Status:, " << state.lightOn << ", sigma: " << state.sigma;
		return out;
	}

	//! Atomic DEVS model of Blinky.
	class Blinky : public Atomic<BlinkyState> {
	 private:
		
	 public:
		Port<bool> out;  
		Port<bool> in;  
		double slowToggleTime;  
		double fastToggleTime;

		/**
		 * Constructor function.
		 * @param id ID of the new Blinky model object.
		 */
		Blinky(const std::string& id): Atomic<BlinkyState>(id, BlinkyState()) {
			out = addOutPort<bool>("out");
			in  = addInPort<bool>("in");
			slowToggleTime = 3.0;  
			fastToggleTime = 0.75;
			state.sigma = fastToggleTime;
		}

		/**
		 * It updates the BlinkyState::lightOn.
		 * @param state reference to the current state of the model.
		 */
		void internalTransition(BlinkyState& state) const override {
			state.lightOn = !state.lightOn;
		}

		/**
		 * Updates BlinkyState::fastToggle and BlinkyState::sigma.
		 * @param state reference to the current model state.
		 * @param e time elapsed since the last state transition function was triggered.
		 * @param x reference to the model input port set.
		 */
		void externalTransition(BlinkyState& state, double e) const override {

			if(!in->empty()){
				for( const auto x : in->getBag()){
					if (x==0)
						state.fastToggle = !state.fastToggle;
				}

				if(state.fastToggle)
					state.sigma = fastToggleTime;
				else
					state.sigma = slowToggleTime;
			}
		}

		/**
		 * It outputs the already processed BlinkyState::lightOn via the out port.
		 * @param state reference to the current model state.
		 * @param y reference to the atomic model output port set.
		 */
		void output(const BlinkyState& state) const override {
			out->addMessage(state.lightOn);	
		}

		/**
		 * It returns the value of BlinkyState::sigma.
		 * @param state reference to the current model state.
		 * @return the sigma value.
		 */
		[[nodiscard]] double timeAdvance(const BlinkyState& state) const override {
			return state.sigma;
		}
	};
}  //namespace cadmium::blinkySystem

#endif //_BLINKY_HPP__
