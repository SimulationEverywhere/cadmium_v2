#ifndef _LED_CONTROLLER_HPP__
#define _LED_CONTROLLER_HPP__

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>

namespace cadmium::example::covidSupervisorySystem {
	//! Class for representing the Processor DEVS model state.
	struct LEDControllerState {
		bool occupancySafe;
		bool c02Safe;
		bool ledColor;
		double sigma;
		//! Processor state constructor. By default, the processor is idling.
		LEDControllerState(): occupancySafe(true), c02Safe(true), ledColor(true), sigma(0)  {}
	};

	/**
	 * Insertion operator for ProcessorState objects. It only displays the value of sigma.
	 * @param out output stream.
	 * @param s state to be represented in the output stream.
	 * @return output stream with sigma already inserted.
	 */
	std::ostream& operator<<(std::ostream &out, const LEDControllerState& state) {
		out << "LED Status:, " << state.ledColor;
		return out;
	}

	//! Atomic DEVS model of a Job processor.
	class LEDController : public Atomic<LEDControllerState> {
	 private:
		
	 public:
		Port<bool> c02In;  
		Port<bool> occupancyIn;
		Port<bool> lEDOut;  
  

		/**
		 * Constructor function.
		 * @param id ID of the new Processor model object.
		 */
		LEDController(const std::string& id): Atomic<LEDControllerState>(id, LEDControllerState()) {
			c02In = addInPort<bool>("c02In");
			occupancyIn = addInPort<bool>("occupancyIn");
			lEDOut = addOutPort<bool>("lEDOut");
		}

		/**
		 * It updates the ProcessorState::clock, clears the ProcessorState::Job being processed, and passivates.
		 * @param state reference to the current state of the model.
		 */
		void internalTransition(LEDControllerState& state) const override {
			state.sigma = std::numeric_limits<double>::infinity();
		}

		/**
		 * Updates ProcessorState::clock and ProcessorState::sigma.
		 * If it is idling and gets a new Job via the Processor::inGenerated port, it starts processing it.
		 * @param state reference to the current model state.
		 * @param e time elapsed since the last state transition function was triggered.
		 * @param x reference to the model input port set.
		 */
		void externalTransition(LEDControllerState& state, double e) const override {
		
			if(!c02In->empty()){
				state.c02Safe = c02In->getBag().back();
			}

			if(!occupancyIn->empty()){
				state.occupancySafe = occupancyIn->getBag().back();
			}

			bool ledColor = state.ledColor;

			if(state.occupancySafe && state.c02Safe){
                state.ledColor = true;  
       	 	}else{
                state.ledColor = false;  
        	}

        	if(ledColor != state.ledColor){
            	state.sigma = 0;
				return;
        	} 
			
			state.sigma = std::numeric_limits<double>::infinity();
		}

		/**
		 * It outputs the already processed ProcessorState::Job via the Processor::outProcessed port.
		 * @param state reference to the current model state.
		 * @param y reference to the atomic model output port set.
		 */
		void output(const LEDControllerState& state) const override {	
			lEDOut->addMessage(state.ledColor);	
		}

		/**
		 * It returns the value of ProcessorState::sigma.
		 * @param state reference to the current model state.
		 * @return the sigma value.
		 */
		[[nodiscard]] double timeAdvance(const LEDControllerState& state) const override {
			return state.sigma;
		}
	};
}  //namespace cadmium::example::covidSupervisorySystem

#endif //_LED_CONTROLLER_HPP__
