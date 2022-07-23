#ifndef CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HI_HPP_
#define CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HI_HPP_

#include "devstone_coupled.hpp"

namespace cadmium::example::devstone {
	//! HI DEVStone model class.
	struct DEVStoneCoupledHI : public DEVStoneCoupled {
		/**
		 * HI model constructor function.
		 * @param width width of the HI DEVStone model.
		 * @param depth depth of the HI DEVStone model.
		 * @param intDelay internal delay for the DEVStone atomic models.
		 * @param extDelay external delay for the DEVStone atomic models.
		 */
		DEVStoneCoupledHI(int width, int depth, int intDelay, int extDelay);
	};
}  //namespace cadmium::example::devstone

#endif //CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HI_HPP_
