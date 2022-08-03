#ifndef CADMIUM_EXAMPLE_ABP_MESSAGE_T_HPP_
#define CADMIUM_EXAMPLE_ABP_MESSAGE_T_HPP_

/*
* Transfered from Cadmium V1 to Cadmium V2 by Jon Menard
* August 3, 2022
*/

#include <iostream>

namespace cadmium::example {
	//!  Messages. Generator objects create new messages for ABP model
	struct Message_t {
		int packet;             //!<Packet Number.
		int bit;  				//!< Bit value.
		
		/**
		 * Constructor function for a Job object. As this job is not processed yet, @ref timeProcessed is set to -1.
		 * @param packet  Packet Number. 
		 * @param bit  Bit value
		 */
		Message_t(int packet, int bit): packet(packet), bit(bit) {};
		Message_t(): packet(0), bit(0) {};
	};

	/**
	 * Insertion operator for Job objects.
	 * @param out output stream.
	 * @param m message to be represented in the output stream.
	 * @return output stream with the value of the job already inserted.
	 */
	std::ostream& operator<<(std::ostream& out, const Message_t& m) {
		out << "{" << m.packet << "," << m.bit << "}";
		return out;
	}

    std::istream& operator>> (std::istream& is, Message_t& m)
    {
        is>> m.packet >> m.bit;
        return is;
    }
}  //namespace cadmium::example::gpt

#endif //CADMIUM_EXAMPLE_ABP_MESSAGE_T_HPP_
