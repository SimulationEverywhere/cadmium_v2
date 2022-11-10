#ifndef CADMIUM_EXAMPLE_IESTREAM_HPP_
#define CADMIUM_EXAMPLE_IESTREAM_HPP_

#include <cadmium/core/modeling/coupled.hpp>
#include <cadmium/lib/iestream.hpp>
#include "message_t.hpp"

namespace cadmium::example::iestream {
	//! Coupled DEVS model to show how the IEStream atomic model works.
	struct iestream_coupled : public Coupled {

		/**
		 * Constructor function for the iestream model.
		 * @param id ID of the iestream model.
		 * @param filePath path to the input file to be read.
		 */
		iestream_coupled(const std::string& id, const char* filePath): Coupled(id) {
			auto iestream1 = addComponent<lib::IEStream<Message_t>>("iestream", filePath);
		}
	};
}  //namespace cadmium::example::iestream

#endif //CADMIUM_EXAMPLE_IESTREAM_HPP_
