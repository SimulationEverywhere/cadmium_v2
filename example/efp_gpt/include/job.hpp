#ifndef CADMIUM_EXAMPLE_GPT_JOB_HPP_
#define CADMIUM_EXAMPLE_GPT_JOB_HPP_

#include <iostream>

namespace cadmium::example::gpt {
	//! Job messages. Generator objects create new jobs, and Processor objects process them.
	struct Job {
		int id;                //!< Job ID number.
		double timeGenerated;  //!< Time in which the job was created.
		double timeProcessed;  //!< Time in which the job was processed. If -1, the job has not been processed yet.

		/**
		 * Constructor function for a Job object. As this job is not processed yet, @ref timeProcessed is set to -1.
		 * @param id  ID of the new job.
		 * @param timeGenerated time in which the job was generated.
		 */
		Job(int id, double timeGenerated): id(id), timeGenerated(timeGenerated), timeProcessed(-1) {};
	};

	/**
	 * Insertion operator for Job objects.
	 * @param out output stream.
	 * @param j job to be represented in the output stream.
	 * @return output stream with the value of the job already inserted.
	 */
	std::ostream& operator<<(std::ostream& out, const Job& j) {
		out << "{" << j.id << "," << j.timeGenerated << "," << j.timeProcessed << "}";
		return out;
	}
}  //namespace cadmium::example::gpt

#endif //CADMIUM_EXAMPLE_GPT_JOB_HPP_
