#include <cadmium/simulation/rt_root_coordinator.hpp>
#include <limits>
#include "blinkySystem.hpp"

#ifdef RT_ARM_MBED
 	#include <cadmium/simulation/rt_clock/mbedclock.hpp>
	#include <cadmium/simulation/logger/stdout.hpp>
	#include "../mbed.h"
#else
	#include <cadmium/simulation/rt_clock/chrono.hpp>
	#include <cadmium/simulation/logger/csv.hpp>
#endif

using namespace cadmium::blinkySystem;

int main(int argc, char *argv[]) {

	auto model = std::make_shared<blinkySystem>("blinkySystem");
#ifdef RT_ARM_MBED
    	// Let's create the RT clock. In this case, we will use an mbed clock with a maximum jitter of 10 milliseconds
    	// note that the max jitter is optional. If left blank, it won't check the delay jitter.
    	long maxJitter = 10000; // 10000 us 
    	auto clock = cadmium::MBEDClock(maxJitter); // choose the appropriate clock
    	// For creating RT coordinators, we must forward the model and the desired RT clock
    	auto rootCoordinator = cadmium::RealTimeRootCoordinator(model, clock);
	// Let's define a logger to log all the atomics' transitions throug stdout which is mapped to the serial port (mbed-os)
	rootCoordinator.setLogger<cadmium::STDOUTLogger>(";");
#else
    	// Let's create the RT clock. In this case, we will use a chrono clock with a maximum jitter of 10 milliseconds
    	// note that the max jitter is optional. If left blank, it won't check the delay jitter.
	auto maxJitter = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::milliseconds(10));
	auto clock = cadmium::ChronoClock(maxJitter);
    	// For creating RT coordinators, we must forward the model and the desired RT clock
	auto rootCoordinator = cadmium::RealTimeRootCoordinator(model, clock);
	// Let's define a logger to log all the atomics' transitions in a csv file
	rootCoordinator.setLogger<cadmium::CSVLogger>("blinkyLog.csv",";");
#endif

	rootCoordinator.start();
// 	rootCoordinator.simulate(std::numeric_limits<double>::infinity());
	rootCoordinator.simulate(10.0); // run simulation for 10s
	rootCoordinator.stop();
	return 0;
}
