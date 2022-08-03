#ifndef CADMIUM_EXAMPLE_IESTREAM_HPP_
#define CADMIUM_EXAMPLE_IESTREAM_HPP_

#include <cadmium/core/modeling/coupled.hpp>
#include <cadmium/lib/iestream.hpp>
#include "message_t.hpp"

namespace cadmium::example::iestream {
	//! Coupled DEVS model to test the iestream atomic model
	struct iestreamTest : public Coupled {
	
		Port<Message_t> output; 
		/**
		 * Constructor function for the iestream model.
		 * @param id ID of the iestream model.
		 * @param filePath path to the input file to be read.
		 */
		iestreamTest(const std::string& id, const char* filePath): Coupled(id) {
			output = addOutPort<Message_t>("output");
			auto iestream1 = addComponent<Iestream<Message_t>>("iestream", filePath);
			addCoupling(iestream1->out, output);
		}
	};
}  //namespace cadmium::example::iestream

#endif //CADMIUM_EXAMPLE_IESTREAM_HPP_
