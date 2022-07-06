#ifndef CADMIUM_EXAMPLE_EFP_GPT_HPP_
#define CADMIUM_EXAMPLE_EFP_GPT_HPP_

#include <cadmium/core/modeling/coupled.hpp>
#include "generator.hpp"
#include "processor.hpp"
#include "transducer.hpp"

namespace cadmium::example::gpt {
	//! Coupled DEVS model of the generator-processor-transducer.
	struct GPT : public Coupled {
		/**
		 * Constructor function for the GPT model.
		 * @param id ID of the gpt model.
		 * @param jobPeriod Job generation period for the Generator model.
		 * @param processingTime Job processing time for the Processor model.
		 * @param obsTime time to wait by the Transducer before asking the Generator to stop creating Job objects.
		 */
		GPT(const std::string& id, double jobPeriod, double processingTime, double obsTime): Coupled(id) {
			auto generator = addComponent<Generator>("generator", jobPeriod);
			auto processor = addComponent<Processor>("processor", processingTime);
			auto transducer = addComponent<Transducer>("transducer", obsTime);

			addCoupling(generator->outGenerated, processor->inGenerated);
			addCoupling(generator->outGenerated, transducer->inGenerated);
			addCoupling(processor->outProcessed, transducer->inProcessed);
			addCoupling(transducer->outStop, generator->inStop);
		}
	};
}  //namespace cadmium::example::gpt

#endif //CADMIUM_EXAMPLE_EFP_GPT_HPP_
