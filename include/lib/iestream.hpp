#ifndef IESTREAM_HPP__
#define IESTREAM_HPP__

/*
* Transfered from Cadmium V1 to Cadmium V2 by Jon Menard
* August 3, 2022
*/

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>
#include <optional>

namespace cadmium {
	template<typename MSG_CLASS>
	class Parser {
		private:
            std::ifstream file;
		public:
			// Constructors
			explicit Parser(const char* file_path) {
                file.open(file_path);
				std::cout << "Opening file " << file_path << "\n";
            };

			std::pair<double,MSG_CLASS> next_timed_input() {
					double sigma;
					MSG_CLASS contents;
					if(!file.is_open())
					{	
						std::cout << "File couild not be opened \n  ";
						MSG_CLASS contents;
						sigma = std::numeric_limits<double>::infinity();
					}else{
						if (file.eof()) throw std::exception();
						file >> sigma; // read time of next message
						file >> contents; // read values to go into message
						// std::cout << next_time << "  " << contents << "\n"; // uncomment to debug input file
					};
					return std::make_pair(sigma,contents);
			}
	};

	//! Class for representing the Iestream DEVS model state.
	template<typename MSG>
	struct States {
        Parser<MSG> parser;
        MSG last_input_read;
        double clock;;
        double sigma;
		bool initalized;

		//! Processor state constructor. By default, the processor is idling.
		explicit States(const char* filename): parser(filename), last_input_read(), clock(), sigma(), initalized(false) {}
	};

	/**
	 * Insertion operator for ProcessorState objects. It only displays the value of sigma.
	 * @param out output stream.
	 * @param s state to be represented in the output stream.
	 * @return output stream with sigma already inserted.
	 */
	template<typename MSG>
	std::ostream& operator<<(std::ostream &out, const States<MSG>& s) {
		out << "," << s.clock << " ," << s.sigma << "," << s.last_input_read;  
		return out;
	}

	//! Atomic DEVS model of a Job processor.
	template<typename MSG>
	class Iestream : public Atomic<States<MSG>> {
	 private:
		//mutable Parser<MSG> parser;
	 public:
		Port<MSG> out;
		/**
		 * Constructor function.
		 * @param id ID of the new Processor model object.
		 * @param filename time it takes the Processor to process a Job.
		 */
		
		Iestream(const std::string& id, const char* filename): Atomic<States<MSG>>(id, States<MSG>(filename)) {
			out = Atomic<States<MSG>>::template addOutPort<MSG>("out");
		}

		/**
		 * Read the next line of file and when that message should be sent
		 * if the time to be sent is in the past, then passivate model
		 * @param s reference to the current state of the model.
		 */
		void internalTransition(States<MSG>& s) const override {
			s.clock += s.sigma;

			if(!s.initalized){
				s.clock = 0;
				s.sigma = std::numeric_limits<double>::infinity();
			}

			try {
				std::pair<double, MSG> parsed_line = s.parser.next_timed_input();
				s.sigma = parsed_line.first - s.clock;
				if (s.sigma < 0) throw std::exception();

				s.last_input_read = parsed_line.second;
			} catch(std::exception& e) {
				s.sigma = std::numeric_limits<double>::infinity();
			}
			s.initalized = true;
		}

		/**
		 * 
		 * @param s reference to the current model state.
		 * @param e time elapsed since the last state transition function was triggered.
		 */
		void externalTransition(States<MSG>& s, double e) const override {
            s.clock += e;
            s.sigma -= e;
        }

		/**
		 * It outputs the next message
		 * @param s reference to the current model state.
		 * @param y reference to the atomic model output port set.
		 */
		void output(const States<MSG>& s) const override {
				if(s.initalized){
					out->addMessage(s.last_input_read);
				}
		}

		/**
		 * It returns the value of State::sigma.
		 * @param s reference to the current model state.
		 * @return the sigma value.
		 */
		[[nodiscard]] double timeAdvance(const States<MSG>& s) const override {
			return s.sigma;
		}
	};
}  //namespace cadmium

#endif //IESTREAM_HPP__