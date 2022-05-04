#ifndef _CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HI_HPP_
#define _CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HI_HPP_

#include "devstone_coupled.hpp"

struct DEVStoneCoupledHI: public DEVStoneCoupled {
	DEVStoneCoupledHI(int width, int depth, int intDelay, int extDelay);
};

#endif //_CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HI_HPP_
