#ifndef _C02_SENSOR_HPP__
#define _C02_SENSOR_HPP__

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>

namespace cadmium::example::covidSupervisorySystem {
	//! Class for representing the Processor DEVS model state.
	struct C02SensorControllerState {
		double sigma;
		double c02;
		//! Processor state constructor. By default, the processor is idling.
		C02SensorControllerState(): sigma(0), c02(1.00) {}
	};

	/**
	 * Insertion operator for ProcessorState objects. It only displays the value of sigma.
	 * @param out output stream.
	 * @param s state to be represented in the output stream.
	 * @return output stream with sigma already inserted.
	 */
	std::ostream& operator<<(std::ostream &out, const C02SensorControllerState& state) {
		out << "C02 Level:," << state.c02;
		return out;
	}

	//! Atomic DEVS model of a Job processor.
	class C02SensorController : public Atomic<C02SensorControllerState> {
	 private:
		const float MIN_CO2_VOLTAGE = 0.5;  //!< Time required by the Processor model to process one Job.
	 public:
		Port<bool> c02In;  //!< Input Port for receiving new Job objects.
		Port<bool> c02Safe;  //!< Output Port for sending processed Job objects.

		/**
		 * Constructor function.
		 * @param id ID of the new Processor model object.
		 */
		C02SensorController(const std::string& id): Atomic<C02SensorControllerState>(id, C02SensorControllerState()) {
			c02In = addInPort<bool>("c02In");
			c02Safe = addOutPort<bool>("c02Safe");
		}

		/**
		 * It updates the ProcessorState::clock, clears the ProcessorState::Job being processed, and passivates.
		 * @param state reference to the current state of the model.
		 */
		void internalTransition(C02SensorControllerState& state) const override {
			state.sigma = std::numeric_limits<double>::infinity();
		}

		/**
		 * Updates ProcessorState::clock and ProcessorState::sigma.
		 * If it is idling and gets a new Job via the Processor::inGenerated port, it starts processing it.
		 * @param state reference to the current model state.
		 * @param e time elapsed since the last state transition function was triggered.
		 * @param x reference to the model input port set.
		 */
		void externalTransition(C02SensorControllerState& state, double e) const override {
		
			if(!c02In->empty()){
				state.sigma = 0;
				if(c02In->getBag().back()){
					state.c02 = 1;
				}else{
					state.c02 = 0; 
				}

				return;
			}

			state.sigma = std::numeric_limits<double>::infinity();

			
		}

		/**
		 * It outputs the already processed ProcessorState::Job via the Processor::outProcessed port.
		 * @param state reference to the current model state.
		 * @param y reference to the atomic model output port set.
		 */
		void output(const C02SensorControllerState& state) const override {
			
			bool safe = false;
        
        	if(state.c02 > MIN_CO2_VOLTAGE){
           	 	safe = true;
        	}
			c02Safe->addMessage(safe);
			
		}

		/**
		 * It returns the value of ProcessorState::sigma.
		 * @param state reference to the current model state.
		 * @return the sigma value.
		 */
		[[nodiscard]] double timeAdvance(const C02SensorControllerState& state) const override {
			return state.sigma;
		}
	};
}  //namespace cadmium::example::covidSupervisorySystem

#endif //_C02_SENSOR_HPP__
