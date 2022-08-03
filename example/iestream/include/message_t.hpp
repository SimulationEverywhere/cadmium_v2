#ifndef CADMIUM_EXAMPLE_ABP_MESSAGE_T_HPP_
#define CADMIUM_EXAMPLE_ABP_MESSAGE_T_HPP_

#include <iostream>

namespace cadmium::example::iestream {
	//!  Messages. Generator objects create new messages for ABP model
	struct Message_t {
		int packet;             //!<Packet Number.
		int bit;  				//!< Bit value.
		
		/**
		 * Constructor function for a message_t object.
		 * @param packet  Packet Number. 
		 * @param bit  Bit value
		 */
		Message_t(int packet, int bit): packet(packet), bit(bit) {};
		Message_t(): packet(0), bit(0) {};
	};

	/**
	 * Insertion operator for Message_t objects.
	 * @param out output stream.
	 * @param m message to be represented in the output stream.
	 * @return output stream with the value of the message_t already inserted.
	 */
	std::ostream& operator<<(std::ostream& out, const Message_t& m) {
		out << "{" << m.packet << "," << m.bit << "}";
		return out;
	}

    std::istream& operator>> (std::istream& is, Message_t& m) {
        is >> m.packet >> m.bit;
        return is;
    }
}  //namespace cadmium::example::iestream

#endif //CADMIUM_EXAMPLE_ABP_MESSAGE_T_HPP_
