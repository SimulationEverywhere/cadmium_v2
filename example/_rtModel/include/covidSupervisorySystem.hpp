/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022-present Jon
 * Copyright (c) 2023-present Román Cárdenas Rodríguez
 */

#ifndef CADMIUM_EXAMPLE_COVID_SUPERVISORY_SYSTEM_HPP_
#define CADMIUM_EXAMPLE_COVID_SUPERVISORY_SYSTEM_HPP_

#include <cadmium/core/modeling/coupled.hpp>
#include <cadmium/lib/iestream.hpp>
#include "./digitalInput.hpp"
#include "./c02SensorController.hpp"
#include "./ledController.hpp"
#include "./occupencyController.hpp"

namespace cadmium::example::covidSupervisorySystem {
	//! Coupled DEVS model to show how the IEStream atomic model works.
	struct covidSupervisorySystem : public Coupled {

		/**
		 * Constructor function for the iestream model.
		 * @param id ID of the iestream model.
		 * @param filePath path to the input file to be read.
		 */
		covidSupervisorySystem(const std::string& id) : Coupled(id) {
			
			auto digitalInput = addComponent<DigitalInput>("digitalInput");
			auto iestreamOccIn = addComponent<IEStream<bool>>("iestreamOccIn", "../example/rtModel/occIn.txt");
			auto iestreamOccOut = addComponent<IEStream<bool>>("iestreamOccOut", "../example/rtModel/occOut.txt");
			

			auto occupencyController = addComponent<OccupencyController>("occupencyController");
			auto ledController = addComponent<LEDController>("ledController");
			auto c02SensorController = addComponent<C02SensorController>("c02SensorController");

			addCoupling(digitalInput->out, c02SensorController->c02In);
			addCoupling(iestreamOccIn->out, occupencyController->personIn);
			addCoupling(iestreamOccOut->out, occupencyController->personOut);
			
			
			addCoupling(occupencyController->personSafe, ledController->occupancyIn);
			addCoupling(c02SensorController->c02Safe, ledController->c02In);

		}
	};
}  //namespace cadmium::example::covidSupervisorySystem

#endif //CADMIUM_EXAMPLE_COVID_SUPERVISORY_SYSTEM_HPP_