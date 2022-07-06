#ifndef CADMIUM_EXAMPLE_DEVSTONE_COUPLED_LI_HPP_
#define CADMIUM_EXAMPLE_DEVSTONE_COUPLED_LI_HPP_

#include "devstone_coupled.hpp"

namespace cadmium::example::devstone {
	//! LI DEVStone model class.
	struct DEVStoneCoupledLI : public DEVStoneCoupled {
		/**
		 * LI model constructor function.
		 * @param width width of the LI DEVStone model.
		 * @param depth depth of the LI DEVStone model.
		 * @param intDelay internal delay for the DEVStone atomic models.
		 * @param extDelay external delay for the DEVStone atomic models.
		 */
		DEVStoneCoupledLI(int width, int depth, int intDelay, int extDelay);
	};
}  //namespace cadmium::example::devstone

#endif //CADMIUM_EXAMPLE_DEVSTONE_COUPLED_LI_HPP_
