#ifndef CADMIUM_EXAMPLE_GPT_EF_HPP_
#define CADMIUM_EXAMPLE_GPT_EF_HPP_

#include <cadmium/core/modeling/coupled.hpp>
#include "generator.hpp"
#include "transducer.hpp"

namespace cadmium::example::gpt {
	//! Coupled DEVS model of the experimental frame.
	struct EF: public Coupled {
		BigPort<Job> inProcessed;   //!< Input Port for processed Job objects.
		BigPort<Job> outGenerated;  //!< Output Port for sending new Job objects to be processed.

		/**
		 * Constructor function for the experimental frame model.
		 * @param id ID of the experimental frame model.
		 * @param jobPeriod Job generation period for the Generator model.
		 * @param obsTime time to wait by the Transducer before asking the Generator to stop creating Job objects.
		 */
		EF(const std::string& id, double jobPeriod, double obsTime): Coupled(id) {
			inProcessed = addInBigPort<Job>("inProcessed");
			outGenerated = addOutBigPort<Job>("outGenerated");

			auto generator = addComponent<Generator>("generator", jobPeriod);
			auto transducer = addComponent<Transducer>("transducer", obsTime);

			addCoupling(inProcessed, transducer->inProcessed);
			addCoupling(transducer->outStop, generator->inStop);
			addCoupling(generator->outGenerated, transducer->inGenerated);
			addCoupling(generator->outGenerated, outGenerated);
		}
	};
}  //namespace cadmium::example::gpt

#endif //CADMIUM_EXAMPLE_GPT_EF_HPP_
