#include <cadmium/simulation/rt_root_coordinator.hpp>

#ifdef RT_ARM_MBED
 	#include <cadmium/simulation/rt_clock/mbedclock.hpp>
#else
	#include <cadmium/simulation/rt_clock/chrono.hpp>
#endif

#ifdef RT_ARM_MBED
		#include <cadmium/simulation/logger/stdout.hpp>
#else
		#include <cadmium/simulation/logger/csv.hpp>
#endif

#include <limits>
#include "blinkySystem.hpp"

#ifdef RT_ARM_MBED
	#include "../mbed.h"
#endif

using namespace cadmium::blinkySystem;

int main(int argc, char *argv[]) {

	auto model = std::make_shared<blinkySystem>("blinkySystem");
#ifdef RT_ARM_MBED
    	// Let's create the RT clock. In this case, we will use a chrono clock with a maximum jitter of 10 milliseconds
    	// note that the max jitter is optional. If left blank, it won't check the delay jitter.
    	long maxJitter = 10000; // 10000 us 
    	auto clock = cadmium::MBEDClock(maxJitter); // choose the appropriate clock
    	// For creating RT coordinators, we must forward the model and the desired RT clock
    	auto rootCoordinator = cadmium::RealTimeRootCoordinator(model, clock);
#else
	auto maxJitter = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::milliseconds(10));
	auto clock = cadmium::ChronoClock(maxJitter);
	auto rootCoordinator = cadmium::RealTimeRootCoordinator(model, clock);
#endif

#ifdef RT_ARM_MBED
	rootCoordinator.setLogger<cadmium::STDOUTLogger>(";");
#else
	rootCoordinator.setLogger<cadmium::CSVLogger>("blinkyLog.csv",";");
#endif

	rootCoordinator.start();
 	rootCoordinator.simulate(std::numeric_limits<double>::infinity());
//	rootCoordinator.simulate(1000.0);
	rootCoordinator.stop();
	return 0;
}
