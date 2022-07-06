#ifndef CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HOMOD_HPP_
#define CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HOMOD_HPP_

#include "devstone_coupled.hpp"

namespace cadmium::example::devstone {
	//! HOmod DEVStone model class
	struct DEVStoneCoupledHOmod : public DEVStoneCoupled {
		/**
		 * HOmod model constructor function.
		 * @param width width of the HOmod DEVStone model.
		 * @param depth depth of the HOmod DEVStone model.
		 * @param intDelay internal delay for the DEVStone atomic models.
		 * @param extDelay external delay for the DEVStone atomic models.
		 */
		DEVStoneCoupledHOmod(int width, int depth, int intDelay, int extDelay);
	};
}  //namespace cadmium::example::devstone

#endif //CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HOMOD_HPP_
