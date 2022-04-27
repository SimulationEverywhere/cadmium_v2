#ifndef _CADMIUM_EXAMPLE_DEVSTONE_COUPLED_LI_HPP_
#define _CADMIUM_EXAMPLE_DEVSTONE_COUPLED_LI_HPP_

#include "devstone_coupled.hpp"

struct DEVStoneCoupledLI: public DEVStoneCoupled {
	DEVStoneCoupledLI(int width, int depth, int intDelay, int extDelay);
};

#endif //_CADMIUM_EXAMPLE_DEVSTONE_COUPLED_LI_HPP_
