#ifndef CADMIUM_EXAMPLE_IESTREAM_HPP_
#define CADMIUM_EXAMPLE_IESTREAM_HPP_

#include <cadmium/core/modeling/coupled.hpp>
#include <lib/iestream.hpp>
#include "message_t.hpp"

/*
* Transfered from Cadmium V1 to Cadmium V2 by Jon Menard
* August 3, 2022
*/

namespace cadmium::example::iestream {
	//! Coupled DEVS model to test the iestream atomic model
	struct iestreamTest : public Coupled {
	
		Port<Message_t> output; 
		/**
		 * Constructor function for the iestream model.
		 * @param id ID of the iestream model.
		 */
		iestreamTest(const std::string& id): Coupled(id) {
			output = addOutPort<Message_t>("output");
			auto iestream1 = addComponent<Iestream<Message_t>>("iestream", "../example/ie_stream/test.txt");
			addCoupling(iestream1->out, output);
		}
	};
}  //namespace cadmium::example::iestream

#endif //CADMIUM_EXAMPLE_IESTREAM_HPP_
