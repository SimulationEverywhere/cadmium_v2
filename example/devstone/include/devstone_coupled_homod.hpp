#ifndef _CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HOMOD_HPP_
#define _CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HOMOD_HPP_

#include "devstone_coupled.hpp"

struct DEVStoneCoupledHOmod: public DEVStoneCoupled {
	DEVStoneCoupledHOmod(int width, int depth, int intDelay, int extDelay);
};

#endif //_CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HOMOD_HPP_
