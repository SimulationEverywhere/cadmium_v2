#include "dhrystone.hpp"
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <cstdlib>
#include <unistd.h>

namespace cadmium::example::devstone {

	void run (int millis) {
		std::this_thread::sleep_for(std::chrono::milliseconds(millis));
	}

	void runDhrystone(int millis) {
		// TODO
		std::this_thread::sleep_for(std::chrono::milliseconds(millis));
		//std::thread t(run, millis);
		//t.join();// pauses until first finishes
		//usleep(millis);
	}
}  //namespace cadmium::example::devstone
