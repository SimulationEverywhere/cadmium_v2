#ifndef CADMIUM_EXAMPLE_EFP_EFP_HPP_
#define CADMIUM_EXAMPLE_EFP_EFP_HPP_

#include <cadmium/core/modeling/coupled.hpp>
#include "ef.hpp"
#include "processor.hpp"

namespace cadmium::example::gpt {
	//! Coupled DEVS model of the experimental frame-processor.
	struct EFP : public Coupled {
		/**
		 * Constructor function for the GPT model.
		 * @param id ID of the gpt model.
		 * @param jobPeriod Job generation period for the Generator model.
		 * @param processingTime Job processing time for the Processor model.
		 * @param obsTime time to wait by the Transducer before asking the Generator to stop creating Job objects.
		 */
		EFP(const std::string& id, double jobPeriod, double processingTime, double obsTime) : Coupled(id) {
			auto ef = addComponent<EF>("ef", jobPeriod, obsTime);
			auto processor = addComponent<Processor>("processor", processingTime);

			addCoupling(ef->outGenerated, processor->inGenerated);
			addCoupling(processor->outProcessed, ef->inProcessed);
		}
	};
}  //namespace cadmium::example::gpt

#endif //CADMIUM_EXAMPLE_EFP_EFP_HPP_
