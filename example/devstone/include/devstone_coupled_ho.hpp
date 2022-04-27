#ifndef _CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HO_HPP_
#define _CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HO_HPP_

#include "devstone_coupled.hpp"

struct DEVStoneCoupledHO: public DEVStoneCoupled {
	DEVStoneCoupledHO(int width, int depth, int intDelay, int extDelay);
};

#endif //_CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HO_HPP_
