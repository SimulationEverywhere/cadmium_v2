/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022-present Jon
 * Copyright (c) 2023-present Román Cárdenas Rodríguez
 */

#ifndef _OCCUPENCY_CONTROLLER_HPP__
#define _OCCUPENCY_CONTROLLER_HPP__

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>


namespace cadmium::example::covidSupervisorySystem {
	//! Class for representing the Processor DEVS model state.
	struct OccupencyControllerState {
		int personCount;
		double sigma;
		//! Processor state constructor. By default, the processor is idling.
		OccupencyControllerState(): personCount(0), sigma(0)  {}
	};

	/**
	 * Insertion operator for ProcessorState objects. It only displays the value of sigma.
	 * @param out output stream.
	 * @param s state to be represented in the output stream.
	 * @return output stream with sigma already inserted.
	 */
	std::ostream& operator<<(std::ostream &out, const OccupencyControllerState& state) {
		out << "Person Count:," << state.personCount;
		return out;
	}

	//! Atomic DEVS model of a Job processor.
	class OccupencyController : public Atomic<OccupencyControllerState> {
	 private:
		const int MAX_OCCUPANCY = 3;   
	 public:
		Port<bool> personIn;  
		Port<bool> personOut;
		Port<bool> personSafe;  
  

		/**
		 * Constructor function.
		 * @param id ID of the new Processor model object.
		 */
		OccupencyController(const std::string& id): Atomic<OccupencyControllerState>(id, OccupencyControllerState()) {
			personIn = addInPort<bool>("personIn");
			personOut = addInPort<bool>("personOut");
			personSafe = addOutPort<bool>("personSafe");
		}

		/**
		 * It updates the ProcessorState::clock, clears the ProcessorState::Job being processed, and passivates.
		 * @param state reference to the current state of the model.
		 */
		void internalTransition(OccupencyControllerState& state) const override {
			state.sigma = std::numeric_limits<double>::infinity();
		}

		/**
		 * Updates ProcessorState::clock and ProcessorState::sigma.
		 * If it is idling and gets a new Job via the Processor::inGenerated port, it starts processing it.
		 * @param state reference to the current model state.
		 * @param e time elapsed since the last state transition function was triggered.
		 * @param x reference to the model input port set.
		 */
		void externalTransition(OccupencyControllerState& state, double e) const override {
		
			int person_n = state.personCount;
			
			if(!personIn->empty()){
				for( const auto personEntered : personIn->getBag()){
					if(personEntered){
                    	state.personCount += 1;
                	}
				}
			}

			if(!personOut->empty()){
				for( const auto personLeft : personOut->getBag()){
					if(state.personCount > 0 && personLeft){
                    	state.personCount -= 1;
                	}
				}
			}


        	if(person_n != state.personCount){
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
		void output(const OccupencyControllerState& state) const override {	
			
			bool safe = false;
        
			if(state.personCount < MAX_OCCUPANCY){
				safe = true;
			}
			
			personSafe->addMessage(safe);	
		}

		/**
		 * It returns the value of ProcessorState::sigma.
		 * @param state reference to the current model state.
		 * @return the sigma value.
		 */
		[[nodiscard]] double timeAdvance(const OccupencyControllerState& state) const override {
			return state.sigma;
		}
	};
}  //namespace cadmium::example::covidSupervisorySystem

#endif //_OCCUPENCY_CONTROLLER_HPP__
