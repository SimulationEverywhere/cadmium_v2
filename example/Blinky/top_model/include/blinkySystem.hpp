#ifndef CADMIUM_EXAMPLE_BLINKY_HPP_
#define CADMIUM_EXAMPLE_BLINKY_HPP_

#include <cadmium/modeling/devs/coupled.hpp>

#ifdef RT_ARM_MBED
	#include <cadmium/simulation/_rt/real_time/arm_mbed/io/digitalOutput.hpp>
	#include <cadmium/simulation/_rt/real_time/arm_mbed/io/digitalInput.hpp>
#endif

#include "digitalInputExt.hpp"

#include "blinky.hpp"

#ifdef RT_ARM_MBED
	#include "../mbed.h"
	#include "PinNames.h"
#else
	#include "generator.hpp"
#endif

namespace cadmium::blinkySystem {

	struct blinkySystem : public Coupled {

			/**
		 	 * Constructor function for the blinkySystem model.
		 	 * @param id ID of the blinkySystem model.
		 	 */
			blinkySystem(const std::string& id) : Coupled(id) {
            		auto blinky = addComponent<Blinky>("blinky");
#ifdef RT_ARM_MBED		
			// NUCLEO F401RE
			auto digitalOutput = addComponent<DigitalOutput>("digitalOuput", LED1);
			auto digitalInput  = addComponent<DigitalInputExt>("digitalInput", PC_13);
			// BLUE PILL
			// auto digitalOutput = addComponent<DigitalOutput>("digitalOuput", PC_13);
			// auto digitalInput  = addComponent<DigitalInput>("digitalInput", PB_14);
			addCoupling(digitalInput->out, blinky->in);
			addCoupling(blinky->out, digitalOutput->in);
#else
			auto generator = addComponent<Generator>("generator");
			auto digitalInput  = addComponent<DigitalInputExt>("digitalInput");
			addCoupling(generator->out, digitalInput->in);
			addCoupling(digitalInput->out, blinky->in);
#endif
		}
	};
}  //namespace cadmium::blinkySystem

#endif //CADMIUM_EXAMPLE_BLINKY_HPP_
