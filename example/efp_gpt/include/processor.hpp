#ifndef CADMIUM_EXAMPLE_GPT_PROCESSOR_HPP_
#define CADMIUM_EXAMPLE_GPT_PROCESSOR_HPP_

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>
#include <optional>
#include "job.hpp"

namespace cadmium::example::gpt {
	//! Class for representing the Processor DEVS model state.
	struct ProcessorState {
		double clock;                   //!< Current simulation time.
		double sigma;                   //!< Time to wait before triggering the next internal transition function.
		std::optional<Job> currentJob;  //!< Job being processed.

		//! Processor state constructor. By default, the processor is idling.
		ProcessorState(): clock(), sigma(std::numeric_limits<double>::infinity()), currentJob() {}
	};

	/**
	 * Insertion operator for ProcessorState objects. It only displays the value of sigma.
	 * @param out output stream.
	 * @param s state to be represented in the output stream.
	 * @return output stream with sigma already inserted.
	 */
	std::ostream& operator<<(std::ostream &out, const ProcessorState& s) {
		out << s.sigma;
		return out;
	}

	//! Atomic DEVS model of a Job processor.
	class Processor : public Atomic<ProcessorState> {
	 private:
		double processingTime;  //!< Time required by the Processor model to process one Job.
	 public:
		BigPort<Job> inGenerated;  //!< Input Port for receiving new Job objects.
		BigPort<Job> outProcessed;  //!< Output Port for sending processed Job objects.

		/**
		 * Constructor function.
		 * @param id ID of the new Processor model object.
		 * @param processingTime time it takes the Processor to process a Job.
		 */
		Processor(const std::string& id, double processingTime): Atomic<ProcessorState>(id, ProcessorState()), processingTime(processingTime) {
			inGenerated = addInBigPort<Job>("inGenerated");
			outProcessed = addOutBigPort<Job>("outProcessed");
		}

		/**
		 * It updates the ProcessorState::clock, clears the ProcessorState::Job being processed, and passivates.
		 * @param s reference to the current state of the model.
		 */
		void internalTransition(ProcessorState& s) const override {
			s.clock += s.sigma;
			s.currentJob.reset();
			s.sigma = std::numeric_limits<double>::infinity();
		}

		/**
		 * Updates ProcessorState::clock and ProcessorState::sigma.
		 * If it is idling and gets a new Job via the Processor::inGenerated port, it starts processing it.
		 * @param s reference to the current model state.
		 * @param e time elapsed since the last state transition function was triggered.
		 * @param x reference to the model input port set.
		 */
		void externalTransition(ProcessorState& s, double e) const override {
			s.clock += e;
			s.sigma -= e;
			if (!s.currentJob.has_value() and !inGenerated->empty()) {  // If Processor is busy, it ignores new Jobs
				auto newJob = inGenerated->getBag().back();
				s.currentJob.emplace(newJob->id, newJob->timeGenerated);
				s.currentJob->timeProcessed = s.clock + processingTime;
				s.sigma = processingTime;
			}
		}

		/**
		 * It outputs the already processed ProcessorState::Job via the Processor::outProcessed port.
		 * @param s reference to the current model state.
		 * @param y reference to the atomic model output port set.
		 */
		void output(const ProcessorState& s) const override {
			if (s.currentJob.has_value()) {
				outProcessed->addMessage(s.currentJob.value());
			}
		}

		/**
		 * It returns the value of ProcessorState::sigma.
		 * @param s reference to the current model state.
		 * @return the sigma value.
		 */
		[[nodiscard]] double timeAdvance(const ProcessorState& s) const override {
			return s.sigma;
		}
	};
}  //namespace cadmium::example::gpt

#endif //CADMIUM_EXAMPLE_GPT_PROCESSOR_HPP_
