#ifndef CADMIUM_EXAMPLE_DEVSTONE_DHRYSTONE_HPP_
#define CADMIUM_EXAMPLE_DEVSTONE_DHRYSTONE_HPP_

namespace cadmium::example::devstone {
	/**
	 * It runs the Dhrystone benchmark for a given period of time.
	 * @param millis it indicates how long (in milliseconds) should the Dhrystone benchmark be executing.
	 */
	void runDhrystone(int millis);
}  //namespace cadmium::example::devstone

#endif //CADMIUM_EXAMPLE_DEVSTONE_DHRYSTONE_HPP_
